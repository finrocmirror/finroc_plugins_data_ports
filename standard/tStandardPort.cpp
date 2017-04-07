//
// You received this file as part of Finroc
// A framework for intelligent robot control
//
// Copyright (C) Finroc GbR (finroc.org)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//----------------------------------------------------------------------
/*!\file    plugins/data_ports/standard/tStandardPort.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-23
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/standard/tStandardPort.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/port/tPortFactory.h"
#include "core/port/tPortWrapperBase.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/type_traits.h"
#include "plugins/data_ports/common/tPullOperation.h"
#include "plugins/data_ports/standard/tMultiTypePortBufferPool.h"
#include "plugins/data_ports/optimized/tCheapCopyPort.h"
#include "plugins/data_ports/optimized/tSingleThreadedCheapCopyPortGeneric.h"
#include "plugins/data_ports/standard/tPullRequestHandlerRaw.h"

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace data_ports
{
namespace standard
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------
namespace internal
{
class tDataPortFactory : public core::tPortFactory
{
  virtual core::tAbstractPort& CreatePortImplementation(const std::string& port_name, core::tFrameworkElement& parent,
      const rrlib::rtti::tType& type, core::tFrameworkElement::tFlags flags) override
  {
    core::tPortWrapperBase::tConstructorArguments<common::tAbstractDataPortCreationInfo> creation_info(port_name, &parent, type, flags);
    return IsCheaplyCopiedType(type) ? // TODO: put it
#ifndef RRLIB_SINGLE_THREADED
           * static_cast<core::tAbstractPort*>(new optimized::tCheapCopyPort(creation_info)) :
#else
           *static_cast<core::tAbstractPort*>(new optimized::tSingleThreadedCheapCopyPortGeneric(creation_info)) :
#endif
           *static_cast<core::tAbstractPort*>(new tStandardPort(creation_info));
    //return *static_cast<core::tAbstractPort*>(new tStandardPort(creation_info));
  }

  virtual bool HandlesDataType(const rrlib::rtti::tType& dt) override
  {
    return IsDataFlowType(dt);
  }
};

tDataPortFactory default_data_port_factory;

} // namespace internal


tStandardPort::tStandardPort(common::tAbstractDataPortCreationInfo creation_info) :
  common::tAbstractDataPort(creation_info),
  buffer_pool(this->GetDataType(), IsOutputPort() ? 2 : 0),
  multi_type_buffer_pool(GetFlag(tFlag::MULTI_TYPE_BUFFER_POOL) ? new tMultiTypePortBufferPool(buffer_pool, GetDataType()) : NULL),
  default_value(CreateDefaultValue(creation_info, buffer_pool)),
  current_value(0),
  standard_assign(!GetFlag(tFlag::NON_STANDARD_ASSIGN) && (!GetFlag(tFlag::HAS_QUEUE))),
  compression_active_status(-2),
  data_compressor_mutex("tStandardPort data compressor"),
  input_queue(),
  pull_request_handler(NULL)
{
  if ((!IsDataFlowType(GetDataType())) || IsCheaplyCopiedType(GetDataType()))
  {
    FINROC_LOG_PRINT(ERROR, "Data type ", GetDataType().GetName(), " is not suitable for standard port implementation.");
    abort();
  }

  // Initialize value
  tPortBufferManager* initial = default_value.get();
  if (!initial)
  {
    initial = GetUnusedBufferRaw().release();
    initial->InitReferenceCounter(0);
  }
  int pointer_tag = initial->GetPointerTag();
  initial->AddLocks(1, pointer_tag);
  initial->SetUnused(false);
  current_value.store(tTaggedBufferPointer(initial, pointer_tag));

  // Initialize queue
  if (GetFlag(tFlag::HAS_QUEUE))
  {
    input_queue.reset(new common::tPortQueue<tLockingManagerPointer>(!GetFlag(tFlag::HAS_DEQUEUE_ALL_QUEUE)));
    if (creation_info.max_queue_size > 0)
    {
      input_queue->SetMaxQueueLength(creation_info.max_queue_size);
    }
  }

  PropagateStrategy(NULL, NULL);  // initialize strategy
}


tStandardPort::~tStandardPort()
{
  tTaggedBufferPointer cur_pointer = current_value.load();
  tPortBufferUnlocker unlocker;
  unlocker(cur_pointer.GetPointer()); // thread safe, since nobody should publish to port anymore

  delete multi_type_buffer_pool;
}

/*!
 * Set current value to default value
 */
void tStandardPort::ApplyDefaultValue()
{
  if (!default_value)
  {
    FINROC_LOG_PRINT(ERROR, "No default value has been set. Doing nothing.");
    return;
  }
  default_value->AddLocks(1);
  tLockingManagerPointer pointer(default_value.get());
  Publish(pointer);
}

void tStandardPort::BrowserPublish(tUnusedManagerPointer& data, bool notify_listener_on_this_port, tChangeStatus change_constant)
{
  if (notify_listener_on_this_port)
  {
    if (change_constant == tChangeStatus::CHANGED_INITIAL)
    {
      PublishImplementation<false, tChangeStatus::CHANGED_INITIAL, true, true>(data);
    }
    else
    {
      PublishImplementation<false, tChangeStatus::CHANGED, true, true>(data);
    }
  }
  else
  {
    if (change_constant == tChangeStatus::CHANGED_INITIAL)
    {
      PublishImplementation<false, tChangeStatus::CHANGED_INITIAL, true, false>(data);
    }
    else
    {
      PublishImplementation<false, tChangeStatus::CHANGED, true, false>(data);
    }
  }
}

void tStandardPort::CallPullRequestHandler(tPublishingData& publishing_data)
{
  tUniversalManagerPointer mgr = pull_request_handler->RawPullRequest(*this);
  if (mgr)
  {
    if (mgr->IsUnused())
    {
      mgr->InitReferenceCounter(publishing_data.added_locks);
    }
    else
    {
      mgr->AddLocks(publishing_data.added_locks - 1); // -1 because we release lock of this pointer later
    }
    publishing_data.Init(mgr.release());
  }
}

tPortBufferManager* tStandardPort::CreateDefaultValue(const common::tAbstractDataPortCreationInfo& creation_info, tBufferPool& buffer_pool)
{
  if (creation_info.DefaultValueSet() || creation_info.flags.Get(tFlag::DEFAULT_ON_DISCONNECT))
  {
    tPortBufferManager* pdm = buffer_pool.GetUnusedBuffer(creation_info.data_type).release();
    pdm->InitReferenceCounter(1);
    if (creation_info.DefaultValueSet())
    {
      rrlib::serialization::tInputStream input(creation_info.GetDefaultGeneric());
      pdm->GetObject().Deserialize(input);
    }
    return pdm;
  }
  return NULL;
}

void tStandardPort::ForwardData(tAbstractDataPort& other)
{
  assert(IsDataFlowType(other.GetDataType()) && (!IsCheaplyCopiedType(other.GetDataType())));
  tLockingManagerPointer pointer = GetCurrentValueRaw();
  (static_cast<tStandardPort&>(other)).Publish(pointer);
}

rrlib::rtti::tGenericObject& tStandardPort::GetDefaultBufferRaw()
{
  if (IsReady())
  {
    FINROC_LOG_PRINT(ERROR, "Please set default value _before_ initializing port");
    abort();
  }
  if (!default_value)
  {
    default_value.reset(GetUnusedBufferRaw().release());
    default_value->InitReferenceCounter(1);
  }
  return default_value->GetObject();
}

int tStandardPort::GetMaxQueueLengthImplementation() const
{
  return input_queue ? input_queue->GetMaxQueueLength() : -1;
}

tStandardPort::tUnusedManagerPointer tStandardPort::GetUnusedBufferRaw(const rrlib::rtti::tType& dt)
{
  assert(multi_type_buffer_pool);
  tUnusedManagerPointer buffer = multi_type_buffer_pool->GetUnusedBuffer(dt);
  buffer->SetUnused(true);
  return buffer;
}

void tStandardPort::InitialPushTo(tAbstractPort& target, bool reverse)
{
  tLockingManagerPointer manager = GetCurrentValueRaw(tStrategy::NEVER_PULL);
  assert(IsReady());

  common::tPublishOperation<tStandardPort, tPublishingData> data(manager, 1000);
  tStandardPort& target_port = static_cast<tStandardPort&>(target);
  if (reverse)
  {
    common::tPublishOperation<tStandardPort, tPublishingData>::Receive<true, tChangeStatus::CHANGED_INITIAL>(data, target_port, *this);
  }
  else
  {
    common::tPublishOperation<tStandardPort, tPublishingData>::Receive<false, tChangeStatus::CHANGED_INITIAL>(data, target_port, *this);
  }
}

void tStandardPort::LockCurrentValueForPublishing(tPublishingData& publishing_data)
{
  tLockingManagerPointer locked_buffer = LockCurrentValueForRead(publishing_data.added_locks);
  publishing_data.Init(locked_buffer.release());
}

void tStandardPort::NonStandardAssign(tPublishingData& publishing_data, tChangeStatus change_constant)
{
  if (GetFlag(tFlag::USES_QUEUE) && change_constant != tChangeStatus::CHANGED_INITIAL)
  {
    assert(GetFlag(tFlag::HAS_QUEUE) && input_queue);

    // enqueue
    publishing_data.AddLock();
    input_queue->Enqueue(tLockingManagerPointer(publishing_data.published_buffer));
  }
}

tStandardPort::tLockingManagerPointer tStandardPort::PullValueRaw(bool ignore_pull_request_handler_on_this_port)
{
  common::tPullOperation<tStandardPort, tPublishingData, tPortBufferManager> pull_operation(200);
  pull_operation.Execute(*this);
  return tLockingManagerPointer(pull_operation.published_buffer);
}

//void tStandardPort::SetMaxQueueLengthImplementation(int length)
//{
//  assert((GetFlag(tFlag::HAS_QUEUE) && queue != NULL));
//  assert((!IsOutputPort()));
//  assert((length >= 1));
//  queue->SetMaxLength(length);
//}

void tStandardPort::SetPullRequestHandler(tPullRequestHandlerRaw* pull_request_handler_)
{
  if (pull_request_handler_ != NULL)
  {
    this->pull_request_handler = pull_request_handler_;
  }
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

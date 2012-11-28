//
// You received this file as part of Finroc
// A Framework for intelligent robot control
//
// Copyright (C) Finroc GbR (finroc.org)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//----------------------------------------------------------------------
/*!\file    plugins/data_ports/optimized/tCheapCopyPort.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/tCheapCopyPort.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/type_traits.h"
#include "plugins/data_ports/common/tPullOperation.h"

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
namespace optimized
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

/*! Creates default value */
static rrlib::rtti::tGenericObject* CreateDefaultValue(const common::tAbstractDataPortCreationInfo& creation_info)
{
  if (creation_info.DefaultValueSet() || creation_info.flags.Get(core::tFrameworkElement::tFlag::DEFAULT_ON_DISCONNECT))
  {
    rrlib::rtti::tGenericObject* result = creation_info.data_type.CreateInstanceGeneric();
    if (creation_info.DefaultValueSet())
    {
      rrlib::serialization::tInputStream input(&creation_info.GetDefaultGeneric());
      result->Deserialize(input);
    }
    return result;
  }
  return NULL;
}

} // namespace internal

tCheapCopyPort::tCheapCopyPort(common::tAbstractDataPortCreationInfo creation_info) :
  common::tAbstractDataPort(creation_info),
  cheaply_copyable_type_index(RegisterPort(creation_info.data_type)),
  default_value(internal::CreateDefaultValue(creation_info)),
  current_value(0),
  standard_assign(!GetFlag(tFlag::NON_STANDARD_ASSIGN) && (!GetFlag(tFlag::HAS_QUEUE))),
  queue_fifo(NULL),
  pull_request_handler(NULL),
  port_listeners(),
  unit(creation_info.unit)
{
  if ((!IsDataFlowType(GetDataType())) || (!IsCheaplyCopiedType(GetDataType())))
  {
    FINROC_LOG_PRINT(ERROR, "Data type ", GetDataType().GetName(), " is not suitable for cheap copy port implementation.");
    abort();
  }

  // Initialize value
  tCheaplyCopiedBufferManager* initial = tGlobalBufferPools::Instance().GetUnusedBuffer(cheaply_copyable_type_index).release();
  initial->InitReferenceCounter(1);
  int pointer_tag = initial->GetPointerTag();
  current_value.store(tTaggedBufferPointer(initial, pointer_tag));

  // set initial value to default?
  if (creation_info.DefaultValueSet())
  {
    initial->GetObject().DeepCopyFrom(*default_value);
  }

  // Initialize queue
  if (GetFlag(tFlag::HAS_QUEUE))
  {
    if (GetFlag(tFlag::HAS_DEQUEUE_ALL_QUEUE))
    {
      queue_all = new tDequeueAllPortQueue();
    }
    else
    {
      queue_fifo = new tFifoPortQueue();
    }
  }

  PropagateStrategy(NULL, NULL);  // initialize strategy
}

tCheapCopyPort::~tCheapCopyPort()
{
  tLock lock(*this);

  tTaggedBufferPointer cur_pointer = current_value.exchange(0);
  tPortBufferUnlocker unlocker;
  unlocker(cur_pointer.GetPointer());

  if (queue_all)
  {
    if (GetFlag(tFlag::HAS_DEQUEUE_ALL_QUEUE))
    {
      delete queue_all;
    }
    else
    {
      delete queue_fifo;
    }
  }
}

void tCheapCopyPort::ApplyDefaultValue()
{
  if (!default_value)
  {
    FINROC_LOG_PRINT(ERROR, "No default value has been set. Doing nothing.");
    return;
  }
  // Publish(*default_value); TODO
}

std::string tCheapCopyPort::BrowserPublishRaw(tUnusedManagerPointer& buffer)
{
  if (buffer->GetThreadLocalOrigin() && buffer->GetThreadLocalOrigin() == tThreadLocalBufferPools::Get()) // Is current thread the owner?
  {
    common::tPublishOperation<tCheapCopyPort, tPublishingDataThreadLocalBuffer> data(buffer);
    data.Execute<false, tChangeStatus::CHANGED, true>(*this);
  }
  else
  {
    common::tPublishOperation<tCheapCopyPort, tPublishingDataGlobalBuffer> data(buffer);
    data.Execute<false, tChangeStatus::CHANGED, true>(*this);
  }
  return "";
}

void tCheapCopyPort::CallPullRequestHandler(tPublishingDataGlobalBuffer& publishing_data, bool intermediate_assign)
{
  tUnusedManagerPointer result = tUnusedManagerPointer(tGlobalBufferPools::Instance().GetUnusedBuffer(cheaply_copyable_type_index).release());
  if (pull_request_handler->PullRequest(*this, *result, intermediate_assign))
  {
    publishing_data.Init(result);
  }
}

void tCheapCopyPort::CallPullRequestHandler(tPublishingDataThreadLocalBuffer& publishing_data, bool intermediate_assign)
{
  tUnusedManagerPointer result = tUnusedManagerPointer(tThreadLocalBufferPools::Get()->GetUnusedBuffer(cheaply_copyable_type_index).release());
  if (pull_request_handler->PullRequest(*this, *result, intermediate_assign))
  {
    publishing_data.Init(result);
  }
}

//bool tCheapCopyPort::ContainsDefaultValue()
//{
//  tCCPortDataManager* c = GetInInterThreadContainer();
//  bool result = c->GetObject()->GetType() == default_value->GetObject()->GetType() && c->GetObject()->Equals(*default_value->GetObject());
//  c->Recycle2();
//  return result;
//}

void tCheapCopyPort::CopyCurrentValueToGenericObject(rrlib::rtti::tGenericObject& buffer, rrlib::time::tTimestamp& timestamp, bool dont_pull)
{
  if (PushStrategy() || dont_pull)
  {
    for (; ;)
    {
      tTaggedBufferPointer current = current_value.load();
      buffer.DeepCopyFrom(GetObject(*current));
      timestamp = current->GetTimestamp();
      tTaggedBufferPointer::tStorage current_raw = current;
      if (current_raw == current_value.load())    // still valid??
      {
        return;
      }
    }
  }
  else
  {
    tLockingManagerPointer dc = PullValueRaw();
    buffer.DeepCopyFrom(dc->GetObject(), NULL);
    timestamp = dc->GetTimestamp();
  }
}

void tCheapCopyPort::ForwardData(tAbstractPort& other)
{
  assert(IsDataFlowType(other.GetDataType()) && (IsCheaplyCopiedType(other.GetDataType())));

  if (tThreadLocalBufferPools::Get())
  {
    tTaggedBufferPointer current_buffer = current_value.load();
    if (tThreadLocalBufferPools::Get() == current_buffer->GetThreadLocalOrigin()) // Is current thread the owner thread?
    {
      common::tPublishOperation<tCheapCopyPort, tPublishingDataThreadLocalBuffer> data(static_cast<tThreadLocalBufferManager*>(current_buffer.GetPointer()), false);
      data.Execute<false, tChangeStatus::CHANGED, false>(static_cast<tCheapCopyPort&>(other));
      return;
    }

    // there obviously will not arrive any buffer from current thread in the meantime

    auto unused_manager = tThreadLocalBufferPools::Get()->GetUnusedBuffer(cheaply_copyable_type_index);
    for (; ;)
    {
      unused_manager->GetObject().DeepCopyFrom(GetObject(*current_buffer));
      unused_manager->SetTimestamp(current_buffer->GetTimestamp());
      typename tTaggedBufferPointer::tStorage current_raw = current_buffer;
      typename tTaggedBufferPointer::tStorage current_raw_2 = current_value.load();
      if (current_raw == current_raw_2)    // still valid??
      {
        common::tPublishOperation<tCheapCopyPort, tPublishingDataThreadLocalBuffer> data(static_cast<tThreadLocalBufferManager*>(current_buffer.GetPointer()), true);
        data.Execute<false, tChangeStatus::CHANGED, false>(static_cast<tCheapCopyPort&>(other));
        return;
      }
      current_buffer = current_raw_2;
    }
  }
  else
  {
    tUnusedManagerPointer unused_manager = tUnusedManagerPointer(tGlobalBufferPools::Instance().GetUnusedBuffer(cheaply_copyable_type_index).release());
    CopyCurrentValueToManager(*unused_manager, true);
    common::tPublishOperation<tCheapCopyPort, tPublishingDataGlobalBuffer> data(unused_manager);
    data.Execute<false, tChangeStatus::CHANGED, false>(static_cast<tCheapCopyPort&>(other));
  }
}

//const rrlib::rtti::tGenericObject* tCheapCopyPort::GetAutoLockedRaw()
//{
//  tThreadLocalCache* tc = tThreadLocalCache::Get();
//
//  if (PushStrategy())
//  {
//    tCCPortDataManagerTL* mgr = GetLockedUnsafeInContainer();
//    tc->AddAutoLock(mgr);
//    return mgr->GetObject();
//  }
//  else
//  {
//    tCCPortDataManager* mgr = GetInInterThreadContainer();
//    tc->AddAutoLock(mgr);
//    return mgr->GetObject();
//  }
//}

int tCheapCopyPort::GetMaxQueueLengthImplementation() const
{
  if (queue_all)
  {
    if (GetFlag(tFlag::HAS_DEQUEUE_ALL_QUEUE))
    {
      return queue_all->GetMaxLength();
    }
    else
    {
      return queue_fifo->GetMaxLength();
    }
  }
  return -1;
}

//tCCPortDataManager* tCheapCopyPort::GetInInterThreadContainer(bool dont_pull)
//{
//  tCCPortDataManager* ccitc = tThreadLocalCache::Get()->GetUnusedInterThreadBuffer(GetDataType());
//  rrlib::time::tTimestamp timestamp;
//  GetRaw(*ccitc->GetObject(), timestamp, dont_pull);
//  ccitc->SetTimestamp(timestamp);
//  return ccitc;
//}
//
//tCCPortDataManagerTL* tCheapCopyPort::GetLockedUnsafeInContainer()
//{
//  tCCPortDataRef* val = value;
//  tCCPortDataManagerTL* val_c = val->GetContainer();
//  if (val_c->GetOwnerThread() == rrlib::thread::tThread::CurrentThreadId())    // if same thread: simply add read lock
//  {
//    val_c->AddLock();
//    return val_c;
//  }
//
//  // not the same thread: create auto-locked new container
//  tThreadLocalCache* tc = tThreadLocalCache::Get();
//  tCCPortDataManagerTL* ccitc = tc->GetUnusedBuffer(this->data_type);
//  ccitc->ref_counter = 1;
//  for (; ;)
//  {
//    ccitc->GetObject()->DeepCopyFrom(val_c->GetObject(), NULL);
//    ccitc->SetTimestamp(val_c->GetTimestamp());
//    if (val == value)    // still valid??
//    {
//      return ccitc;
//    }
//    val = value;
//    val_c = val->GetContainer();
//  }
//}
//
//tCCPortDataManager* tCheapCopyPort::GetPullInInterthreadContainerRaw(bool intermediate_assign, bool ignore_pull_request_handler_on_this_port)
//{
//  tCCPortDataManagerTL* tmp = PullValueRaw(intermediate_assign, ignore_pull_request_handler_on_this_port);
//  tCCPortDataManager* ret = tThreadLocalCache::GetFast()->GetUnusedInterThreadBuffer(this->data_type);
//  ret->GetObject()->DeepCopyFrom(tmp->GetObject(), NULL);
//  ret->SetTimestamp(tmp->GetTimestamp());
//  tmp->ReleaseLock();
//  return ret;
//}
//

void tCheapCopyPort::InitialPushTo(tAbstractPort& target, bool reverse)
{
  // this is a one-time event => use global buffer
  tUnusedManagerPointer unused_manager(tGlobalBufferPools::Instance().GetUnusedBuffer(cheaply_copyable_type_index).release());
  CopyCurrentValueToManager(*unused_manager, true);
  common::tPublishOperation<tCheapCopyPort, tPublishingDataGlobalBuffer> data(unused_manager);
  tCheapCopyPort& target_port = static_cast<tCheapCopyPort&>(target);
  if (reverse)
  {
    common::tPublishOperation<tCheapCopyPort, tPublishingDataGlobalBuffer>::Receive<true, tChangeStatus::CHANGED_INITIAL>(data, target_port, *this);
  }
  else
  {
    common::tPublishOperation<tCheapCopyPort, tPublishingDataGlobalBuffer>::Receive<false, tChangeStatus::CHANGED_INITIAL>(data, target_port, *this);
  }
}

void tCheapCopyPort::LockCurrentValueForPublishing(tPublishingDataGlobalBuffer& publishing_data)
{
  while (true)
  {
    tTaggedBufferPointer current_buffer = current_value.load();
    if (current_buffer->GetThreadLocalOrigin())
    {
      tUnusedManagerPointer unused_manager = tUnusedManagerPointer(tGlobalBufferPools::Instance().GetUnusedBuffer(cheaply_copyable_type_index).release());
      CopyCurrentValueToManager(*unused_manager, true);
      publishing_data.Init(unused_manager);
      return;
    }
    else
    {
      if (current_buffer->TryLock(tPublishingDataGlobalBuffer::cADD_LOCKS, current_buffer.GetStamp()))
      {
        // successful
        publishing_data.InitSuccessfullyLocked(current_buffer.GetPointer());
        return;
      }
    }
  }
}

void tCheapCopyPort::LockCurrentValueForPublishing(tPublishingDataThreadLocalBuffer& publishing_data)
{
  assert(tThreadLocalBufferPools::Get());
  tTaggedBufferPointer current_buffer = current_value.load();
  if (tThreadLocalBufferPools::Get() == current_buffer->GetThreadLocalOrigin()) // Is current thread the owner thread?
  {
    publishing_data = tPublishingDataThreadLocalBuffer(static_cast<tThreadLocalBufferManager*>(current_buffer.GetPointer()), false);
    return;
  }

  // there obviously will not arrive any buffer from current thread in the meantime

  auto unused_manager = tThreadLocalBufferPools::Get()->GetUnusedBuffer(cheaply_copyable_type_index);
  for (; ;)
  {
    unused_manager->GetObject().DeepCopyFrom(GetObject(*current_buffer));
    unused_manager->SetTimestamp(current_buffer->GetTimestamp());
    typename tTaggedBufferPointer::tStorage current_raw = current_buffer;
    typename tTaggedBufferPointer::tStorage current_raw_2 = current_value.load();
    if (current_raw == current_raw_2)    // still valid??
    {
      publishing_data = tPublishingDataThreadLocalBuffer(unused_manager.release(), true);
      return;
    }
    current_buffer = current_raw_2;
  }
}

bool tCheapCopyPort::NonStandardAssign(tPublishingDataGlobalBuffer& publishing_data)
{
  if (GetFlag(tFlag::USES_QUEUE))
  {
    assert((GetFlag(tFlag::HAS_QUEUE)));

    // enqueue
    publishing_data.AddLock();
    if (GetFlag(tFlag::HAS_DEQUEUE_ALL_QUEUE))
    {
      queue_all->Enqueue(tLockingManagerPointer(publishing_data.published_buffer));
    }
    else
    {
      queue_fifo->Enqueue(tLockingManagerPointer(publishing_data.published_buffer));
    }
  }
  return true;
}

bool tCheapCopyPort::NonStandardAssign(tPublishingDataThreadLocalBuffer& publishing_data)
{
  if (GetFlag(tFlag::USES_QUEUE))
  {
    assert((GetFlag(tFlag::HAS_QUEUE)));

    // enqueue
    publishing_data.AddLock();
    if (GetFlag(tFlag::HAS_DEQUEUE_ALL_QUEUE))
    {
      queue_all->Enqueue(tLockingManagerPointer(publishing_data.published_buffer));
    }
    else
    {
      queue_fifo->Enqueue(tLockingManagerPointer(publishing_data.published_buffer));
    }
  }
  return true;
}

void tCheapCopyPort::NotifyDisconnect()
{
  if (GetFlag(tFlag::DEFAULT_ON_DISCONNECT))
  {
    ApplyDefaultValue();
  }
}

tCheapCopyPort::tLockingManagerPointer tCheapCopyPort::PullValueRaw(bool intermediate_assign, bool ignore_pull_request_handler_on_this_port)
{
  if (tThreadLocalBufferPools::Get())
  {
    common::tPullOperation<tCheapCopyPort, tPublishingDataThreadLocalBuffer, tThreadLocalBufferManager> pull_operation;
    pull_operation.Execute(*this, intermediate_assign);
    return tLockingManagerPointer(pull_operation.published_buffer);
  }
  else
  {
    common::tPullOperation<tCheapCopyPort, tPublishingDataGlobalBuffer, tCheaplyCopiedBufferManager> pull_operation;
    pull_operation.Execute(*this, intermediate_assign);
    return tLockingManagerPointer(pull_operation.published_buffer);
  }
}

void tCheapCopyPort::SetDefault(rrlib::rtti::tGenericObject& new_default)
{
  if (IsReady())
  {
    FINROC_LOG_PRINT(ERROR, "Please set default value _before_ initializing port");
    abort();
  }
  if (new_default.GetType() != this->GetDataType())
  {
    FINROC_LOG_PRINT(ERROR, "New default value has wrong type: ", new_default.GetType().GetName());
    abort();
  }

  if (!default_value)
  {
    default_value.reset(new_default.GetType().CreateInstanceGeneric());
  }
  else if (default_value->GetType() != new_default.GetType())
  {
    FINROC_LOG_PRINT(ERROR, "Provided default value has wrong type. Ignoring.");
    return;
  }
  default_value->DeepCopyFrom(new_default);
}

//void tCheapCopyPort::SetMaxQueueLengthImpl(int length)
//{
//  assert((GetFlag(tFlag::HAS_QUEUE) && queue != NULL));
//  assert((!IsOutputPort()));
//  assert((length >= 1));
//  queue->SetMaxLength(length);
//}

void tCheapCopyPort::SetPullRequestHandler(tPullRequestHandlerRaw* pull_request_handler_)
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
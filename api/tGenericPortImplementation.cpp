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
/*!\file    plugins/data_ports/api/tGenericPortImplementation.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tGenericPortImplementation.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tPort.h"
#include "plugins/data_ports/tPullRequestHandler.h"

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
namespace api
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

template <typename T>
struct tBoundsSetter
{
  static void SetBounds(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    typedef api::tBoundedPort<T, api::tPortImplementationTypeTrait<T>::type> tBoundedPort;
    if (typeid(port) != typeid(tBoundedPort))
    {
      FINROC_LOG_PRINT_STATIC(ERROR, "Cannot set bounds for port ", rrlib::rtti::tDataType<T>().GetName(), ". It is not a bounded port.");
      return;
    }
    static_cast<tBoundedPort&>(port).SetBounds(tBounds<T>(min.GetData<T>(), max.GetData<T>()));
  }
};

struct tNoBoundsSetter
{
  static void SetBounds(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    FINROC_LOG_PRINT_STATIC(ERROR, "Cannot set bounds for type ", port.GetDataType().GetName());
  }
};


template <typename T>
class tGenericPortImplementationTyped : public tGenericPortImplementation
{
public:

  /*! Should methods dealing with bounds be available? */
  enum { cBOUNDABLE = IsBoundable<T>::value };

  /*! Class that contains actual implementation of most functionality */
  typedef api::tPortImplementation<T, api::tPortImplementationTypeTrait<T>::type> tImplementation;

  typedef typename tImplementation::tPortBase tPortBase;

  virtual core::tAbstractPort* CreatePort(const common::tAbstractDataPortCreationInfo& creation_info) override
  {
    tPort<T> port(creation_info);
    return port.GetWrapped();
  }

  virtual void Get(core::tAbstractPort& port, rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp) override
  {
    tImplementation::CopyCurrentPortValue(static_cast<tPortBase&>(port), result.GetData<T>(), timestamp);
  }

  virtual tPortDataPointer<const rrlib::rtti::tGenericObject> GetPointer(core::tAbstractPort& abstract_port, tStrategy strategy) override
  {
    tPortBase& port = static_cast<tPortBase&>(abstract_port);
    if ((strategy == tStrategy::DEFAULT && port.PushStrategy()) || strategy == tStrategy::NEVER_PULL || definitions::cSINGLE_THREADED)
    {
      tPortDataPointer<rrlib::rtti::tGenericObject> buffer = this->GetUnusedBuffer(port);
      rrlib::time::tTimestamp timestamp;
      port.CopyCurrentValueToGenericObject(*buffer.Get(), timestamp, strategy);
      buffer.SetTimestamp(timestamp);
      return std::move(buffer);
    }
#ifndef RRLIB_SINGLE_THREADED
    else
    {
      auto buffer_pointer = port.GetPullRaw(strategy == tStrategy::PULL_IGNORING_HANDLER_ON_THIS_PORT);
      return tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false>(buffer_pointer.release(), false);
    }
#endif
  }

  virtual void Publish(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp) override
  {
    tImplementation::CopyAndPublish(static_cast<tPortBase&>(port), data.GetData<T>(), timestamp);
  }

  virtual void SetBounds(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max) override
  {
    std::conditional<cBOUNDABLE, tBoundsSetter<T>, tNoBoundsSetter>::type::SetBounds(port, min, max);
  }
};


class tGenericPortImplementationCheapCopy : public tGenericPortImplementation
{
public:

  typedef tCheapCopyPort tPortBase;

  virtual core::tAbstractPort* CreatePort(const common::tAbstractDataPortCreationInfo& creation_info) override
  {
    return new tPortBase(creation_info);
  }

  virtual void Get(core::tAbstractPort& port, rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp) override
  {
    static_cast<tPortBase&>(port).CopyCurrentValueToGenericObject(result, timestamp);
  }

  virtual tPortDataPointer<const rrlib::rtti::tGenericObject> GetPointer(core::tAbstractPort& abstract_port, tStrategy strategy) override
  {
    tPortBase& port = static_cast<tPortBase&>(abstract_port);
    if ((strategy == tStrategy::DEFAULT && port.PushStrategy()) || strategy == tStrategy::NEVER_PULL || definitions::cSINGLE_THREADED)
    {
      tPortDataPointer<rrlib::rtti::tGenericObject> buffer = this->GetUnusedBuffer(port);
      rrlib::time::tTimestamp timestamp;
      port.CopyCurrentValueToGenericObject(*buffer.Get(), timestamp, strategy);
      buffer.SetTimestamp(timestamp);
      return std::move(buffer);
    }
#ifndef RRLIB_SINGLE_THREADED
    else
    {
      auto buffer_pointer = port.GetPullRaw(strategy == tStrategy::PULL_IGNORING_HANDLER_ON_THIS_PORT);
      return tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false>(buffer_pointer.release(), false);
    }
#endif
  }

  virtual void Publish(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp) override
  {
#ifndef RRLIB_SINGLE_THREADED
    assert(data.GetType() == port.GetDataType());
    optimized::tThreadLocalBufferPools* thread_local_pools = optimized::tThreadLocalBufferPools::Get();
    if (thread_local_pools)
    {
      typename optimized::tThreadLocalBufferPools::tBufferPointer buffer = thread_local_pools->GetUnusedBuffer(static_cast<tPortBase&>(port).GetCheaplyCopyableTypeIndex());
      buffer->SetTimestamp(timestamp);
      buffer->GetObject().DeepCopyFrom(data);
      common::tPublishOperation<optimized::tCheapCopyPort, typename optimized::tCheapCopyPort::tPublishingDataThreadLocalBuffer> publish_operation(buffer.release(), true);
      publish_operation.Execute<false, tChangeStatus::CHANGED, false, false>(static_cast<tPortBase&>(port));
    }
    else
    {
      typename optimized::tCheapCopyPort::tUnusedManagerPointer buffer(optimized::tGlobalBufferPools::Instance().GetUnusedBuffer(static_cast<tPortBase&>(port).GetCheaplyCopyableTypeIndex()).release());
      buffer->SetTimestamp(timestamp);
      buffer->GetObject().DeepCopyFrom(data);
      common::tPublishOperation<optimized::tCheapCopyPort, typename optimized::tCheapCopyPort::tPublishingDataGlobalBuffer> publish_operation(buffer);
      publish_operation.Execute<false, tChangeStatus::CHANGED, false, false>(static_cast<tPortBase&>(port));
    }
#else
    static_cast<tPortBase&>(port).Publish(data, timestamp);
#endif
  }

  virtual void SetBounds(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max) override
  {
    FINROC_LOG_PRINT(ERROR, "Cannot set bounds for type ", port.GetDataType().GetName());
  }
};

class tGenericPortImplementationStandard : public tGenericPortImplementation
{
public:

  typedef standard::tStandardPort tPortBase;

  virtual core::tAbstractPort* CreatePort(const common::tAbstractDataPortCreationInfo& creation_info) override
  {
    return new tPortBase(creation_info);
  }

  virtual void Get(core::tAbstractPort& port, rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp) override
  {
    typename tPortBase::tLockingManagerPointer mgr = static_cast<tPortBase&>(port).GetCurrentValueRaw();
    result.DeepCopyFrom(mgr->GetObject());
    timestamp = mgr->GetTimestamp();
  }

  virtual tPortDataPointer<const rrlib::rtti::tGenericObject> GetPointer(core::tAbstractPort& port, tStrategy strategy) override
  {
    typename tPortBase::tLockingManagerPointer mgr = static_cast<tPortBase&>(port).GetCurrentValueRaw(strategy);
    return tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false>(mgr.release(), false);
  }

  virtual void Publish(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp) override
  {
    assert(data.GetType() == port.GetDataType());
    typename tPortBase::tUnusedManagerPointer mgr = static_cast<tPortBase&>(port).GetUnusedBufferRaw();
    mgr->GetObject().DeepCopyFrom(data);
    mgr->SetTimestamp(timestamp);
    static_cast<tPortBase&>(port).Publish(mgr);
  }

  virtual void SetBounds(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max) override
  {
    FINROC_LOG_PRINT(ERROR, "Cannot set bounds for type ", port.GetDataType().GetName());
  }
};

template <typename T>
static void CheckCreateImplementationForType(rrlib::rtti::tType type)
{
  if (type.GetRttiName() == typeid(T).name())
  {
    static internal::tGenericPortImplementationTyped<T> cINSTANCE;
    type.AddAnnotation<tGenericPortImplementation*>(&cINSTANCE);
  }
}

tGenericPortImplementationCheapCopy cINSTANCE_CHEAP_COPY;
tGenericPortImplementationStandard cINSTANCE_STANDARD;

} // namespace internal

void tGenericPortImplementation::CreateImplementations()
{
  static rrlib::thread::tMutex mutex;
  static size_t initialized_types = 0;
  rrlib::thread::tLock lock(mutex);

  for (; initialized_types < rrlib::rtti::tType::GetTypeCount(); initialized_types++)
  {
    rrlib::rtti::tType type = rrlib::rtti::tType::GetType(initialized_types);
    if (IsDataFlowType(type))
    {
      // typed implementations for certain types
      internal::CheckCreateImplementationForType<int8_t>(type);
      internal::CheckCreateImplementationForType<int16_t>(type);
      internal::CheckCreateImplementationForType<int>(type);
      internal::CheckCreateImplementationForType<long long int>(type);
      internal::CheckCreateImplementationForType<uint8_t>(type);
      internal::CheckCreateImplementationForType<uint16_t>(type);
      internal::CheckCreateImplementationForType<unsigned int>(type);
      internal::CheckCreateImplementationForType<unsigned long long int>(type);
      internal::CheckCreateImplementationForType<double>(type);
      internal::CheckCreateImplementationForType<float>(type);
      internal::CheckCreateImplementationForType<char>(type);  // is neither int8_t nor uint8_t
      internal::CheckCreateImplementationForType<numeric::tNumber>(type);

      if (!type.GetAnnotation<tGenericPortImplementation*>())
      {
        assert((type.GetTypeTraits() & rrlib::rtti::trait_flags::cIS_INTEGRAL) == 0 || type.GetRttiName() == typeid(bool).name());
        if (IsCheaplyCopiedType(type))
        {
          type.AddAnnotation<tGenericPortImplementation*>(&internal::cINSTANCE_CHEAP_COPY);
        }
        else
        {
          type.AddAnnotation<tGenericPortImplementation*>(&internal::cINSTANCE_STANDARD);
        }
      }
    }
  }
}

void tGenericPortImplementation::SetPullRequestHandler(core::tAbstractPort& port, tPullRequestHandler<rrlib::rtti::tGenericObject>* pull_request_handler)
{
  if (IsCheaplyCopiedType(port.GetDataType()))
  {
    static_cast<tCheapCopyPort&>(port).SetPullRequestHandler(pull_request_handler);
  }
  else
  {
    static_cast<standard::tStandardPort&>(port).SetPullRequestHandler(pull_request_handler);
  }
}


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

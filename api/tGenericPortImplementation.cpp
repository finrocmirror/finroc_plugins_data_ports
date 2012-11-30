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
      FINROC_LOG_PRINT(ERROR, "Cannot set bounds for port ", rrlib::rtti::tDataType<T>().GetName(), ". It is not a bounded port.");
      return;
    }
    static_cast<tBoundedPort&>(port).SetBounds(tBounds<T>(min.GetData<T>(), max.GetData<T>()));
  }
};

struct tNoBoundsSetter
{
  static void SetBounds(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    FINROC_LOG_PRINT(ERROR, "Cannot set bounds for type ", port.GetDataType().GetName());
  }
};


template <typename T>
class tGenericPortImplementationTyped : public tGenericPortImplementation
{
public:

  /*! Should methods dealing with bounds be available? */
  enum { cBOUNDABLE = tIsBoundable<T>::value };

  /*! Class that contains actual implementation of most functionality */
  typedef api::tPortImplementation<T, api::tPortImplementationTypeTrait<T>::type> tImplementation;

  typedef optimized::tCheapCopyPort tPortBase;

  virtual core::tAbstractPort* CreatePort(const common::tAbstractDataPortCreationInfo& creation_info)
  {
    tPort<T> port(creation_info);
    return port.GetWrapped();
  }

  virtual void Get(core::tAbstractPort& port, rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp)
  {
    tImplementation::CopyCurrentPortValue(static_cast<tPortBase&>(port), result.GetData<T>(), timestamp);
  }

  virtual void Publish(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp)
  {
    tImplementation::CopyAndPublish(static_cast<tPortBase&>(port), data.GetData<T>(), timestamp);
  }

  virtual void SetBounds(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    std::conditional<cBOUNDABLE, tBoundsSetter<T>, tNoBoundsSetter>::type::SetBounds(port, min, max);
  }
};


class tGenericPortImplementationCheapCopy : public tGenericPortImplementation
{
public:

  typedef optimized::tCheapCopyPort tPortBase;

  virtual core::tAbstractPort* CreatePort(const common::tAbstractDataPortCreationInfo& creation_info)
  {
    return new tPortBase(creation_info);
  }

  virtual void Get(core::tAbstractPort& port, rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp)
  {
    static_cast<tPortBase&>(port).CopyCurrentValueToGenericObject(result, timestamp);
  }

  virtual void Publish(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp)
  {
    assert(data.GetType() == port.GetDataType());
    optimized::tThreadLocalBufferPools* thread_local_pools = optimized::tThreadLocalBufferPools::Get();
    if (thread_local_pools)
    {
      typename optimized::tThreadLocalBufferPools::tBufferPointer buffer = thread_local_pools->GetUnusedBuffer(static_cast<tPortBase&>(port).GetCheaplyCopyableTypeIndex());
      buffer->SetTimestamp(timestamp);
      buffer->GetObject().DeepCopyFrom(data);
      common::tPublishOperation<optimized::tCheapCopyPort, typename optimized::tCheapCopyPort::tPublishingDataThreadLocalBuffer> publish_operation(buffer.release(), true);
      publish_operation.Execute<false, common::tAbstractDataPort::tChangeStatus::CHANGED, false>(static_cast<tPortBase&>(port));
    }
    else
    {
      typename optimized::tCheapCopyPort::tUnusedManagerPointer buffer(optimized::tGlobalBufferPools::Instance().GetUnusedBuffer(static_cast<tPortBase&>(port).GetCheaplyCopyableTypeIndex()).release());
      buffer->SetTimestamp(timestamp);
      buffer->GetObject().DeepCopyFrom(data);
      common::tPublishOperation<optimized::tCheapCopyPort, typename optimized::tCheapCopyPort::tPublishingDataGlobalBuffer> publish_operation(buffer);
      publish_operation.Execute<false, common::tAbstractDataPort::tChangeStatus::CHANGED, false>(static_cast<tPortBase&>(port));
    }
  }

  virtual void SetBounds(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    FINROC_LOG_PRINT(ERROR, "Cannot set bounds for type ", port.GetDataType().GetName());
  }
};

class tGenericPortImplementationStandard : public tGenericPortImplementation
{
public:

  typedef standard::tStandardPort tPortBase;

  virtual core::tAbstractPort* CreatePort(const common::tAbstractDataPortCreationInfo& creation_info)
  {
    return new tPortBase(creation_info);
  }

  virtual void Get(core::tAbstractPort& port, rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp)
  {
    typename tPortBase::tLockingManagerPointer mgr = static_cast<tPortBase&>(port).GetCurrentValueRaw();
    result.DeepCopyFrom(mgr->GetObject());
    timestamp = mgr->GetTimestamp();
  }

  virtual void Publish(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp)
  {
    assert(data.GetType() == port.GetDataType());
    typename tPortBase::tUnusedManagerPointer mgr = static_cast<tPortBase&>(port).GetUnusedBufferRaw();
    mgr->GetObject().DeepCopyFrom(data);
    mgr->SetTimestamp(timestamp);
    static_cast<tPortBase&>(port).Publish(mgr);
  }

  virtual void SetBounds(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    FINROC_LOG_PRINT(ERROR, "Cannot set bounds for type ", port.GetDataType().GetName());
  }
};

template <typename T>
static void CheckCreateImplementationForType(rrlib::rtti::tType type)
{
  if (type.GetRttiName() == typeid(T).name())
  {
    type.AddAnnotation<tGenericPortImplementation>(new internal::tGenericPortImplementationTyped<T>());
  }
}

} // namespace internal

void tGenericPortImplementation::CreateImplementations()
{
  static rrlib::thread::tMutex mutex;
  static int16_t initialized_types = 0;
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
      internal::CheckCreateImplementationForType<long int>(type);
      internal::CheckCreateImplementationForType<long long int>(type);
      internal::CheckCreateImplementationForType<uint8_t>(type);
      internal::CheckCreateImplementationForType<uint16_t>(type);
      internal::CheckCreateImplementationForType<unsigned int>(type);
      internal::CheckCreateImplementationForType<unsigned long int>(type);
      internal::CheckCreateImplementationForType<unsigned long long int>(type);
      internal::CheckCreateImplementationForType<double>(type);
      internal::CheckCreateImplementationForType<float>(type);
      internal::CheckCreateImplementationForType<numeric::tNumber>(type);

      if (!type.GetAnnotation<tGenericPortImplementation>())
      {
        assert((type.GetTypeTraits() & rrlib::rtti::trait_flags::cIS_INTEGRAL) == 0);
        if (IsCheaplyCopiedType(type))
        {
          type.AddAnnotation<tGenericPortImplementation>(new internal::tGenericPortImplementationCheapCopy());
        }
        else
        {
          type.AddAnnotation<tGenericPortImplementation>(new internal::tGenericPortImplementationStandard());
        }
      }
    }
  }
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

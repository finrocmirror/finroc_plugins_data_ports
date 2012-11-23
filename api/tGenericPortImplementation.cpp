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
template <typename T>
class tGenericPortImplementationTyped : public tGenericPortImplementation
{
public:
  tPort<T> port;

  tGenericPortImplementationTyped(const common::tAbstractDataPortCreationInfo& pci) :
    port(pci)
  {}

  virtual void Get(rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp)
  {
    port.Get(result.GetData<T>(), timestamp);
  }

  virtual common::tAbstractDataPort* GetWrapped()
  {
    return port.GetWrapped();
  }

  virtual void Publish(const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp)
  {
    port.Publish(data.GetData<T>(), timestamp);
  }

  virtual void SetBounds(const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    FINROC_LOG_PRINT(ERROR, "Cannot set bounds for type ", rrlib::rtti::tDataType<T>().GetName());
  }
};

template <typename T>
class tGenericPortImplementationNumeric : public tGenericPortImplementationTyped<T>
{
public:

  tGenericPortImplementationNumeric(const common::tAbstractDataPortCreationInfo& pci) :
    tGenericPortImplementationTyped<T>(pci)
  {}

  virtual void SetBounds(const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    tGenericPortImplementationTyped<T>::port.SetBounds(tBounds<T>(min.GetData<T>(), max.GetData<T>()));
  }
};

class tGenericPortImplementationCheapCopy : public tGenericPortImplementation
{
public:
  optimized::tCheapCopyPort* port;

  tGenericPortImplementationCheapCopy(const common::tAbstractDataPortCreationInfo& creation_info) :
    port(new optimized::tCheapCopyPort(creation_info))
  {
    if (creation_info.DefaultValueSet())
    {
      std::unique_ptr<rrlib::rtti::tGenericObject> temp(creation_info.data_type.CreateInstanceGeneric());
      rrlib::serialization::tInputStream is(&creation_info.GetDefaultGeneric());
      is >> (*temp);
      port->SetDefault(*temp);

      // publish for value caching in Parameter classes
      rrlib::serialization::tInputStream is2(&creation_info.GetDefaultGeneric());
      optimized::tCheapCopyPort::tUnusedManagerPointer buffer_manager_pointer(
        optimized::tGlobalBufferPools::Instance().GetUnusedBuffer(port->GetCheaplyCopyableTypeIndex()).release());
      is2 >> buffer_manager_pointer->GetObject();
      tString error = port->BrowserPublishRaw(buffer_manager_pointer);
      if (error.length() > 0)
      {
        FINROC_LOG_PRINT(WARNING, "Could not set default value: ", error);
      }
    }
  }

  virtual void Get(rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp)
  {
    port->CopyCurrentValueToGenericObject(result, timestamp);
  }

  virtual common::tAbstractDataPort* GetWrapped()
  {
    return port;
  }

  virtual void Publish(const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp)
  {
    assert(data.GetType() == port->GetDataType());
    optimized::tThreadLocalBufferPools* thread_local_pools = optimized::tThreadLocalBufferPools::Get();
    if (thread_local_pools)
    {
      typename optimized::tThreadLocalBufferPools::tBufferPointer buffer = thread_local_pools->GetUnusedBuffer(port->GetCheaplyCopyableTypeIndex());
      buffer->SetTimestamp(timestamp);
      buffer->GetObject().DeepCopyFrom(data);
      common::tPublishOperation<optimized::tCheapCopyPort, typename optimized::tCheapCopyPort::tPublishingDataThreadLocalBuffer> publish_operation(buffer.release(), true);
      publish_operation.Execute<false, common::tAbstractDataPort::tChangeStatus::CHANGED, false>(*port);
    }
    else
    {
      typename optimized::tCheapCopyPort::tUnusedManagerPointer buffer(optimized::tGlobalBufferPools::Instance().GetUnusedBuffer(port->GetCheaplyCopyableTypeIndex()).release());
      buffer->SetTimestamp(timestamp);
      buffer->GetObject().DeepCopyFrom(data);
      common::tPublishOperation<optimized::tCheapCopyPort, typename optimized::tCheapCopyPort::tPublishingDataGlobalBuffer> publish_operation(buffer);
      publish_operation.Execute<false, common::tAbstractDataPort::tChangeStatus::CHANGED, false>(*port);
    }
  }

  virtual void SetBounds(const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    FINROC_LOG_PRINT(ERROR, "Cannot set bounds for type ", port->GetDataType().GetName());
  }
};

class tGenericPortImplementationStandard : public tGenericPortImplementation
{
public:
  standard::tStandardPort* port;

  tGenericPortImplementationStandard(const common::tAbstractDataPortCreationInfo& creation_info) :
    port(new standard::tStandardPort(creation_info))
  {
    if (creation_info.DefaultValueSet())
    {
      rrlib::serialization::tInputStream is(&creation_info.GetDefaultGeneric());
      is >> (port->GetDefaultBufferRaw());
    }
  }

  virtual void Get(rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp)
  {
    typename standard::tStandardPort::tLockingManagerPointer mgr = port->GetCurrentValueRaw();
    result.DeepCopyFrom(mgr->GetObject());
    timestamp = mgr->GetTimestamp();
  }

  virtual common::tAbstractDataPort* GetWrapped()
  {
    return port;
  }

  virtual void Publish(const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp)
  {
    assert(data.GetType() == port->GetDataType());
    typename standard::tStandardPort::tUnusedManagerPointer mgr = port->GetUnusedBufferRaw();
    mgr->GetObject().DeepCopyFrom(data);
    mgr->SetTimestamp(timestamp);
    port->Publish(mgr);
  }

  virtual void SetBounds(const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    FINROC_LOG_PRINT(ERROR, "Cannot set bounds for type ", port->GetDataType().GetName());
  }
};

tGenericPortImplementation* tGenericPortImplementation::CreatePortImplementation(const common::tAbstractDataPortCreationInfo& creation_info)
{
  assert(creation_info.data_type != NULL);
  int t = creation_info.data_type.GetTypeTraits();
  if ((t & rrlib::rtti::trait_flags::cIS_INTEGRAL) || (t & rrlib::rtti::trait_flags::cIS_FLOATING_POINT))
  {
    if (creation_info.data_type.GetRttiName() == typeid(int8_t).name())
    {
      return new tGenericPortImplementationNumeric<int8_t>(creation_info);
    }
    else if (creation_info.data_type.GetRttiName() == typeid(int16_t).name())
    {
      return new tGenericPortImplementationNumeric<int16_t>(creation_info);
    }
    else if (creation_info.data_type.GetRttiName() == typeid(int32_t).name())
    {
      return new tGenericPortImplementationNumeric<int32_t>(creation_info);
    }
    else if (creation_info.data_type.GetRttiName() == typeid(int64_t).name())
    {
      return new tGenericPortImplementationNumeric<int64_t>(creation_info);
    }
    else if (creation_info.data_type.GetRttiName() == typeid(uint8_t).name())
    {
      return new tGenericPortImplementationNumeric<uint8_t>(creation_info);
    }
    else if (creation_info.data_type.GetRttiName() == typeid(uint16_t).name())
    {
      return new tGenericPortImplementationNumeric<uint16_t>(creation_info);
    }
    else if (creation_info.data_type.GetRttiName() == typeid(uint32_t).name())
    {
      return new tGenericPortImplementationNumeric<uint32_t>(creation_info);
    }
    else if (creation_info.data_type.GetRttiName() == typeid(uint64_t).name())
    {
      return new tGenericPortImplementationNumeric<uint64_t>(creation_info);
    }
    else if (creation_info.data_type.GetRttiName() == typeid(float).name())
    {
      return new tGenericPortImplementationNumeric<float>(creation_info);
    }
    else if (creation_info.data_type.GetRttiName() == typeid(double).name())
    {
      return new tGenericPortImplementationNumeric<double>(creation_info);
    }
  }
  else if (creation_info.data_type.GetRttiName() == typeid(std::string).name())
  {
    return new tGenericPortImplementationTyped<std::string>(creation_info);
  }
  else if (creation_info.data_type.GetRttiName() == typeid(tString).name())
  {
    return new tGenericPortImplementationTyped<tString>(creation_info);
  }
  else if (creation_info.data_type.GetRttiName() == typeid(bool).name())
  {
    return new tGenericPortImplementationTyped<bool>(creation_info);
  }
  else if (IsCheaplyCopiedType(creation_info.data_type))
  {
    return new tGenericPortImplementationCheapCopy(creation_info);
  }
  else
  {
    return new tGenericPortImplementationStandard(creation_info);
  }
  FINROC_LOG_PRINT(ERROR, "Cannot create port for type ", creation_info.data_type.GetName());
  return NULL;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

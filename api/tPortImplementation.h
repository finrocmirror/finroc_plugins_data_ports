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
/*!\file    plugins/data_ports/api/tPortImplementation.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-25
 *
 * \brief   Contains tPortImplementation
 *
 * \b tPortImplementation
 *
 * Implements methods in tPort<T> for different kinds of data types T.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__api__tPortImplementation_h__
#define __plugins__data_ports__api__tPortImplementation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tPortImplementationTypeTrait.h"
#include "plugins/data_ports/api/tBoundedPort.h"
#include "plugins/data_ports/tPortDataPointer.h"
#include "plugins/data_ports/numeric/tNumber.h"
#include "plugins/data_ports/optimized/tCheapCopyPort.h"
#include "plugins/data_ports/standard/tStandardPort.h"

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
// Class declaration
//----------------------------------------------------------------------
//! Port implementation for different types
/*!
 * Implements methods in tPort<T> for different kinds of data types T.
 */
template <typename T, tPortImplementationType TYPE>
struct tPortImplementation;

/*!
 * Common implementation for all ports derived from tCheapCopyPort
 */
template <typename TWrapper, typename TBuffer, tPortImplementationType TYPE, bool BOUNDABLE>
struct tCheapCopyPortBaseImplementation
{
  typedef optimized::tCheapCopyPort tPortBase;

  static core::tAbstractPort* CreatePort(tPortCreationInfo<TWrapper> creation_info)
  {
    creation_info.UnsetDefaultValue(); // TBuffer might differ from TWrapper => corrupts default value (it is set later in tPort)
    if (creation_info.BoundsSet())
    {
      return new tBoundedPort<TWrapper, tPortImplementationType::CHEAP_COPY>(creation_info);  // Crashes with release mode in gcc 4.6 :-/
      //return new optimized::tCheapCopyPort(creation_info);
    }
    else
    {
      return new optimized::tCheapCopyPort(creation_info);
    }
  }
};

// for non-boundable types
template <typename TWrapper, typename TBuffer, tPortImplementationType TYPE>
struct tCheapCopyPortBaseImplementation<TWrapper, TBuffer, TYPE, false>
{
  typedef optimized::tCheapCopyPort tPortBase;

  static core::tAbstractPort* CreatePort(tPortCreationInfo<TWrapper> creation_info)
  {
    creation_info.UnsetDefaultValue(); // TBuffer might differ from TWrapper => corrupts default value (it is set later in tPort)
    if (creation_info.BoundsSet())
    {
      FINROC_LOG_PRINT(WARNING, "Bounds are not supported for type '", creation_info.data_type.GetName(), "'. Ignoring.");
    }
    return new optimized::tCheapCopyPort(creation_info);
  }
};


// standard cheap-copy implementation
template <typename T, tPortImplementationType TYPE>
struct tCheapCopyPortImplementation :
  public tCheapCopyPortBaseImplementation<T, T, tPortImplementationType::CHEAP_COPY, tIsBoundable<T>::value>
{
  typedef T tPortBuffer;

  static inline void Assign(T& buffer, const T& value, const tUnit& port_unit)
  {
    buffer = value;
  }

  static inline T ToValue(const T& value, const tUnit& port_unit)
  {
    return value;
  }
};

// numeric cheap-copy implementation
template <typename T>
struct tCheapCopyPortImplementation<T, tPortImplementationType::NUMERIC> :
  public tCheapCopyPortBaseImplementation<T, numeric::tNumber, tPortImplementationType::CHEAP_COPY, true>
{
  typedef numeric::tNumber tPortBuffer;

  static inline void Assign(numeric::tNumber& buffer, T value, const tUnit& port_unit)
  {
    buffer.SetValue(value, port_unit);
  }

  static inline T ToCorrectUnit(const numeric::tNumber& number, const tUnit& port_unit)
  {
    T value = number.Value<T>();
    if (number.GetUnit() != tUnit::cNO_UNIT && port_unit != tUnit::cNO_UNIT && number.GetUnit() != port_unit)
    {
      value = number.GetUnit().ConvertTo(value, port_unit);
    }
    return value;
  }

  static inline T ToValue(const numeric::tNumber& value, const tUnit& port_unit)
  {
    return ToCorrectUnit(value, port_unit);
  }
};

// tNumber cheap-copy implementation
template <>
struct tCheapCopyPortImplementation<numeric::tNumber, tPortImplementationType::NUMERIC> :
  public tCheapCopyPortBaseImplementation<numeric::tNumber, numeric::tNumber, tPortImplementationType::CHEAP_COPY, true>
{
  typedef numeric::tNumber tPortBuffer;

  static inline void Assign(numeric::tNumber& buffer, const numeric::tNumber value, const tUnit& port_unit)
  {
    buffer = value;
    buffer.SetUnit(port_unit);
  }

  static inline numeric::tNumber ToCorrectUnit(const numeric::tNumber& number, const tUnit& port_unit)
  {
    if (number.GetUnit() != tUnit::cNO_UNIT && port_unit != tUnit::cNO_UNIT && number.GetUnit() != port_unit)
    {
      return numeric::tNumber(number.GetUnit().ConvertTo(number.Value<double>(), port_unit), port_unit);
    }
    return number;
  }

  static inline numeric::tNumber ToValue(const numeric::tNumber& value, const tUnit& port_unit)
  {
    return ToCorrectUnit(value, port_unit);
  }
};

// implementation for all type handled by tCheapCopyPort port implementation
template <typename T, tPortImplementationType TYPE>
struct tPortImplementation : public tCheapCopyPortImplementation<T, TYPE>
{
  typedef tCheapCopyPortImplementation<T, TYPE> tBase;

  static inline void BrowserPublish(optimized::tCheapCopyPort& port, const T& data, const rrlib::time::tTimestamp& timestamp)
  {
    typename optimized::tCheapCopyPort::tUnusedManagerPointer buffer(optimized::tGlobalBufferPools::Instance().GetUnusedBuffer(port.GetCheaplyCopyableTypeIndex()).release());
    buffer->SetTimestamp(timestamp);
    tBase::Assign(buffer->GetObject().GetData<typename tBase::tPortBuffer>(), data, port.GetUnit());
    port.BrowserPublishRaw(buffer);
  }

  static inline void CopyAndPublish(optimized::tCheapCopyPort& port, const T& data, const rrlib::time::tTimestamp& timestamp)
  {
    optimized::tThreadLocalBufferPools* thread_local_pools = optimized::tThreadLocalBufferPools::Get();
    if (thread_local_pools)
    {
      typename optimized::tThreadLocalBufferPools::tBufferPointer buffer = thread_local_pools->GetUnusedBuffer(port.GetCheaplyCopyableTypeIndex());
      buffer->SetTimestamp(timestamp);
      tBase::Assign(buffer->GetObject().GetData<typename tBase::tPortBuffer>(), data, port.GetUnit());
      common::tPublishOperation<optimized::tCheapCopyPort, typename optimized::tCheapCopyPort::tPublishingDataThreadLocalBuffer> publish_operation(buffer.release(), true);
      publish_operation.Execute<false, common::tAbstractDataPort::tChangeStatus::CHANGED, false, false>(port);
    }
    else
    {
      typename optimized::tCheapCopyPort::tUnusedManagerPointer buffer(optimized::tGlobalBufferPools::Instance().GetUnusedBuffer(port.GetCheaplyCopyableTypeIndex()).release());
      buffer->SetTimestamp(timestamp);
      tBase::Assign(buffer->GetObject().GetData<typename tBase::tPortBuffer>(), data, port.GetUnit());
      common::tPublishOperation<optimized::tCheapCopyPort, typename optimized::tCheapCopyPort::tPublishingDataGlobalBuffer> publish_operation(buffer);
      publish_operation.Execute<false, common::tAbstractDataPort::tChangeStatus::CHANGED, false, false>(port);
    }
  }

  static inline void CopyCurrentPortValue(optimized::tCheapCopyPort& port, T& result_buffer, rrlib::time::tTimestamp& timestamp_buffer)
  {
    typename tBase::tPortBuffer temp_buffer;
    port.CopyCurrentValue(temp_buffer, timestamp_buffer);
    result_buffer = tBase::ToValue(temp_buffer, port.GetUnit());
  }

  static inline tPortDataPointer<const T> GetPointer(optimized::tCheapCopyPort& port)
  {
    typename tBase::tPortBuffer buffer;
    rrlib::time::tTimestamp timestamp;
    port.CopyCurrentValue(buffer, timestamp);
    return tPortDataPointerImplementation<T, true>(tBase::ToValue(buffer, port.GetUnit()), timestamp);
  }

  static inline tPortDataPointer<T> GetUnusedBuffer(optimized::tCheapCopyPort& port)
  {
    return tPortDataPointerImplementation<T, true>();
  }

  template <typename TPointer>
  static inline void Publish(optimized::tCheapCopyPort& port, TPointer && data)
  {
    CopyAndPublish(port, *data, data.GetTimestamp());
  }

  template <typename TPointer>
  static inline void PublishConstBuffer(optimized::tCheapCopyPort& port, TPointer && data)
  {
    CopyAndPublish(port, *data, data.GetTimestamp());
  }

  static void SetDefault(optimized::tCheapCopyPort& port, const T& new_default)
  {
    typename tBase::tPortBuffer buffer;
    tBase::Assign(buffer, new_default, port.GetUnit());
    rrlib::rtti::tGenericObjectWrapper<typename tBase::tPortBuffer> wrapper(buffer);
    port.SetDefault(wrapper);
  }
};

// implementation for standard types
template <typename T>
struct tPortImplementation<T, tPortImplementationType::STANDARD>
{
  typedef standard::tStandardPort tPortBase;
  typedef T tPortBuffer;

  static inline void BrowserPublish(tPortBase& port, const T& data, const rrlib::time::tTimestamp& timestamp)
  {
    typename tPortBase::tUnusedManagerPointer buffer = port.GetUnusedBufferRaw();
    buffer->SetTimestamp(timestamp);
    rrlib::rtti::sStaticTypeInfo<T>::DeepCopy(data, buffer->GetObject().GetData<T>());
    port.BrowserPublish(buffer);
  }

  static inline void CopyAndPublish(tPortBase& port, const T& data, const rrlib::time::tTimestamp& timestamp)
  {
    typename tPortBase::tUnusedManagerPointer buffer = port.GetUnusedBufferRaw();
    buffer->SetTimestamp(timestamp);
    rrlib::rtti::sStaticTypeInfo<T>::DeepCopy(data, buffer->GetObject().GetData<T>());
    port.Publish(buffer);
  }

  static inline void CopyCurrentPortValue(tPortBase& port, T& result_buffer, rrlib::time::tTimestamp& timestamp_buffer)
  {
    typename standard::tStandardPort::tLockingManagerPointer pointer = port.GetCurrentValueRaw();
    rrlib::rtti::sStaticTypeInfo<T>::DeepCopy(pointer->GetObject().GetData<T>(), result_buffer);
    timestamp_buffer = pointer->GetTimestamp();
  }

  static core::tAbstractPort* CreatePort(tPortCreationInfo<T>& pci)
  {
    if (pci.BoundsSet())
    {
      FINROC_LOG_PRINT(WARNING, "Bounds are not supported for type '", pci.data_type.GetName(), "'. Ignoring.");
    }
    return new standard::tStandardPort(pci);
  }

  static inline tPortDataPointer<const T> GetPointer(standard::tStandardPort& port)
  {
    auto buffer_pointer = port.GetCurrentValueRaw();
    return tPortDataPointer<const T>(buffer_pointer, port);
  }

  static inline tPortDataPointer<T> GetUnusedBuffer(standard::tStandardPort& port)
  {
    auto buffer_pointer = port.GetUnusedBufferRaw();
    return tPortDataPointerImplementation<T, false>(buffer_pointer);
  }

  template <typename TPointer>
  static inline void Publish(standard::tStandardPort& port, TPointer && data)
  {
    typename tPortBase::tUnusedManagerPointer buffer(data.implementation.Release());
    assert(buffer->IsUnused());
    port.Publish(buffer);
  }

  template <typename TPointer>
  static inline void PublishConstBuffer(standard::tStandardPort& port, TPointer && data)
  {
    typename tPortBase::tLockingManagerPointer buffer(data.implementation.Release());
    assert(!buffer->IsUnused());
    port.Publish(buffer);
  }

  static void SetDefault(standard::tStandardPort& port, const T& new_default)
  {
    rrlib::rtti::sStaticTypeInfo<T>::DeepCopy(new_default, port.GetDefaultBufferRaw().GetData<T>());
  }
};


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

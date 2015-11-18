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
/*!\file    plugins/data_ports/api/tBoundedPort.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-17
 *
 * \brief   Contains tBoundedPort
 *
 * \b tBoundedPort
 *
 * Port with upper and lower bounds for values.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__api__tBoundedPort_h__
#define __plugins__data_ports__api__tBoundedPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tPortCreationInfo.h"
#include "plugins/data_ports/optimized/tCheapCopyPort.h"
#include "plugins/data_ports/api/tSingleThreadedCheapCopyPort.h"

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
template <typename T, tPortImplementationType TYPE>
struct tPortImplementation;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Bounded port
/*!
 * Port with upper and lower bounds for values.
 */
template <typename T, tPortImplementationType TYPE>
class tBoundedPort : public std::conditional<definitions::cSINGLE_THREADED, api::tSingleThreadedCheapCopyPort<T>, optimized::tCheapCopyPort>::type
{
  static_assert((!std::is_integral<T>::value) || TYPE == tPortImplementationType::NUMERIC || definitions::cSINGLE_THREADED, "Type must be numeric for numeric type");

  typedef typename std::conditional<TYPE == tPortImplementationType::NUMERIC, numeric::tNumber, T>::type tBufferType;
  typedef tPortImplementation<T, TYPE> tImplementationVariation;
  typedef typename std::conditional<definitions::cSINGLE_THREADED, api::tSingleThreadedCheapCopyPort<T>, optimized::tCheapCopyPort>::type tPortBase;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tBoundedPort(const tPortCreationInfo<T>& creation_info) :
    tPortBase(AdjustCreationInfo(creation_info)),
    bounds(creation_info.GetBounds())
  {
  }

  /*!
   * \return the bounds of this port
   */
  inline tBounds<T> GetBounds() const
  {
    return bounds;
  }

  /*!
   * Set Bounds
   * (may only be done before port is initialized - due to thread-safety)
   *
   * \param new_bounds New Bounds for this port
   */
  inline void SetBounds(const tBounds<T>& new_bounds)
  {
    if (this->IsReady())
    {
      FINROC_LOG_PRINT(WARNING, "Port has already been initialized. Cannot change bounds.");
      return;
    }
    bounds = new_bounds;
#ifndef RRLIB_SINGLE_THREADED
    tBufferType value_buffer = tBufferType();
    this->CopyCurrentValue(value_buffer, tStrategy::NEVER_PULL);
    T value = tImplementationVariation::ToValue(value_buffer);
    if (!bounds.InBounds(value))
    {
      typename tPortBase::tUnusedManagerPointer new_buffer(optimized::tGlobalBufferPools::Instance().GetUnusedBuffer(this->GetCheaplyCopyableTypeIndex()).release());
      tImplementationVariation::Assign(new_buffer->GetObject().template GetData<tBufferType>(), bounds.GetOutOfBoundsDefault());
      this->BrowserPublishRaw(new_buffer); // If port is already connected, could this have undesirable side-effects? (I do not think so - otherwise we need to do something more sophisticated here)
    }
#else
    T value = this->CurrentValue();
    if (!bounds.InBounds(value))
    {
      T new_value = bounds.GetOutOfBoundsDefault();
      BrowserPublishRaw(rrlib::rtti::tGenericObjectWrapper<T>(new_value), rrlib::time::cNO_TIME);
    }
#endif
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Bounds of this port */
  tBounds<T> bounds;

  /*!
   * Make sure non-standard assign flag is set
   */
  inline static tPortCreationInfo<T> AdjustCreationInfo(tPortCreationInfo<T> creation_info)
  {
    creation_info.flags |= core::tFrameworkElement::tFlag::NON_STANDARD_ASSIGN;
    return creation_info;
  }

#ifndef RRLIB_SINGLE_THREADED
  virtual std::string BrowserPublishRaw(typename tPortBase::tUnusedManagerPointer& buffer, bool notify_listener_on_this_port = true,
                                        tChangeStatus change_constant = tChangeStatus::CHANGED) override
  {
    if (buffer->GetObject().GetType() != this->GetDataType())
    {
      return "Buffer has wrong type";
    }
    const tBufferType& value_buffer = buffer->GetObject().template GetData<tBufferType>();
    T value = tImplementationVariation::ToValue(value_buffer);
    if (!bounds.InBounds(value))
    {
      return GenerateErrorMessage(value);
    }
    return tPortBase::BrowserPublishRaw(buffer, notify_listener_on_this_port, change_constant);
  }
#else
  virtual std::string BrowserPublishRaw(const rrlib::rtti::tGenericObject& buffer, rrlib::time::tTimestamp timestamp,
                                        bool notify_listener_on_this_port = true, tChangeStatus change_constant = tChangeStatus::CHANGED) override
  {
    if (buffer.GetType() != this->GetDataType())
    {
      return "Buffer has wrong type";
    }
    T value = buffer.template GetData<T>();
    if (!bounds.InBounds(value))
    {
      return GenerateErrorMessage(value);
    }
    return tPortBase::BrowserPublishRaw(buffer, timestamp, notify_listener_on_this_port, change_constant);
  }

#endif

  /*!
   * Generates error message for BrowserPublishRaw
   *
   * \param current_vale Value to generate out of bounds error message for
   * \return Error message that value is out of bounds
   */
  template <bool ENABLE = rrlib::serialization::IsStringSerializable<T>::value>
  std::string GenerateErrorMessage(const typename std::enable_if<ENABLE, T>::type& current_value)
  {
    rrlib::serialization::tStringOutputStream sos;
    sos << "Value " << current_value << " is out of bounds [" << bounds.GetMin() << "; " << bounds.GetMax() << "]";
    return sos.ToString();
  }
  template <bool ENABLE = rrlib::serialization::IsStringSerializable<T>::value>
  std::string GenerateErrorMessage(const typename std::enable_if < !ENABLE, T >::type& current_value)
  {
    return "Value is out of bounds";
  }

#ifndef RRLIB_SINGLE_THREADED
  virtual bool NonStandardAssign(typename tPortBase::tPublishingDataGlobalBuffer& publishing_data, tChangeStatus change_constant) override
  {
    return NonStandardAssignImplementation(publishing_data, change_constant);
  }

  virtual bool NonStandardAssign(typename tPortBase::tPublishingDataThreadLocalBuffer& publishing_data, tChangeStatus change_constant) override
  {
    return NonStandardAssignImplementation(publishing_data, change_constant);
  }


  template <typename TPublishingData>
  bool NonStandardAssignImplementation(TPublishingData& publishing_data, tChangeStatus change_constant)
  {
    const tBufferType& value_buffer = publishing_data.published_buffer->GetObject().template GetData<tBufferType>();
    T value = tImplementationVariation::ToValue(value_buffer);
    if (!bounds.InBounds(value))
    {
      if (bounds.GetOutOfBoundsAction() == tOutOfBoundsAction::DISCARD)
      {
        return false;
      }
      rrlib::time::tTimestamp timestamp = publishing_data.published_buffer->GetTimestamp();
      typename tPortBase::tUnusedManagerPointer buffer = this->GetUnusedBuffer(publishing_data);
      publishing_data.Init(buffer);
      tImplementationVariation::Assign(publishing_data.published_buffer->GetObject().template GetData<tBufferType>(),
                                       bounds.GetOutOfBoundsAction() == tOutOfBoundsAction::ADJUST_TO_RANGE ? bounds.ToBounds(value) : bounds.GetOutOfBoundsDefault());
      publishing_data.published_buffer->SetTimestamp(timestamp);
    }
    return tPortBase::NonStandardAssign(publishing_data, change_constant);
    // tCheapCopyPort::Assign(publishing_data); done anyway
  }

#else

  virtual bool NonStandardAssign(typename tPortBase::tPublishingData& publishing_data, tChangeStatus change_constant) override
  {
    T value = publishing_data.template Value<T>();
    if (!bounds.InBounds(value))
    {
      if (bounds.GetOutOfBoundsAction() == tOutOfBoundsAction::DISCARD)
      {
        return false;
      }

      *static_cast<T*>(this->current_value.data_pointer) = bounds.GetOutOfBoundsAction() == tOutOfBoundsAction::ADJUST_TO_RANGE ? bounds.ToBounds(value) : bounds.GetOutOfBoundsDefault();
      this->current_value.timestamp = publishing_data.value->timestamp;
      publishing_data.value = &this->current_value;
    }
    return tPortBase::NonStandardAssign(publishing_data, change_constant);
  }

#endif
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

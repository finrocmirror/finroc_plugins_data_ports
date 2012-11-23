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
class tBoundedPort : public optimized::tCheapCopyPort
{
  typedef typename std::conditional<TYPE == tPortImplementationType::NUMERIC, numeric::tNumber, T>::type tBufferType;
  typedef tPortImplementation<T, TYPE> tImplementationVariation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tBoundedPort(const tPortCreationInfo<T>& creation_info) :
    optimized::tCheapCopyPort(AdjustCreationInfo(creation_info)),
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
    if (IsReady())
    {
      FINROC_LOG_PRINT(WARNING, "Port has already been initialized. Cannot change bounds.");
      return;
    }
    bounds = new_bounds;
    tBufferType value_buffer = tBufferType();
    CopyCurrentValue(value_buffer, true);
    T value = tImplementationVariation::ToValue(value_buffer, GetUnit());
    if (!bounds.InBounds(value))
    {
      tUnusedManagerPointer new_buffer(optimized::tGlobalBufferPools::Instance().GetUnusedBuffer(GetCheaplyCopyableTypeIndex()).release());
      tImplementationVariation::Assign(new_buffer->GetObject().GetData<tBufferType>(), bounds.GetOutOfBoundsDefault(), GetUnit());
      BrowserPublishRaw(new_buffer); // If port is already connected, could this have undesirable side-effects? (I do not think so - otherwise we need to do something more sophisticated here)
    }
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
    creation_info.flags |= tFrameworkElement::tFlag::NON_STANDARD_ASSIGN;
    return creation_info;
  }

  virtual std::string BrowserPublishRaw(tUnusedManagerPointer& buffer) // FIXME: explicitly override when we use gcc 4.7
  {
    if (buffer->GetObject().GetType() != GetDataType())
    {
      return "Buffer has wrong type";
    }
    const tBufferType& value_buffer = buffer->GetObject().GetData<tBufferType>();
    T value = tImplementationVariation::ToValue(value_buffer, GetUnit());
    if (!bounds.InBounds(value))
    {
      return GenerateErrorMessage(value);
    }
    return tCheapCopyPort::BrowserPublishRaw(buffer);
  }

  /*!
   * Generates error message for BrowserPublishRaw
   *
   * \param current_vale Value to generate out of bounds error message for
   * \return Error message that value is out of bounds
   */
  template < bool ENABLE = rrlib::serialization::tIsStringSerializable<T>::value>
  std::string GenerateErrorMessage(const typename std::enable_if<ENABLE, T>::type& current_value)
  {
    rrlib::serialization::tStringOutputStream sos;
    sos << "Value " << current_value << " is out of bounds [" << bounds.GetMin() << "; " << bounds.GetMax() << "]";
    return sos.ToString();
  }
  template < bool ENABLE = rrlib::serialization::tIsStringSerializable<T>::value>
  std::string GenerateErrorMessage(const typename std::enable_if < !ENABLE, T >::type& current_value)
  {
    return "Value is out of bounds";
  }

  virtual bool NonStandardAssign(tPublishingDataGlobalBuffer& publishing_data) // FIXME: explicitly override when we use gcc 4.7
  {
    return NonStandardAssignImplementation(publishing_data);
  }

  virtual bool NonStandardAssign(tPublishingDataThreadLocalBuffer& publishing_data) // FIXME: explicitly override when we use gcc 4.7
  {
    return NonStandardAssignImplementation(publishing_data);
  }

  template <typename TPublishingData>
  bool NonStandardAssignImplementation(TPublishingData& publishing_data)
  {
    const tBufferType& value_buffer = publishing_data.published_buffer->GetObject().template GetData<tBufferType>();
    T value = tImplementationVariation::ToValue(value_buffer, GetUnit());
    if (!bounds.InBounds(value))
    {
      if (bounds.GetOutOfBoundsAction() == tOutOfBoundsAction::DISCARD)
      {
        return false;
      }
      rrlib::time::tTimestamp timestamp = publishing_data.published_buffer->GetTimestamp();
      tUnusedManagerPointer buffer = GetUnusedBuffer(publishing_data);
      publishing_data.Init(buffer);
      tImplementationVariation::Assign(publishing_data.published_buffer->GetObject().template GetData<tBufferType>(),
                                       bounds.GetOutOfBoundsAction() == tOutOfBoundsAction::ADJUST_TO_RANGE ? bounds.ToBounds(value) : bounds.GetOutOfBoundsDefault(), GetUnit());
      publishing_data.published_buffer->SetTimestamp(timestamp);
    }
    return tCheapCopyPort::NonStandardAssign(publishing_data);
    // tCheapCopyPort::Assign(publishing_data); done anyway
  }
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

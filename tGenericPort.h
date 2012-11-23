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
/*!\file    plugins/data_ports/tGenericPort.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 * \brief   Contains tGenericPort
 *
 * \b tGenericPort
 *
 * Wrapper class for ports whose type is not known at compile time.
 *
 * (note: Get and Publish are currently only implemented based on deep-copying data.
 *  Therefore, Generic ports are only efficient for data types that can be copied cheaply)
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tGenericPort_h__
#define __plugins__data_ports__tGenericPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tGenericPortImplementation.h"

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace data_ports
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Generic port
/*!
 * Wrapper class for ports whose type is not known at compile time.
 *
 * (note: Get and Publish are currently only implemented based on deep-copying data.
 *  Therefore, Generic ports are only efficient for data types that can be copied cheaply)
 */
class tGenericPort : public tPortWrapperBase
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Empty constructor in case port is created later
   */
  tGenericPort() :
    implementation()
  {}

  /*!
   * Constructor takes variadic argument list... just any properties you want to assign to port.
   *
   * The first string is interpreted as port name, the second possibly as config entry (relevant for parameters only).
   * A framework element pointer is interpreted as parent.
   * unsigned int arguments are interpreted as flags.
   * int argument is interpreted as queue length.
   * tBounds<T> are port's bounds.
   * tUnit argument is port's unit.
   * int16/short argument is interpreted as minimum network update interval.
   * tPortCreationBase argument is copied. This is only allowed as first argument.
   */
  template <typename ... ARGS>
  tGenericPort(const ARGS&... args)
  {
    common::tAbstractDataPortCreationInfo creation_info(args...);
    implementation.reset(tGenericPortImplementation::CreatePortImplementation(creation_info));
    wrapped = implementation->GetWrapped();
  }

  /*!
   * Gets Port's current value
   *
   * (Using this get()-variant is more efficient when using CC types, but can be extremely costly with large data types)
   *
   * \param result Buffer to (deep) copy port's current value to
   * \param timestamp Buffer to copy timestamp attached to data to (optional)
   */
  inline const void Get(rrlib::rtti::tGenericObject& result)
  {
    rrlib::time::tTimestamp timestamp;
    Get(result, timestamp);
  }
  inline const void Get(rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp)
  {
    implementation->Get(result, timestamp);
  }

  /*!
   * Publish Data Buffer. This data will be forwarded to any connected ports.
   * Should only be called on output ports.
   *
   * (This publish() method is efficient when using CC types, but can be extremely costly with large data types)
   *
   * \param data Data to publish. It will be deep-copied.
   * \param timestamp Timestamp to attach to data.
   */
  inline void Publish(const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp = rrlib::time::cNO_TIME)
  {
    implementation->Publish(data, timestamp);
  }

  /*!
   * Set new bounds
   * (This is not thread-safe and must only be done in "pause mode")
   *
   * \param b New Bounds
   */
  inline void SetBounds(const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    implementation->SetBounds(min, max);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /** Port implementation */
  std::shared_ptr<api::tGenericPortImplementation> implementation;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

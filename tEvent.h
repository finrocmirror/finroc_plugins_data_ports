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
/*!\file    plugins/data_ports/tEvent.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-07-07
 *
 * \brief   Contains tEvent
 *
 * \b tEvent
 *
 * Simple data type that signals that an event has occured.
 * Event data types clearly indicate that ports are not used for transferring a
 * continuous flow of data, but only occasional events.
 * This event type without any attached data can be used to e.g. trigger certain
 * actions.
 * Furthermore, it can be used as base type for more complex event types.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tEvent_h__
#define __plugins__data_ports__tEvent_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

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
//! Event data type
/*!
 * Simple data type that signals that an event has occured.
 * Event data types clearly indicate that ports are not used for transferring a
 * continuous flow of data, but only occasional events.
 * This event type without any attached data can be used to e.g. trigger certain
 * actions.
 * Furthermore, it can be used as base type for more complex event types.
 */
class tEvent
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tEvent() {}

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

};

inline rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tEvent& number)
{
  return stream;
}

inline rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tEvent& number)
{
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

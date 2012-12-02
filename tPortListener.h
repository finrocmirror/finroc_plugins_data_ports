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
/*!\file    plugins/data_ports/tPortListener.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-21
 *
 * \brief   Contains tPortListener
 *
 * \b tPortListener
 *
 * Can register at port to receive callbacks whenever the port's value changes
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tPortListener_h__
#define __plugins__data_ports__tPortListener_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tPortListenerAdapter.h"

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
//! Port data change listener
/*!
 * Can register at port to receive callbacks whenever the port's value changes.
 *
 * \tparam T If port to register at has type U, then T can be (1) U directly (2) tPortDataPointer<const U> or (3) const void*
 */
template <typename T>
class tPortListener : public api::tPortListenerAdapter<T, api::tPortImplementationTypeTrait<T>::type>
{

  /*!
   * Called whenever port's value has changed
   * Needs to be overridden by subclass
   *
   * \param origin Port that value comes from
   * \param value Port's new value (locked for duration of method call)
   * \param timestamp Timestamp attached to new value
   */
  virtual void PortChanged(common::tAbstractDataPort& origin, const T& value, const rrlib::time::tTimestamp& timestamp) = 0;

};

template <typename T>
class tPortListener<tPortDataPointer<const T>> : public api::tPointerPortListenerAdapter<T>
{

  /*!
   * Called whenever port's value has changed
   * Needs to be overridden by subclass
   *
   * \param origin Port that value comes from
   * \param value Port's new value (locked for duration of method call)
   * \param timestamp Timestamp attached to new value
   */
  virtual void PortChanged(common::tAbstractDataPort& origin, tPortDataPointer<const T>& value, const rrlib::time::tTimestamp& timestamp) = 0;

};

template <>
class tPortListener<const void*> : public api::tVoidPointerPortListenerAdapter
{

  /*!
   * Called whenever port's value has changed
   * Needs to be overridden by subclass
   *
   * \param origin Port that value comes from
   * \param value Port's new value (locked for duration of method call)
   * \param timestamp Timestamp attached to new value
   */
  virtual void PortChanged(common::tAbstractDataPort& origin, const void* value, const rrlib::time::tTimestamp& timestamp) = 0;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

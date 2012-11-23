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
/*!\file    plugins/data_ports/common/tPortListenerRaw.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-21
 *
 * \brief   Contains tPortListenerRaw
 *
 * \b tPortListenerRaw
 *
 * Can register at port to receive callbacks whenever the port's value changes
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__common__tPortListenerRaw_h__
#define __plugins__data_ports__common__tPortListenerRaw_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/buffer_pools/tBufferManagementInfo.h"
#include "rrlib/rtti/rtti.h"
#include "rrlib/time/time.h"

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
namespace common
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
class tAbstractDataPort;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Raw (untyped) port listener
/*!
 * Can register at port to receive callbacks whenever the port's value changes
 */
class tPortListenerRaw
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Called whenever port's value has changed
   *
   * \param origin Port that value comes from
   * \param value Base class of manager of port's new value
   * \param timestamp Timestamp attached to value
   */
  virtual void PortChangedRaw(tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp) = 0;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

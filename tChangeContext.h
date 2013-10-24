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
/*!\file    plugins/data_ports/tChangeContext.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-02-25
 *
 * \brief   Contains tChangeContext
 *
 * \b tChangeContext
 *
 * Contains information on context of a port buffer change
 * (e.g. timestamp, port that changed, type of change)
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tChangeContext_h__
#define __plugins__data_ports__tChangeContext_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tAbstractDataPort.h"

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
//! Information on port buffer change
/*!
 * Contains information on context of a port buffer change
 * (e.g. timestamp, port that changed, type of change)
 */
class tChangeContext
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tChangeContext(common::tAbstractDataPort& origin, const rrlib::time::tTimestamp& timestamp, tChangeStatus change_type) :
    origin(origin),
    timestamp(timestamp),
    change_type(change_type)
  {}

  /*!
   * \return Is this a change from an ordinary publishing operation - or e.g. an initial push?
   */
  tChangeStatus ChangeType()
  {
    return change_type;
  }

  /*!
   * \return Port that value comes from
   */
  common::tAbstractDataPort& Origin()
  {
    return origin;
  }

  /*!
   * \return Timestamp attached to new port value/buffer
   */
  rrlib::time::tTimestamp Timestamp()
  {
    return timestamp;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Port that value comes from */
  common::tAbstractDataPort& origin;

  /*! Timestamp attached to new port value/buffer */
  rrlib::time::tTimestamp timestamp;

  /*! Is this a change from an ordinary publishing operation - or e.g. an initial push? */
  tChangeStatus change_type;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

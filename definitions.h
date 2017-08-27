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
/*!\file    plugins/data_ports/definitions.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-04-04
 *
 * \brief
 *
 * Various definitions for data_ports plugin.
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__definitions_h__
#define __plugins__data_ports__definitions_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/time/time.h"
#include "core/tFrameworkElement.h"

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

/*!
 * Strategy for get operations
 */
enum tStrategy
{
  DEFAULT,                            //!< Use strategy set in port
  NEVER_PULL,                         //!< Do not attempt to pull data - even if port is on pull strategy
  PULL,                               //!< Always pull port data (regardless of port's strategy)
  PULL_IGNORING_HANDLER_ON_THIS_PORT  //!< Always pull port data (regardless of port's strategy). Any pull request handler on this port is ignored.
};

/*! Constants for port's change flag */
enum class tChangeStatus : int8_t
{
  NO_CHANGE,      //!< Port data has not changed since last reset
  CHANGED,        //!< Port data has changed since last reset
  CHANGED_INITIAL //!< Port data has changed since last reset - due to initial pushing on new connection. Also set after port construction.
};

/*! Timeout for pull operations */
constexpr rrlib::time::tDuration cPULL_TIMEOUT = std::chrono::seconds(1);

/*! Default flags for input and output data ports */
constexpr core::tFrameworkElement::tFlags cDEFAULT_INPUT_PORT_FLAGS = core::tFrameworkElement::tFlag::ACCEPTS_DATA | core::tFrameworkElement::tFlag::PUSH_STRATEGY;
constexpr core::tFrameworkElement::tFlags cDEFAULT_OUTPUT_PORT_FLAGS = core::tFrameworkElement::tFlag::EMITS_DATA | core::tFrameworkElement::tFlag::OUTPUT_PORT;

enum { cMAX_SIZE_CHEAPLY_COPIED_TYPES = 256 };  //!< Types bigger than this value (in bytes) are never considered cheaply-copied types

//----------------------------------------------------------------------
// Function declarations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

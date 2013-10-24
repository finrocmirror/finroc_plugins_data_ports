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
/*!\file    plugins/data_ports/tPullRequestHandler.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 * \brief   Contains tPullRequestHandler
 *
 * \b tPullRequestHandler
 *
 * Can be used to handle pull requests of - typically - output ports
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tPullRequestHandler_h__
#define __plugins__data_ports__tPullRequestHandler_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tPullRequestHandlerAdapter.h"
#include "plugins/data_ports/api/tPullRequestHandlerAdapter.h"
#include "plugins/data_ports/standard/tStandardPort.h"

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
//! Port pull request callback
/*!
 * Can be used to handle pull requests of - typically - output ports
 * in a custom way.
 */
template <typename T>
class tPullRequestHandler : public api::tPullRequestHandlerAdapter<T, tIsCheaplyCopiedType<T>::value>
{

  /*!
   * Called whenever a pull request is received
   *
   * \param origin Port that pull request comes from (the port that this handler is attached to)
   * \return Pulled buffer (used or unused) - return NULL if pull request should be handled by port instead (as if no pull handler was present)
   */
  virtual tPortDataPointer<const T> OnPullRequest(tOutputPort<T>& origin) = 0;

  //Note: origin used to have type 'common::tAbstractDataPort'. However, this leads to overloading
  //      conflicts when a class is pull request handler for multiple types)
};

// Generic variant
template <>
class tPullRequestHandler<rrlib::rtti::tGenericObject> : public api::tPullRequestHandlerAdapterGeneric
{

  /*!
   * Called whenever a pull request is received
   *
   * \param origin Port that pull request comes from (the port that this handler is attached to)
   * \return Pulled buffer (used or unused) - return NULL if pull request should be handled by port instead (as if no pull handler was present)
   */
  virtual tPortDataPointer<const rrlib::rtti::tGenericObject> OnPullRequest(tGenericPort& origin) = 0;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

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
/*!\file    plugins/data_ports/standard/tPullRequestHandlerRaw.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-30
 *
 * \brief   Contains tPullRequestHandlerRaw
 *
 * \b tPullRequestHandlerRaw
 *
 * Can be used to handle pull requests of - typically - output ports
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__standard__tPullRequestHandlerRaw_h__
#define __plugins__data_ports__standard__tPullRequestHandlerRaw_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/standard/tStandardPort.h"

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace data_ports
{
namespace standard
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Pull Request handler interface for standard ports.
/*!
 * Can be used to handle pull requests of - typically - output ports
 */
class tPullRequestHandlerRaw : private rrlib::util::tNoncopyable
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Called whenever a pull request is intercepted
   *
   * \param origin (Output) Port that received pull request
   * \return PortData to answer request with
   */
  virtual typename standard::tStandardPort::tUniversalManagerPointer RawPullRequest(tStandardPort& origin) = 0;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

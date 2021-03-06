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
/*!\file    plugins/data_ports/optimized/tPullRequestHandlerRaw.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-04
 *
 * \brief   Contains tPullRequestHandlerRaw
 *
 * \b tPullRequestHandlerRaw
 *
 * Can be used to handle pull requests of - typically - output ports
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__optimized__tPullRequestHandlerRaw_h__
#define __plugins__data_ports__optimized__tPullRequestHandlerRaw_h__

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
namespace optimized
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
class tCheapCopyPort;
class tCheaplyCopiedBufferManager;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Pull Request handler interface for cheap copy ports.
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
   * \param origin (Output) Port pull request comes from
   * \param result_buffer Buffer with result
   * \return Was pull request handled (and result buffer filled) - or should it be handled by port in the standard way (now)?
   */
  virtual bool RawPullRequest(tCheapCopyPort& origin, tCheaplyCopiedBufferManager& result_buffer) = 0;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

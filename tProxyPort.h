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
/*!\file    plugins/data_ports/tProxyPort.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-25
 *
 * \brief   Contains tProxyPort
 *
 * \b tProxyPort
 *
 * This port class is used in applications.
 * It provides a convenient API for creating proxy (or 'routing') ports.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tProxyPort_h__
#define __plugins__data_ports__tProxyPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tPort.h"

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
//! Proxy port
/*!
 * This port class is used in applications.
 * It provides a convenient API for creating proxy (or 'routing') ports.
 *
 * \tparam T T is the data type of the port (see tPort)
 * \tparam OUTPUT_PORT True, if this an output proxy port? (otherwise it is an input proxy port)
 */
template <typename T, bool OUTPUT_PORT>
class tProxyPort : public tPort<T>
{
  typedef core::tPortWrapperBase::tNoArgument tNoArgument;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Creates no wrapped port */
  tProxyPort() {}

  /*!
   * Constructor takes variadic argument list... just any properties you want to assign to port.
   *
   * The first string is interpreted as port name, the second possibly as config entry (relevant for parameters only).
   * A framework element pointer is interpreted as parent.
   * tFrameworkElement::tFlags arguments are interpreted as flags.
   * A tQueueSettings argument creates an input queue with the specified settings.
   * tBounds<T> are port's bounds.
   * tUnit argument is port's unit.
   * const T& is interpreted as port's default value.
   * tPortCreationInfo<T> argument is copied. This is only allowed as first argument.
   *
   * This becomes a little tricky when T is a string type. There we have these rules:
   * The second string argument is interpreted as default_value. The third as config entry.
   */
  template <typename TArg1, typename TArg2, typename ... TRest>
  tProxyPort(const TArg1& arg1, const TArg2& arg2, const TRest&... args) :
    tPort<T>(arg1, arg2, args..., core::tFrameworkElement::tFlag::EMITS_DATA | core::tFrameworkElement::tFlag::ACCEPTS_DATA |
             (OUTPUT_PORT ? core::tFrameworkElement::tFlag::OUTPUT_PORT : core::tFrameworkElement::tFlag::EMITS_DATA))
  {}

  // with a single argument, we do not want catch calls for copy construction
  template < typename TArgument1, bool ENABLE = !std::is_base_of<tProxyPort, TArgument1>::value >
  tProxyPort(const TArgument1& argument1, typename std::enable_if<ENABLE, tNoArgument>::type no_argument = tNoArgument()) :
    tPort<T>(argument1, core::tFrameworkElement::tFlag::EMITS_DATA | core::tFrameworkElement::tFlag::ACCEPTS_DATA |
             (OUTPUT_PORT ? core::tFrameworkElement::tFlag::OUTPUT_PORT : core::tFrameworkElement::tFlag::EMITS_DATA))
  {}

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private :

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------

}
}


#endif

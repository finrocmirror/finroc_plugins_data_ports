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

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Constructor takes variadic argument list... just any properties you want to assign to port.
   *
   * The first string is interpreted as port name, the second possibly as config entry (relevant for parameters only).
   * A framework element pointer is interpreted as parent.
   * tFrameworkElement::tFlags arguments are interpreted as flags.
   * int argument is interpreted as queue length.
   * tBounds<T> are port's bounds.
   * tUnit argument is port's unit.
   * int16/short argument is interpreted as minimum network update interval.
   * const T& is interpreted as port's default value.
   * tPortCreationInfo<T> argument is copied. This is only allowed as first argument.
   *
   * This becomes a little tricky when port has numeric or string type.
   * There we have these rules:
   *
   * string type: The second string argument is interpreted as default_value. The third as config entry.
   * numeric type: The first numeric argument is interpreted as default_value.
   */
  template <typename ... ARGS>
  tProxyPort(const ARGS && ... args) :
    tPort<T>(core::tFrameworkElement::tFlag::EMITS_DATA | core::tFrameworkElement::tFlag::ACCEPTS_DATA |
             (OUTPUT_PORT ? core::tFrameworkElement::tFlag::OUTPUT_PORT : core::tFrameworkElement::tFlag::EMITS_DATA), std::forward<const ARGS>(args)...)
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

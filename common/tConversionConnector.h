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
/*!\file    plugins/data_ports/common/tConversionConnector.h
 *
 * \author  Max Reichardt
 *
 * \date    2016-08-26
 *
 * \brief   Contains tConversionConnector
 *
 * \b tConversionConnector
 *
 * Port connector with type conversion attached
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__common__tConversionConnector_h__
#define __plugins__data_ports__common__tConversionConnector_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/tCompiledConversionOperation.h"
#include "core/port/tConnector.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/definitions.h"

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

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Conversion Connector
/*!
 * Port connector with type conversion attached
 */
class tConversionConnector : public core::tConnector
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param source_port Source Port
   * \param destination_port Destination Port
   * \param connect_options Connect options for this connector
   */
  tConversionConnector(core::tAbstractPort& source_port, core::tAbstractPort& destination_port, const core::tConnectOptions& connect_options);

  ~tConversionConnector();

  /*!
   * Convert and publish data via this connector
   *
   * \param input_data Input data (to convert and publish)
   * \param change_constant Change constant to use
   */
  void Publish(const rrlib::rtti::tGenericObject& input_data, tChangeStatus change_constant) const;

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Compiled conversion operation */
  const rrlib::rtti::conversion::tCompiledConversionOperation conversion_operation;

  /*! Destination port as generic port */
  void* destination_port_generic_memory[2];
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

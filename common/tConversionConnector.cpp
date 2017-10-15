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
/*!\file    plugins/data_ports/common/tConversionConnector.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2016-08-26
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tConversionConnector.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tGenericPort.h"

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
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

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tConversionConnector::tConversionConnector(core::tAbstractPort& source_port, core::tAbstractPort& destination_port, const core::tConnectOptions& connect_options) :
  tConnector(source_port, destination_port, connect_options, conversion_operation),
  conversion_operation(connect_options.conversion_operations.Compile(false, source_port.GetDataType(), destination_port.GetDataType()))
{
  static_assert(sizeof(destination_port_generic_memory) == sizeof(tGenericPort), "Adjust array size");
  tGenericPort destination_port_generic = tGenericPort::Wrap(destination_port);
  memcpy(destination_port_generic_memory, &destination_port_generic, sizeof(tGenericPort));
}

tConversionConnector::~tConversionConnector()
{}

void tConversionConnector::Publish(const rrlib::rtti::tGenericObject& input_data, const rrlib::time::tTimestamp& timestamp, tChangeStatus change_constant) const
{
  try
  {
    tGenericPort& generic_port = const_cast<tGenericPort&>(reinterpret_cast<const tGenericPort&>(destination_port_generic_memory[0]));
    tPortDataPointer<rrlib::rtti::tGenericObject> buffer = generic_port.GetUnusedBuffer();
    buffer.SetTimestamp(timestamp);
    conversion_operation.Convert(input_data, *buffer);
    generic_port.BrowserPublish(buffer, true, change_constant);
  }
  catch (const std::exception& e)
  {
    FINROC_LOG_PRINT_STATIC(WARNING, "Converting data failed between ports '", Source(), "' and '", Destination(), "': ", e);
  }
}


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

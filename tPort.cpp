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
/*!\file    plugins/data_ports/tPort.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-21
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/tPort.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tEvent.h"

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

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

/*! Initializes tEvent data type */
static rrlib::rtti::tDataType<tEvent> cINIT_EVENT_DATA_TYPE("Event");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------
template class tPort<int>;
template class tPort<long long int>;
template class tPort<float>;
template class tPort<double>;
template class tPort<numeric::tNumber>;
template class tPort<std::string>;
template class tPort<bool>;
template class tPort<rrlib::serialization::tMemoryBuffer>;

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

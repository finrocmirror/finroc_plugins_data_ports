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
/*!\file    plugins/data_ports/standard/tPortBufferManager.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-30
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/standard/tPortBufferManager.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

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
namespace standard
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

tPortBufferManager::tPortBufferManager() :
  unused(true),
  derived_from(NULL),
  lock_id(0)
{}

tPortBufferManager* tPortBufferManager::CreateInstance(const rrlib::rtti::tType& type)
{
  static_assert(sizeof(tPortBufferManager) % 8 == 0, "Port Data manager must be aligned to 8 byte boundary");
  char* placement = (char*)operator new(sizeof(tPortBufferManager) + type.GetSize(true));
  type.CreateInstanceGeneric(placement + sizeof(tPortBufferManager));
  return new(placement) tPortBufferManager();
}

rrlib::rtti::tGenericObject& tPortBufferManager::GetObjectImplementation()
{
  return GetObject();
}


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

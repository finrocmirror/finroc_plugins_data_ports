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
/*!\file    plugins/data_ports/optimized/tThreadLocalBufferManager.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-03
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/tThreadLocalBufferManager.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/tThreadLocalBufferPools.h"

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
namespace optimized
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

tThreadLocalBufferManager::tThreadLocalBufferManager() :
  tCheaplyCopiedBufferManager(tThreadLocalBufferPools::Get())
{
  assert(GetThreadLocalOrigin() || definitions::cSINGLE_THREADED);
}

tThreadLocalBufferManager::~tThreadLocalBufferManager()
{
}

tThreadLocalBufferManager* tThreadLocalBufferManager::CreateInstance(uint32_t buffer_size)
{
  static_assert(sizeof(tThreadLocalBufferManager) % 8 == 0, "Port Data manager must be aligned to 8 byte boundary");
  char* placement = (char*)operator new(sizeof(tThreadLocalBufferManager) + sizeof(rrlib::rtti::tGenericObject) + buffer_size);
  memset(placement + sizeof(tThreadLocalBufferManager), 0, buffer_size);
  rrlib::rtti::tDataType<int>().EmplaceGenericObject(placement + sizeof(tCheaplyCopiedBufferManager)).release();  // Type is adjusted later
  return new(placement) tThreadLocalBufferManager();
}


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

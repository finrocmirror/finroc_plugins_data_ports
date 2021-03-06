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
/*!\file    plugins/data_ports/optimized/tCheaplyCopiedBufferManager.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-04
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/tCheaplyCopiedBufferManager.h"

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

tCheaplyCopiedBufferManager::tCheaplyCopiedBufferManager(tThreadLocalBufferPools* origin) :
  reference_counter(0),
  reuse_counter(0),
  origin(origin)
{}

tCheaplyCopiedBufferManager::~tCheaplyCopiedBufferManager()
{
  if (!GetThreadLocalOrigin())
  {
    GetObject().~tGenericObject();
  }
}

tCheaplyCopiedBufferManager* tCheaplyCopiedBufferManager::CreateInstance(const rrlib::rtti::tType& type)
{
  static_assert(sizeof(tCheaplyCopiedBufferManager) % 8 == 0, "Port Data manager must be aligned to 8 byte boundary");
  char* placement = (char*)operator new(sizeof(tCheaplyCopiedBufferManager) + type.GetSize(true));
  type.CreateInstanceGeneric(placement + sizeof(tCheaplyCopiedBufferManager));
  return new(placement) tCheaplyCopiedBufferManager();
}

rrlib::rtti::tGenericObject& tCheaplyCopiedBufferManager::GetObjectImplementation()
{
  return GetObject();
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

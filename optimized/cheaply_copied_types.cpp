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
/*!\file    plugins/data_ports/optimized/cheaply_copied_types.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-03
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/cheaply_copied_types.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/thread/tLock.h"
#include "core/definitions.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/type_traits.h"
#include "plugins/data_ports/optimized/tThreadSpecificBufferPools.h"

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
namespace
{

enum { cPOOL_BUFFER_SIZE_STEP = tThreadSpecificBufferPools<true>::cPOOL_BUFFER_SIZE_STEP };

/*! Register with types */
struct tRegister
{
  /*! Cheaply copied types used in ports */
  std::array < uint32_t, cMAX_SIZE_CHEAPLY_COPIED_TYPES / cPOOL_BUFFER_SIZE_STEP > used_pools;

  tRegister() :
    used_pools()
  {
    used_pools.fill(0);
  }
};

/*!
 * \return Register singleton
 */
tRegister& GetRegister()
{
  static tRegister the_register;
  return the_register;
}

}

uint32_t GetCheaplyCopiedBufferPoolIndex(const rrlib::rtti::tType& type)
{
  uint32_t result = (type.GetSize() <= cPOOL_BUFFER_SIZE_STEP) ? 0 : ((type.GetSize() - 1) / cPOOL_BUFFER_SIZE_STEP);
  assert(result >= 0 && result < cMAX_SIZE_CHEAPLY_COPIED_TYPES / cPOOL_BUFFER_SIZE_STEP);
  return result;
}

size_t GetPortCount(uint32_t cheaply_copied_type_buffer_pool_index)
{
  return GetRegister().used_pools[cheaply_copied_type_buffer_pool_index];
}

uint32_t RegisterPort(const rrlib::rtti::tType& type)
{
  uint32_t result = GetCheaplyCopiedBufferPoolIndex(type);
  GetRegister().used_pools[result]++;
  return result;
}

void UnregisterPort(const rrlib::rtti::tType& type)
{
  GetRegister().used_pools[GetCheaplyCopiedBufferPoolIndex(type)]--;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

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
/*!\file    plugins/data_ports/optimized/tGlobalBufferPools.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-04
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/tGlobalBufferPools.h"

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

typedef rrlib::design_patterns::tSingletonHolder<tGlobalBufferPools, rrlib::design_patterns::singleton::Longevity> tGlobalBufferPoolsInstance;
static inline unsigned int GetLongevity(tGlobalBufferPools*)
{
  return 0xFE000000; // should be deleted before tGarbageFromDeletedBufferPools
}

struct tGlobalBufferPoolsInit
{
  tGlobalBufferPoolsInit()
  {
    tGlobalBufferPoolsInstance::Instance();
  }
};

static tGlobalBufferPoolsInit init_global_buffer_pools;

tGlobalBufferPools* tGlobalBufferPools::instance = NULL;

tGlobalBufferPools::tGlobalBufferPools()
{
  instance = this;
}

tGlobalBufferPools::~tGlobalBufferPools()
{
  instance = NULL;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

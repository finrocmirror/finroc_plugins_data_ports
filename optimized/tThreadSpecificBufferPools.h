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
/*!\file    plugins/data_ports/optimized/tThreadSpecificBufferPools.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-03
 *
 * \brief   Contains tThreadSpecificBufferPools
 *
 * \b tThreadSpecificBufferPools
 *
 * Contains thread-specific buffer pools for all 'cheaply copied' types.
 * These pools can be thread-local.
 * There is also a global instance of this class shared by the remaining threads.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__optimized__tThreadSpecificBufferPools_h__
#define __plugins__data_ports__optimized__tThreadSpecificBufferPools_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/definitions.h"
#include "plugins/data_ports/common/tPortBufferPool.h"
#include "plugins/data_ports/optimized/cheaply_copied_types.h"
#include "plugins/data_ports/optimized/tCheaplyCopiedBufferManager.h"
#include "plugins/data_ports/optimized/tThreadLocalBufferManager.h"

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
// Class declaration
//----------------------------------------------------------------------
//! Thread-specific buffer pools
/*!
 * Contains thread-specific buffer pools for all 'cheaply copied' types.
 * These pools can be thread-local.
 * There is also a global instance of this class shared by the remaining threads.
 *
 * There separate pools for different buffer sizes.
 *
 * \tparam SHARED True if this pool is shared by multiple threads
 */
template <bool SHARED>
class tThreadSpecificBufferPools : public std::conditional<SHARED, rrlib::thread::tMutex, rrlib::thread::tNoMutex>::type
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Type of buffers in pools */
  typedef typename std::conditional<SHARED, tCheaplyCopiedBufferManager, tThreadLocalBufferManager>::type tBufferType;

  /*! Type of ordinary buffer pool */
  typedef common::tPortBufferPool < tBufferType,
          SHARED ? rrlib::concurrent_containers::tConcurrency::FULL : rrlib::concurrent_containers::tConcurrency::NONE > tBufferPool;

  /*! Auto-recycling buffer pointer */
  typedef typename tBufferPool::tPointer tBufferPointer;

  /*! Step size for buffer pool size increase (8 means there is of buffer pool with buffers of 8 byte in size, 16 byte in size, etc.) */
  enum { cPOOL_BUFFER_SIZE_STEP = 8 };


  tThreadSpecificBufferPools() :
    pools()
  {
    if (SHARED)
    {
      AddMissingPools();
    }
  }

  /*!
   * \param buffer_pool_index Pool index of buffer to obtain
   * \param type Desired data type of buffer
   * \return Unused buffer of specified type
   */
  tBufferPointer GetUnusedBuffer(uint32_t buffer_pool_index, const rrlib::rtti::tType& type)
  {
    tBufferPointer result = pools[buffer_pool_index].GetUnusedBuffer((buffer_pool_index + 1) * cPOOL_BUFFER_SIZE_STEP, type);
    result->SetType(type);
    return std::move(result);
  }

//----------------------------------------------------------------------
// Protected fields and methods
//----------------------------------------------------------------------
protected:

  /*! The set of pools (index is buffer size / cPOOL_BUFFER_SIZE_STEP) */
  std::array < tBufferPool, cMAX_SIZE_CHEAPLY_COPIED_TYPES / cPOOL_BUFFER_SIZE_STEP > pools;

  /*!
   * Adds/Initialized buffer pools for existing types
   */
  void AddMissingPools()
  {
    for (uint32_t i = 0; i < pools.size(); i++)
    {
      size_t initial_size = (i == 0) ? 50 : std::min<size_t>(GetPortCount(i), 10); // TODO: add proper heuristics/mechanisms for initial buffer allocation
      pools[i].AllocateAdditionalBuffers((i + 1) * cPOOL_BUFFER_SIZE_STEP, initial_size);
    }
  }
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

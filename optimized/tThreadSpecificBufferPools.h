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


  tThreadSpecificBufferPools() :
    pools()
  {
    AddMissingPools();
  }

  /*!
   * \param cheaply_copied_type_index 'Cheaply copied type index' of buffer to obtain
   * \return Unused buffer of specified type
   */
  tBufferPointer GetUnusedBuffer(uint32_t cheaply_copied_type_index)
  {
    return pools[cheaply_copied_type_index].GetUnusedBuffer(cheaply_copied_type_index);
  }

//----------------------------------------------------------------------
// Protected fields and methods
//----------------------------------------------------------------------
protected:

  /*! The set of pools (index is index in CheaplyCopiedTypeRegister) */
  std::array<tBufferPool, cMAX_CHEAPLY_COPYABLE_TYPES> pools;

  /*!
   * Adds/Initialized buffer pools for existing types
   */
  void AddMissingPools()
  {
    uint32_t type_count = GetRegisteredTypeCount();
    for (uint32_t i = 0; i < type_count; i++)
    {
      size_t initial_size = (i == 0) ? 50 : std::min<size_t>(GetPortCount(i), 10); // TODO: add proper heuristics/mechanisms for initial buffer allocation
      pools[i].AllocateAdditionalBuffers(GetType(i), initial_size);
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

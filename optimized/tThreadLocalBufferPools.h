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
/*!\file    plugins/data_ports/optimized/tThreadLocalBufferPools.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-04
 *
 * \brief   Contains tThreadLocalBufferPools
 *
 * \b tThreadLocalBufferPools
 *
 * Contains thread-local publishing memory and also thread-local buffer pools
 * for all 'cheaply copyable' types used in ports.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__optimized__tThreadLocalBufferPools_h__
#define __plugins__data_ports__optimized__tThreadLocalBufferPools_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/tThreadSpecificBufferPools.h"

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
class tThreadLocalBufferManagement;

namespace optimized
{
namespace internal
{
struct tDeletionList;
}

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Thread-local buffer pools
/*!
 * Contains thread-local buffer pools
 * for all 'cheaply copyable' types used in ports.
 */
class tThreadLocalBufferPools : public tThreadSpecificBufferPools<false>
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Type of queue for buffers returned from other threads */
  typedef rrlib::concurrent_containers::tQueue < tBufferPointer, rrlib::concurrent_containers::tConcurrency::MULTIPLE_WRITERS,
          rrlib::concurrent_containers::tDequeueMode::ALL > tReturnedBufferQueue;

  typedef typename tThreadSpecificBufferPools<false>::tBufferPointer tBufferPointer;

  /*!
   * Buffer pools of current thread - NULL if none has been set (=> use DEFAULT)
   */
  static tThreadLocalBufferPools* Get()
  {
    return thread_local_instance;
  }

  /*!
   * Processes buffers in returned_buffer_queue
   *
   * \return True, if any buffer has been returned and added as unused buffer to a pool.
   */
  bool ProcessReturnedBuffers();

  /*!
   * Returns buffer whose locks have (partly) been released by another thread
   *
   * \return buffer Returned buffer
   */
  void ReturnBufferFromOtherThread(tThreadLocalBufferManager* buffer)
  {
    returned_buffer_queue.Enqueue(tBufferPointer(buffer));
  }

  /*!
   * Safely deletes buffer pools
   */
  void SafeDelete();

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend struct internal::tDeletionList;
  friend class finroc::data_ports::tThreadLocalBufferManagement;

  tThreadLocalBufferPools();

  virtual ~tThreadLocalBufferPools();


  /*! Buffer pools of current thread */
  static __thread tThreadLocalBufferPools* thread_local_instance;

  /*!
   * Queue for buffers returned from other threads
   * (more precisely: buffers with locks released by other threads)
   */
  tReturnedBufferQueue returned_buffer_queue;


  /*!
   * After SafeDelete() has been called on tThreadLocalBufferPools,
   * this is called regularly by garbage deleter until all buffers have been deleted.
   *
   * \param Is this the initial call to this function?
   * \return True if all buffers have been deleted
   */
  bool DeleteAllGarbage(bool initial_call);
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

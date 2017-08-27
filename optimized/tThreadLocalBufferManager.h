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
/*!\file    plugins/data_ports/optimized/tThreadLocalBufferManager.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-03
 *
 * \brief   Contains tThreadLocalBufferManager
 *
 * \b tThreadLocalBufferManager
 *
 * This class manages a single port buffer.
 * It handles information on locks, data type, timestamp etc.
 * It is only published from a single thread.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__optimized__tThreadLocalBufferManager_h__
#define __plugins__data_ports__optimized__tThreadLocalBufferManager_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/tCheaplyCopiedBufferManager.h"

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
class tThreadLocalBufferPools;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Buffer manager managed by a single thread only.
/*!
 * This class manages a single port buffer.
 * It handles information on locks, data type, timestamp etc.
 * It is only published and managed from a single thread.
 * This includes its reference counter.
 */
class tThreadLocalBufferManager : public tCheaplyCopiedBufferManager
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Mask for lowest bits from reuse counter to use in port pointer tag
   * order to avoid the ABA problem
   */
  enum { cTAG_MASK = 0x7 };

  ~tThreadLocalBufferManager();

  /*!
   * Adds Locks from owner thread
   *
   * \param locks_to_add number of locks to add
   */
  inline void AddThreadLocalLocks(int locks_to_add)
  {
    reference_counter += locks_to_add;
  }

  /*!
   * Creates instance of tThreadLocalBufferManager containing a buffer
   * of the specified size
   *
   * \param buffer_size Size of buffer
   * \return Created instance (can be deleted like any other objects using delete operator)
   */
  static tThreadLocalBufferManager* CreateInstance(uint32_t buffer_size);

  /*!
   * \return Pointer tag to use for current buffer publishing operation
   */
  inline int GetPointerTag() const
  {
    return reuse_counter & cTAG_MASK;
  }

  /*!
   * \return Current value of owner thread's reference counter
   */
  inline int GetThreadLocalReferenceCounter() const
  {
    return reference_counter;
  }

  /*!
   * Increments reuse counter
   *
   * \return Pointer tag to use for this buffer publishing operation
   */
  inline int IncrementReuseCounter()
  {
    reuse_counter++;
    return reuse_counter & cTAG_MASK;
  }

  /*!
   * Processes lock releases from other threads
   * Transfers atomic reference counter to thread-local reference counter
   */
  template <typename TDeleterType>
  inline void ProcessLockReleasesFromOtherThreads()
  {
    int old_value = reference_and_reuse_counter.exchange(0) >> 16;
    assert(old_value < 0);
    ReleaseThreadLocalLocks<TDeleterType>(-old_value);
  }

  /*!
   * Releases locks from a thread that does not own this buffer
   *
   * \param locks_to_release number of locks to release
   */
  template <typename TThreadLocalBufferPools = tThreadLocalBufferPools> // template to get rid of cyclic dependency while keeping inline functions
  inline void ReleaseLocksFromOtherThread(int locks_to_release)
  {
    int old_value = reference_and_reuse_counter.fetch_sub(locks_to_release << 16) >> 16;
    if (old_value == 0)
    {
      static_cast<TThreadLocalBufferPools*>(this->GetThreadLocalOrigin())->ReturnBufferFromOtherThread(this);
    }
  }

  /*!
   * Releases locks from owner thread
   *
   * \param locks_to_release number of locks to release
   */
  template <typename TDeleterType>
  inline void ReleaseThreadLocalLocks(int locks_to_release)
  {
    // assert(IsOwnerThread()); - May also be garbage collector or main thread
    reference_counter -= locks_to_release;
    assert(reference_counter >= 0 && "negative reference counter detected");
    if (reference_counter == 0)
    {
      TDeleterType deleter;
      deleter(this);
    }
  }

  /*!
   * Sets data type of managed object
   *
   * \param type Type of object
   */
  void SetType(const rrlib::rtti::tType& type)
  {
    const_cast<rrlib::rtti::tType&>(GetObject().GetType()) = type;
  }

  /*!
   * \return Thread-local reference counter (If additional locks are required during publishing operation, adding to this counter is a safe and efficient way of doing this)
   */
  inline int& ThreadLocalReferenceCounter()
  {
    return reference_counter;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  tThreadLocalBufferManager();

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

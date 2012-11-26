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
/*!\file    plugins/data_ports/common/tReferenceCountingBufferManager.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-03
 *
 * \brief   Contains tReferenceCountingBufferManager
 *
 * \b tReferenceCountingBufferManager
 *
 * This class manages a single port buffer.
 * It handles information on locks, data type, timestamp etc.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__common__tReferenceCountingBufferManager_h__
#define __plugins__data_ports__common__tReferenceCountingBufferManager_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tAbstractPortBufferManager.h"

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace data_ports
{
namespace common
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Manages a port buffer using a concurrent reference and reuse counter.
/*!
 * This class manages a single port buffer.
 * It handles information on locks, data type, timestamp etc.
 */
class tReferenceCountingBufferManager : public tAbstractPortBufferManager<rrlib::concurrent_containers::tQueueability::FULL_OPTIMIZED>
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
  enum { cREUSE_COUNTER_MASK = 0xFFFF };

  /*!
   * Adds Locks
   *
   * \param locks_to_add number of locks to add
   * \return current pointer tag
   */
  inline int AddLocks(int locks_to_add)
  {
    return reference_and_reuse_counter.fetch_add(locks_to_add << 16) & cTAG_MASK;
  }

  /*!
   * Adds Locks
   *
   * \param locks_to_add number of locks to add
   * \param check_tag Check that reference counter tag is still this specified value
   */
  inline void AddLocks(int locks_to_add, int check_tag)
  {
    __attribute__((unused)) int old_value = reference_and_reuse_counter.fetch_add(locks_to_add << 16);
    assert((old_value & cTAG_MASK) == check_tag && "corrupted tag detected");
  }

  /*!
   * \return Pointer tag to use with curent reference counter
   */
  inline int GetPointerTag() const
  {
    return reference_and_reuse_counter.load() & cTAG_MASK;
  }

  /*!
   * Initializes reference counter for next use (set tag etc.)
   *
   * \param initial_number_of_locks Set reference counter to this value initially
   * \return Pointer tag to use for this buffer publishing operation
   */
  inline int InitReferenceCounter(int initial_number_of_locks)
  {
    int new_use_count = (reference_and_reuse_counter.load() + 1) & cREUSE_COUNTER_MASK;
    reference_and_reuse_counter.store((initial_number_of_locks << 16) | new_use_count);
    return new_use_count & cTAG_MASK;
  }

  /*!
   * Releases locks
   *
   * \param locks_to_release number of locks to release
   * \return Previous reference_and_reuse_counter value (only meant for class-internal use)
   */
  template <typename TDeleterType, typename TThis = tReferenceCountingBufferManager>
  inline int ReleaseLocks(int locks_to_release)
  {
    int old_value = reference_and_reuse_counter.fetch_sub(locks_to_release << 16);
    int old_counter = old_value >> 16;
    assert(old_counter - locks_to_release >= 0 && "negative reference counter detected");
    if (old_counter - locks_to_release == 0)
    {
      TDeleterType deleter;
      deleter(static_cast<TThis*>(this));
    }
    return old_value;
  }

  /*!
   * Releases locks
   *
   * \param locks_to_release number of locks to release
   * \param check_tag Check that reference counter tag is still this specified value
   */
  template <typename TDeleterType, typename TThis = tReferenceCountingBufferManager>
  inline void ReleaseLocks(int locks_to_release, int check_tag)
  {
    __attribute__((unused)) int old_value = ReleaseLocks<TDeleterType, TThis>(locks_to_release);
    assert((old_value & cTAG_MASK) == check_tag && "corrupted tag detected");
  }

  /*!
   * Try to lock port buffer manager
   * Will only succeed if number of locks is greater than zero
   * and pointer tag matches
   *
   * \return Returns whether locking succeeded
   */
  inline bool TryLock(int locks_to_add, int pointer_tag)
  {
    int current_value = reference_and_reuse_counter.load();
    while (((current_value >> 16) > 0) && (current_value & cTAG_MASK) == pointer_tag)
    {
      int new_value = current_value + (locks_to_add << 16);
      if (reference_and_reuse_counter.compare_exchange_strong(current_value, new_value))
      {
        return true;
      }
    }
    return false;
  }

//----------------------------------------------------------------------
// Protected fields and methods
//----------------------------------------------------------------------
protected:

  /*! Upper 16 bit: reference counter - lower 16 bit: reuse counter */
  std::atomic<int> reference_and_reuse_counter;


  tReferenceCountingBufferManager() : reference_and_reuse_counter(0) {}

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

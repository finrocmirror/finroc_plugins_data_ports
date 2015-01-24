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
/*!\file    plugins/data_ports/optimized/tThreadLocalBufferPools.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-04
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/tThreadLocalBufferPools.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/log_messages.h"

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

namespace internal
{

struct tDeletionList
{
  /*! Mutex to synchronize access on list */
  rrlib::thread::tMutex mutex;

  /*! List with pools that have not been completely deleted yet */
  std::list<tThreadLocalBufferPools*> garbage_pools;

  ~tDeletionList()
  {
    DeleteGarbage();
    if (garbage_pools.size())
    {
      RRLIB_LOG_PRINT(WARNING, garbage_pools.size(), " buffer pools have not been completely deleted.");
    }
  }

  void DeleteGarbage()
  {
    rrlib::thread::tLock lock(mutex);
    for (auto it = garbage_pools.begin(); it != garbage_pools.end();)
    {
      if ((*it)->DeleteAllGarbage(false))
      {
        delete *it;
        it = garbage_pools.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
};

typedef rrlib::design_patterns::tSingletonHolder<tDeletionList, rrlib::design_patterns::singleton::Longevity> tDeletionListInstance;
static inline unsigned int GetLongevity(tDeletionList*)
{
  return 0xFF000000; // should exist longer than any reusable (and longer than any thread objects)
}

static void DeleteGarbage()
{
  tDeletionList& list = tDeletionListInstance::Instance();
  list.DeleteGarbage();
}

struct tInitRegularDeleteTask
{
  tInitRegularDeleteTask()
  {
    core::internal::tGarbageDeleter::AddRegularTask(DeleteGarbage);
  }
};

} // namespace internal

__thread tThreadLocalBufferPools* tThreadLocalBufferPools::thread_local_instance = NULL;

tThreadLocalBufferPools::tThreadLocalBufferPools() :
  returned_buffer_queue()
{
  if (thread_local_instance)
  {
    FINROC_LOG_PRINT(ERROR, "Thread local buffers already instantiated for this thread");
    abort();
  }
  thread_local_instance = this;
  AddMissingPools();
}

tThreadLocalBufferPools::~tThreadLocalBufferPools()
{
  thread_local_instance = NULL;
}

bool tThreadLocalBufferPools::DeleteAllGarbage(bool initial_call)
{
  int missing = 0;
  if (ProcessReturnedBuffers() || initial_call)
  {
    for (auto it = pools.begin(); it != pools.end(); ++it)
    {
      missing += it->InternalBufferManagement().DeleteGarbage();
    }
  }
  else
  {
    return false;
  }
  return missing == 0;
}

bool tThreadLocalBufferPools::ProcessReturnedBuffers()
{
  rrlib::concurrent_containers::tQueueFragment<tBufferPointer> returned_buffers = returned_buffer_queue.DequeueAll();
  bool processed_buffers = false;
  while (!returned_buffers.Empty())
  {
    tThreadLocalBufferManager* buffer_pointer = returned_buffers.PopAny().release();
    buffer_pointer->ProcessLockReleasesFromOtherThreads<tBufferPointer::deleter_type>();
    processed_buffers = true;
  }
  return processed_buffers;
}


void tThreadLocalBufferPools::SafeDelete()
{
  if (!DeleteAllGarbage(true))
  {
    static internal::tInitRegularDeleteTask init_delete_task;
    internal::tDeletionList& list = internal::tDeletionListInstance::Instance();
    rrlib::thread::tLock lock(list.mutex);
    list.garbage_pools.push_back(this);
  }
  else
  {
    delete this;
  }
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

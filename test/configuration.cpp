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
/*!\file    plugins/data_ports/test/configuration.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-26
 *
 * Outputs diverse information on current finroc configuration
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <bitset>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tPort.h"

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
//----------------------------------------------------------------------
using namespace finroc::core;
using namespace finroc::data_ports;

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------
int main(int, char**)
{
#ifdef RRLIB_THREAD_ENFORCE_LOCK_ORDER
  FINROC_LOG_PRINT(USER, " RRLIB_THREAD_ENFORCE_LOCK_ORDER: true\n");
#else
  FINROC_LOG_PRINT(USER, " RRLIB_THREAD_ENFORCE_LOCK_ORDER: false\n");
#endif

#ifdef RRLIB_SINGLE_THREADED
  FINROC_LOG_PRINT(USER, " RRLIB_SINGLE_THREADED: true\n");
#else
  FINROC_LOG_PRINT(USER, " RRLIB_SINGLE_THREADED: false\n");
#endif

  FINROC_LOG_PRINT(USER, " sizeof(tPortBufferManager): ", sizeof(finroc::data_ports::standard::tPortBufferManager));
  FINROC_LOG_PRINT(USER, " sizeof(tThreadLocalBufferManager): ", sizeof(finroc::data_ports::optimized::tThreadLocalBufferManager));
  FINROC_LOG_PRINT(USER, " sizeof(tNumber): ", sizeof(finroc::data_ports::numeric::tNumber));
  typedef rrlib::concurrent_containers::tSet<void*, rrlib::concurrent_containers::tAllowDuplicates::NO, rrlib::thread::tNoMutex, rrlib::concurrent_containers::set::storage::ArrayChunkBased<2, 6>> tNoMutexSet;
  typedef rrlib::concurrent_containers::tSet<void*, rrlib::concurrent_containers::tAllowDuplicates::NO, rrlib::thread::tOrderedMutex, rrlib::concurrent_containers::set::storage::ArrayChunkBased<2, 6>> tOrderedMutexSet;
  FINROC_LOG_PRINT(USER, " sizeof(rrlib::concurrent_containers::tSet without mutex): ", sizeof(tNoMutexSet));
  FINROC_LOG_PRINT(USER, " sizeof(rrlib::concurrent_containers::tSet with ordered mutex): ", sizeof(tOrderedMutexSet));
  FINROC_LOG_PRINT(USER, " sizeof(bitset<1>): ", sizeof(std::bitset<1>));
  FINROC_LOG_PRINT(USER, " sizeof(tMutex): ", sizeof(rrlib::thread::tMutex));
  FINROC_LOG_PRINT(USER, " sizeof(tOrderedMutex): ", sizeof(rrlib::thread::tOrderedMutex));
  FINROC_LOG_PRINT(USER, " sizeof(tFrameworkElement): ", sizeof(finroc::core::tFrameworkElement));
  FINROC_LOG_PRINT(USER, " sizeof(tAbstractPort): ", sizeof(finroc::core::tAbstractPort));
  FINROC_LOG_PRINT(USER, " sizeof(tStandardPort): ", sizeof(finroc::data_ports::standard::tStandardPort));
  FINROC_LOG_PRINT(USER, " sizeof(tCheapCopyPort): ", sizeof(finroc::data_ports::optimized::tCheapCopyPort));

  std::atomic<void*> atomic_pointer;
  std::atomic<int64_t> atomic_int64;
  //FINROC_LOG_PRINT(USER, " std::atomic<void*>::is_lock_free(): ", atomic_pointer.is_lock_free()); // linker error on gcc 4.7
  //FINROC_LOG_PRINT(USER, " std::atomic<void*>::is_lock_free(): ", atomic_int64.is_lock_free());
  return 0;
}

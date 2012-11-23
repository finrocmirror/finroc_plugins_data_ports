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
/*!\file    plugins/data_ports/optimized/tBufferManagement.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-03
 *
 * \brief   Contains tBufferManagement
 *
 * \b tBufferManagement
 *
 * Implements different modes of buffer management.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__optimized__tBufferManagement_h__
#define __plugins__data_ports__optimized__tBufferManagement_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
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

/*!
 * Different modes of buffer management
 */
enum class tBufferManagementMode
{
  /*! Slowest - used if no other management is initialized */
  DEFAULT,

  /*!
   * Initializes thread-local publishing memory. Requires
   * sizeof(void*) * core::internal::tFrameworkElementRegister<tAbstractPort*>::cMAX_ELEMENTS
   * memory per thread.
   */
  PUBLISHING_MEMORY,

  /*!
   * Initializes thread-local publishing memory and thread-local buffer pools for maximum performance.
   * In addition to memory consumption from publishing memory
   * (sizeof(void*) * core::internal::tFrameworkElementRegister<tAbstractPort*>::cMAX_ELEMENTS),
   * produces overhead for buffer pool initialization and destruction.
   * Should only be used by long-lived, frequently publishing threads.
   */
  THREAD_LOCAL
};

/*!
 * Base class for all buffer management policies
 * Has virtual destructor so that C++ runtime type information can be obtained
 */
class tBufferManagementBase : boost::noncopyable
{


};


//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Buffer management implementations
/*!
 * Implements different modes of buffer management.
 *
 * Can exist once per thread and will greatly increase publishing performance
 * (and also memory usage:
 *  sizeof(void*) * core::internal::tFrameworkElementRegister<tAbstractPort*>::cMAX_ELEMENTS)
 * Should be allocated on the thread's stack so that it is always deleted when
 * the thread exits.
 * (Alternatively it can be attached to thread object to ensure this:
 *  tThread::LockObject(...)  )
 *
 * \tparam MODE Buffer management mode
 */
template <tBufferManagementMode MODE>
class tBufferManagement : boost::noncopyable
{
  typedef typename std::conditional < MODE == tBufferManagementMode::DEFAULT, tDefaultBufferManagement,
          typename std::conditional < MODE == tBufferManagementMode::PUBLISHING_MEMORY,
          tPublishMemorizingBufferManagement, tThreadLocalBufferManagement >::type >::type tImplementation;

  typedef typename tImplementation::tBufferPools tBufferPools;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Auto-recycling buffer pointer */
  typedef typename tBufferPools::tBufferPointer tBufferPointer;

  /*! Auto-recycling unique buffer pointer */
  typedef typename tBufferPools::tUniqueBufferPointer tUniqueBufferPointer;

  /*!
   * \param cheaply_copied_type_index 'Cheaply copied type index' of buffer to obtain
   * \return Unused buffer of specified type
   */
  static inline tBufferPointer GetUnusedBuffer(uint32_t cheaply_copied_type_index)
  {
    return implementation.GetUnusedBuffer(cheaply_copied_type_index);
  }

  /*!
   * \param cheaply_copied_type_index 'Cheaply copied type index' of unique buffer to obtain
   * \return Unused buffer of specified type
   */
  static inline tUniqueBufferPointer GetUnusedUniqueBuffer(uint32_t cheaply_copied_type_index)
  {
    return implementation.GetUnusedUniqueBuffer(cheaply_copied_type_index);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Buffer management implementation */
  tImplementation implementation;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

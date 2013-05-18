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
/*!\file    plugins/data_ports/common/tPortBufferPool.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-30
 *
 * \brief   Contains tPortBufferPool
 *
 * \b tPortBufferPool
 *
 * Pool of buffers used in ports - for a specific data type.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__common__tPortBufferPool_h__
#define __plugins__data_ports__common__tPortBufferPool_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/buffer_pools/tBufferPool.h"
#include "rrlib/rtti/rtti.h"
#include "core/definitions.h"
#include "core/internal/tGarbageDeleter.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tAbstractPortBufferManager.h"
#include "plugins/data_ports/optimized/cheaply_copied_types.h"

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
//! Port buffer pool
/*!
 * Pool of buffers used in ports - for a specific data type.
 *
 * In order to be real-time-capable, enough buffers need to be allocated initially...
 * otherwise the application becomes real-time-capable later - after enough buffers
 * have been allocated.
 *
 * \tparam TBufferManager Buffer manager type (derived from tAbstractPortBufferManager)
 * \tparam CONCURRENCY specifies if threads can return (write) and retrieve (read) buffers from the pool concurrently.
 */
template <typename TBufferManager, rrlib::concurrent_containers::tConcurrency CONCURRENCY>
class tPortBufferPool : private rrlib::util::tNoncopyable
{

  /*!
   * As buffers (at least their management information (reference counter etc.))
   * could be accessed by a thread while it is returned to possibly deleted pool -
   * deleting of these buffers should happen when its safe.
   * Therefore, this deleter passes them to tGarbageDeleter.
   */
  struct tPortBufferDeleter
  {
    void operator()(TBufferManager* p) const
    {
      core::internal::tGarbageDeleter::DeleteDeferred<TBufferManager>(p);
    }
  };

  typedef rrlib::buffer_pools::tBufferPool < TBufferManager, CONCURRENCY, rrlib::buffer_pools::management::QueueBased,
          rrlib::buffer_pools::deleting::ComplainOnMissingBuffers, rrlib::buffer_pools::recycling::UseOwnerStorageInBuffer,
          tPortBufferDeleter > tBufferPoolSingleThreaded;

  typedef rrlib::buffer_pools::tBufferPool < TBufferManager, CONCURRENCY, rrlib::buffer_pools::management::QueueBased,
          rrlib::buffer_pools::deleting::CollectGarbage, rrlib::buffer_pools::recycling::UseOwnerStorageInBuffer,
          tPortBufferDeleter > tBufferPoolConcurrent;

  /*! Type of wrapped buffer pool */
  typedef typename std::conditional < CONCURRENCY == rrlib::concurrent_containers::tConcurrency::NONE,
          tBufferPoolSingleThreaded, tBufferPoolConcurrent >::type tBufferPool;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! std::unique_ptr returned by this class that will automatically recycle buffer when out of scope */
  typedef typename tBufferPool::tPointer tPointer;

  /*!
   * \param data_type Type of buffers in pool
   * \param intial_size Number of buffer to allocate initially
   */
  tPortBufferPool(const rrlib::rtti::tType& data_type, int initial_size) :
    buffer_pool()
  {
    AllocateAdditionalBuffers(data_type, initial_size);
  }

  tPortBufferPool() : buffer_pool()
  {}

  /*!
   * Allocates the specified number of additional buffers and adds them to pool
   *
   * \param data_type Data type of buffers to add
   * \param count Number of buffers to allocate and add
   */
  inline void AllocateAdditionalBuffers(const rrlib::rtti::tType& data_type, size_t count)
  {
    for (size_t i = 0; i < count; i++)
    {
      CreateBuffer(data_type);
    }
  }

//  /*!
//   * \return Data Type of buffers in pool
//   */
//  inline rrlib::rtti::tType GetDataType() const
//  {
//    return data_type;
//  }

  /*!
   * \param cheaply_copyable_type_index Index of 'cheaply copied' data type of pool
   * \return Returns unused buffer. If there are no buffers that can be reused, a new buffer is allocated.
   */
  inline tPointer GetUnusedBuffer(uint32_t cheaply_copyable_type_index)
  {
    tPointer buffer = buffer_pool.GetUnusedBuffer();
    if (buffer)
    {
      return std::move(buffer);
    }
    return CreateBuffer(optimized::GetType(cheaply_copyable_type_index));
  }

  /*!
   * \param data_type Data type of buffers in this pool
   * \param possibly_create_buffer Create new buffer if there is none in pool at the moment?
   * \return Returns unused buffer. If there are no buffers that can be reused, a new buffer is possibly allocated.
   */
  inline tPointer GetUnusedBuffer(const rrlib::rtti::tType& data_type, bool possibly_create_buffer = true)
  {
    tPointer buffer = buffer_pool.GetUnusedBuffer();
    if (buffer)
    {
      return std::move(buffer);
    }
    return possibly_create_buffer ? CreateBuffer(data_type) : tPointer();
  }

  /*!
   * \return Returns internal buffer management backend for special manual tweaking of buffer pool.
   */
  typename tBufferPool::tBufferManagement& InternalBufferManagement()
  {
    return buffer_pool.InternalBufferManagement();
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Data Type of buffers in pool */
  //const rrlib::rtti::tType data_type;  This information would be redundant

  /*! Wrapped buffer pool */
  tBufferPool buffer_pool;


  /*
   * \param data_type Data type of buffers in this pool
   * \return Create new buffer/instance of port data and add to pool
   */
  tPointer CreateBuffer(const rrlib::rtti::tType& data_type)
  {
    std::unique_ptr<TBufferManager> new_buffer(TBufferManager::CreateInstance(data_type));

    // In case we have a string: allocate a certain buffer size (for RT capabilities with smaller payload) -
    // We do not need this check for 'cheaply copied' types - therefore the concurrency condition
    if (CONCURRENCY != rrlib::concurrent_containers::tConcurrency::NONE && new_buffer->GetObject().GetType().GetRttiName() == typeid(tString).name())
    {
      static_cast<rrlib::rtti::tGenericObject&>(new_buffer->GetObject()).GetData<tString>().reserve(512);  // TODO: move to parameter in some config.h
    }
    return buffer_pool.AddBuffer(std::move(new_buffer));
  }

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#endif

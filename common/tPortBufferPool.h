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

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
namespace standard
{
class tPortBufferManager;
}

namespace common
{

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

  /*! Whether this is a buffer pool for a standard port */
  enum { cSTANDARD_PORT = std::is_same<TBufferManager, standard::tPortBufferManager>::value };

  /*! Information what kind of content buffers contain (either data type - or buffer size) */
  typedef typename std::conditional<cSTANDARD_PORT, rrlib::rtti::tType, uint32_t>::type tContentId;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! std::unique_ptr returned by this class that will automatically recycle buffer when out of scope */
  typedef typename tBufferPool::tPointer tPointer;

  /*!
   * \param buffer_content Buffer content
   * \param intial_size Number of buffers to allocate initially
   */
  tPortBufferPool(const tContentId& buffer_content, int initial_size) :
    buffer_pool()
  {
    AllocateAdditionalBuffers(buffer_content, initial_size);
  }

  tPortBufferPool() : buffer_pool()
  {}

  /*!
   * Allocates the specified number of additional buffers and adds them to pool
   *
   * \param buffer_content Buffer content
   * \param count Number of buffers to allocate and add
   */
  inline void AllocateAdditionalBuffers(const tContentId& buffer_content, size_t count)
  {
    for (size_t i = 0; i < count; i++)
    {
      CreateBuffer(buffer_content);
    }
  }

  /*!
   * \param data_type Data type of desired buffer
   * \param possibly_create_buffer Create new buffer if there is none in pool at the moment?
   * \return Returns unused buffer. If there are no buffers that can be reused, a new buffer is allocated.
   */
  template <bool Tstandard_port = cSTANDARD_PORT>
  inline tPointer GetUnusedBuffer(const typename std::enable_if<Tstandard_port, rrlib::rtti::tType>::type& data_type, bool possibly_create_buffer = true)
  {
    tPointer buffer = buffer_pool.GetUnusedBuffer();
    if (buffer)
    {
      return std::move(buffer);
    }
    return possibly_create_buffer ? CreateBuffer(data_type) : tPointer();
  }

  /*!
   * \param buffer_size Size of buffer
   * \param data_type Data type of desired buffer
   * \return Returns unused buffer. If there are no buffers that can be reused, a new buffer is allocated.
   */
  template <bool Tstandard_port = cSTANDARD_PORT>
  inline tPointer GetUnusedBuffer(typename std::enable_if < !Tstandard_port, uint32_t >::type buffer_size, const rrlib::rtti::tType& data_type)
  {
    tPointer buffer = buffer_pool.GetUnusedBuffer();
    if (buffer)
    {
      return std::move(buffer);
    }
    return CreateBuffer(buffer_size);
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

  /*! Wrapped buffer pool */
  tBufferPool buffer_pool;


  /*
   * \param buffer_content Buffer content
   * \return Create new buffer/instance of port data and add to pool
   */
  tPointer CreateBuffer(const tContentId& buffer_content)
  {
    std::unique_ptr<TBufferManager> new_buffer(TBufferManager::CreateInstance(buffer_content));

    // In case we have a string: allocate a certain buffer size (for RT capabilities with smaller payload) -
    // We do not need this check for 'cheaply copied' types - therefore the concurrency condition
    if (cSTANDARD_PORT && new_buffer->GetObject().GetType().GetRttiName() == typeid(tString).name())
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

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
/*!\file    plugins/data_ports/common/tPortQueue.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-01-10
 *
 * \brief   Contains tPortQueue
 *
 * \b tPortQueue
 *
 * Queue for incoming port values - used in input ports.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__common__tPortQueue_h__
#define __plugins__data_ports__common__tPortQueue_h__

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
namespace common
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Input port queue
/*!
 * Queue for incoming port values - used in input ports.
 *
 * \tparam TLockingPointer Unique pointer to port buffer that unlocks buffer on release
 */
template <typename TLockingPointer>
class tPortQueue : boost::noncopyable
{

  /*! Extra container class, since port buffers may enqueued in multiple queues */
  class tPortBufferContainer :
    public rrlib::buffer_pools::tBufferManagementInfo,
    public rrlib::concurrent_containers::tQueueable<rrlib::concurrent_containers::tQueueability::FULL_OPTIMIZED>
  {
  public:
    /*! locked buffer */
    TLockingPointer locked_buffer;
  };

  /*! Buffer container pool */
  typedef rrlib::buffer_pools::tBufferPool < tPortBufferContainer, rrlib::concurrent_containers::tConcurrency::FULL,
          rrlib::buffer_pools::management::QueueBased, rrlib::buffer_pools::deleting::ComplainOnMissingBuffers,
          rrlib::buffer_pools::recycling::UseOwnerStorageInBuffer > tPortBufferContainerPool;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Buffer container pointer */
  typedef typename tPortBufferContainerPool::tPointer tPortBufferContainerPointer;

  tPortQueue(bool fifo_queue) :
    port_buffer_container_pool(),
    fifo_queue(fifo_queue)
  {
    if (fifo_queue)
    {
      queue_fifo = new tFifoPortQueue();
    }
    else
    {
      queue_all = new tDequeueAllPortQueue();
    }
  }

  ~tPortQueue()
  {
    if (fifo_queue)
    {
      delete queue_fifo;
    }
    else
    {
      delete queue_all;
    }
  }

  /*!
   * Dequeue locked buffer from queue
   */
  TLockingPointer Dequeue()
  {
    assert(fifo_queue);
    tPortBufferContainerPointer ptr = queue_fifo->Dequeue();
    return ptr ? std::move(ptr->locked_buffer) : TLockingPointer();
  }

  /*!
   * Dequeue all locked buffers from queue
   */
  rrlib::concurrent_containers::tQueueFragment<tPortBufferContainerPointer> DequeueAll()
  {
    assert(!fifo_queue);
    return queue_all->DequeueAll();
  }

  /*!
   * Enqueue locked buffer in queue
   */
  void Enqueue(TLockingPointer && pointer)
  {
    tPortBufferContainerPointer container_pointer = port_buffer_container_pool.GetUnusedBuffer();
    if (!container_pointer)
    {
      container_pointer = port_buffer_container_pool.AddBuffer(std::unique_ptr<tPortBufferContainer>(new tPortBufferContainer()));
    }
    container_pointer->locked_buffer = std::move(pointer);
    if (fifo_queue)
    {
      queue_fifo->Enqueue(container_pointer);
    }
    else
    {
      queue_all->Enqueue(container_pointer);
    }
  }

  int GetMaxQueueLength()
  {
    return fifo_queue ? queue_fifo->GetMaxLength() : queue_all->GetMaxLength();
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  typedef rrlib::concurrent_containers::tQueue < tPortBufferContainerPointer, rrlib::concurrent_containers::tConcurrency::FULL,
          rrlib::concurrent_containers::tDequeueMode::FIFO, true > tFifoPortQueue;

  typedef rrlib::concurrent_containers::tQueue < tPortBufferContainerPointer, rrlib::concurrent_containers::tConcurrency::FULL,
          rrlib::concurrent_containers::tDequeueMode::ALL, true > tDequeueAllPortQueue;

  /*! Buffer container pool instance */
  tPortBufferContainerPool port_buffer_container_pool;

  /*! Do we use a FIFO queue? */
  const bool fifo_queue;

  union
  {
    /*! FIFO Queue for ports with incoming value queue */
    tFifoPortQueue* queue_fifo;

    /*! Queue for ports with incoming value queue that dequeues all values at once */
    tDequeueAllPortQueue* queue_all;
  };

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

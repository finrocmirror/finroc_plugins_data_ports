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
/*!\file    plugins/data_ports/standard_ports/tStandardPort.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-23
 *
 * \brief   Contains tStandardPort
 *
 * \b tStandardPort
 *
 * This is the untyped standard implementation for data ports.
 * It can be used with any kind of C++ data type.
 *
 * Convention: Protected Methods do not perform any necessary synchronization
 * with respect to concurrency.
 * Methods in the public interface need to make sure they perform the necessary means.
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__standard__tStandardPort_h__
#define __plugins__data_ports__standard__tStandardPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tAbstractDataPort.h"
#include "plugins/data_ports/common/tPortBufferPool.h"
#include "plugins/data_ports/common/tPortListenerRaw.h"
#include "plugins/data_ports/common/tPublishOperation.h"
#include "plugins/data_ports/standard/tPortBufferManager.h"
#include "plugins/data_ports/standard/tPullRequestHandlerRaw.h"

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
template<typename T>
class tPort;

namespace common
{
template <typename TPort, typename TPublishingData, typename TManager>
class tPullOperation;
}

namespace standard
{
class tMultiTypePortBufferPool;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Standard data port implementation
/*!
 * This is the untyped standard implementation for data ports.
 * It can be used with any kind of C++ data type.
 *
 * Convention: Protected Methods do not perform any necessary synchronization
 * with respect to concurrency.
 * Methods in the public interface need to make sure they perform the necessary means.
 */
class tStandardPort : public common::tAbstractDataPort
{
public:

  /*! Buffer pool used by this port implementation */
  typedef common::tPortBufferPool<tPortBufferManager, rrlib::concurrent_containers::tConcurrency::FULL> tBufferPool;

private:

  typedef rrlib::util::tTaggedPointer<tPortBufferManager, true, 3> tTaggedBufferPointer;

  struct tPortBufferUnlocker
  {
    void operator()(tPortBufferManager* p) const
    {
      p->ReleaseLocks<typename tBufferPool::tPointer::deleter_type, tPortBufferManager>(1);
    }
  };

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! std::unique_ptr that automatically releases lock when deleted */
  typedef std::unique_ptr<tPortBufferManager, tPortBufferUnlocker> tLockingManagerPointer;

  /*! std::unique_ptr that automatically recycles buffer when deleted */
  typedef typename tBufferPool::tPointer tUnusedManagerPointer;

  /*!
   * \param creation_info PortCreationInformation
   */
  tStandardPort(common::tAbstractDataPortCreationInfo creation_info);

  virtual ~tStandardPort();

  /*!
   * \param listener Listener to add
   */
  inline void AddPortListenerRaw(common::tPortListenerRaw& listener)
  {
    tLock l(*this);
    port_listeners.Add(&listener);
  }

  /*!
   * Set current value to default value
   */
  void ApplyDefaultValue();

  /*!
   * Publish buffer through port
   * (not in normal operation, but from browser; difference: listeners on this port will be notified)
   *
   * \param buffer Buffer with data (must be owned by current thread)
   */
  void BrowserPublish(tUnusedManagerPointer& data);

  /*!
   * \return Does port contain default value?
   */
  inline bool ContainsDefaultValue()
  {
    tTaggedBufferPointer current_buffer = current_value.load();
    return current_buffer.GetPointer() == default_value.get();
  }

  /*!
   * Dequeue all elements currently in port's input queue
   * (use only with ports that have input queue with tDequeueMode::ALL)
   *
   * \return fragment Fragment containing all dequeued values
   */
  rrlib::concurrent_containers::tQueueFragment<tLockingManagerPointer> DequeueAllRaw()
  {
    assert(GetFlag(tFlag::HAS_QUEUE) && GetFlag(tFlag::HAS_DEQUEUE_ALL_QUEUE));
    return queue_all->DequeueAll();
  }

  /*!
   * Dequeue first/oldest element in queue.
   * Because queue is bounded, continuous dequeueing may skip some values.
   * Use dequeueAll if a continuous set of values is required.
   *
   * Container will be recycled automatically by unique pointer
   * (Use only with ports that have a FIFO input queue)
   *
   * \return Dequeued first/oldest element in queue
   */
  tLockingManagerPointer DequeueSingleRaw()
  {
    assert(GetFlag(tFlag::HAS_QUEUE) && (!GetFlag(tFlag::HAS_DEQUEUE_ALL_QUEUE)));
    return queue_fifo->Dequeue();
  }

  virtual void ForwardData(tAbstractPort& other);

  /*!
   * \param never_pull Do not attempt to pull data - even if port is on push strategy
   * \return Current locked port data buffer
   */
  inline tLockingManagerPointer GetCurrentValueRaw(bool never_pull = false)
  {
    if (PushStrategy() || never_pull)
    {
      return LockCurrentValueForRead();
    }
    else
    {
      return PullValueRaw();
    }
  }

  /*!
   * May only be called before port is initialized
   *
   * \return Buffer with default value. Can be used to change default value for port.
   */
  rrlib::rtti::tGenericObject& GetDefaultBufferRaw();

  /*!
   * Pulls port data (regardless of strategy)
   *
   * \param intermediate_assign Assign pulled value to ports in between?
   * \param ignore_pull_request_handler_on_this_port Ignore any pull request handler on this port?
   * \return Pulled data in locked buffer
   */
  inline tLockingManagerPointer GetPullRaw(bool intermediate_assign, bool ignore_pull_request_handler_on_this_port)
  {
    return PullValueRaw(intermediate_assign, ignore_pull_request_handler_on_this_port);
  }

  /*!
   * \return Unused buffer from send buffers for writing.
   * (Using this method, typically no new buffers/objects need to be allocated)
   */
  inline tUnusedManagerPointer GetUnusedBufferRaw()
  {
    tUnusedManagerPointer buffer = multi_type_buffer_pool ? GetUnusedBufferRaw(GetDataType()) : buffer_pool.GetUnusedBuffer(GetDataType());
    buffer->SetUnused(true);
    return buffer;
  }

  virtual tUnusedManagerPointer GetUnusedBufferRaw(const rrlib::rtti::tType& dt);

  virtual void NotifyDisconnect();

  /*!
   * Publish Data Buffer. This data will be forwarded to any connected ports.
   * It should not be modified thereafter.
   * Should only be called on output ports
   *
   * \param data Data buffer acquired from a port using getUnusedBuffer (or locked data received from another port)
   */
  inline void Publish(tUnusedManagerPointer& data)
  {
    PublishImplementation<false, tChangeStatus::CHANGED, false>(data);
  }
  inline void Publish(tLockingManagerPointer& data)
  {
    PublishImplementation<false, tChangeStatus::CHANGED, false>(data);
  }

  /*!
   * \param listener Listener to remove
   */
  inline void RemovePortListenerRaw(common::tPortListenerRaw& listener)
  {
    tLock l(*this);
    port_listeners.Remove(&listener);
  }

  /*!
   * \param pull_request_handler Object that handles pull requests - null if there is none (typical case)
   */
  void SetPullRequestHandler(tPullRequestHandlerRaw* pull_request_handler);

//  /*!
//   * Does port (still) have this value?
//   * (calling this is only safe, when pd is locked)
//   *
//   * \param pd Port value
//   * \return Answer
//   */
//  inline bool ValueIs(const void* pd) const
//  {
//    return value.Get()->GetData()->GetRawDataPtr() == pd;
//  }

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*!
   * Temporary set of variables with info on current publishing operation
   */
  struct tPublishingData : boost::noncopyable
  {
    enum { cCOPY_ON_RECEIVE = 0 };

    /*!
     * Number of locks already added to reference counter for current publishing operation
     * Must be equal or larger than locks required for this publising operation
     */
    int added_locks;

    /*! Number of locks that were required for assignments etc. */
    int used_locks;

    /*! Pointer tag to use */
    int pointer_tag;

    /*! Pointer to port data used in current publishing operation */
    tPortBufferManager* published_buffer;

    /*! Tagged pointer to port data used in current publishing operation */
    tTaggedBufferPointer published_buffer_tagged_pointer;

    /*!
     * Registers another lock on buffer.
     * Will increase lock_estimate and reference counter if necessary.
     */
    void AddLock()
    {
      used_locks++;
      assert(used_locks <= added_locks && "Too many locks in this publishing operation");
    }

    tPublishingData(tUnusedManagerPointer& published, int add_locks) :
      added_locks(add_locks),
      used_locks(0),
      pointer_tag(published->InitReferenceCounter(added_locks)),
      published_buffer(published.get()),
      published_buffer_tagged_pointer(published.release(), pointer_tag)
    {
      published_buffer->SetUnused(false);
    }

    tPublishingData(tLockingManagerPointer& published, int add_locks) :
      added_locks(add_locks),
      used_locks(0),
      pointer_tag(published->AddLocks(added_locks - 1)), // 1 was already added by/for "tLockingManagerPointer& published"
      published_buffer(published.get()),
      published_buffer_tagged_pointer(published.release(), pointer_tag)
    {
    }

    tPublishingData(int add_locks) :
      added_locks(add_locks),
      used_locks(0),
      pointer_tag(0),
      published_buffer(NULL),
      published_buffer_tagged_pointer()
    {
    }

    // Removes obsolete locks
    ~tPublishingData()
    {
      if (published_buffer)
      {
        assert((used_locks <= added_locks) && "Too many locks in this publishing operation");
        if (used_locks < added_locks)
        {
          published_buffer->ReleaseLocks<typename tUnusedManagerPointer::deleter_type, tPortBufferManager>(added_locks - used_locks);
        }
      }
    }

    void CheckRecycle() {}

    void Init(tPortBufferManager* published)
    {
      assert(!published_buffer);
      pointer_tag = published->GetPointerTag();
      published_buffer = published;
      published_buffer_tagged_pointer = tTaggedBufferPointer(published, pointer_tag);
    }
  };

  /*!
   * Custom special assignment to port.
   * Used, for instance, for queued ports.
   *
   * \param publishing_info Info on current publishing operation
   */
  virtual void NonStandardAssign(tPublishingData& publishing_info);

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  template <typename TPort, typename TPublishingData>
  friend class common::tPublishOperation;

  template <typename TPort, typename TPublishingData, typename TManager>
  friend class common::tPullOperation;

  typedef rrlib::concurrent_containers::tQueue < tLockingManagerPointer, rrlib::concurrent_containers::tConcurrency::FULL,
          rrlib::concurrent_containers::tDequeueMode::FIFO, true > tFifoPortQueue;

  typedef rrlib::concurrent_containers::tQueue < tLockingManagerPointer, rrlib::concurrent_containers::tConcurrency::FULL,
          rrlib::concurrent_containers::tDequeueMode::ALL, true > tDequeueAllPortQueue;

  /*! Current type of port data - relevant for ports with multi type buffer pool */
  //const rrlib::rtti::tType cur_data_type;

  /*! Pool with reusable buffers that are published from this port - by any thread */
  tBufferPool buffer_pool;

  /*! Pool with different types of reusable buffers that are published from this port - by any thread */
  tMultiTypePortBufferPool* multi_type_buffer_pool;

  /*! Default value - optional - NULL if not set*/
  tLockingManagerPointer default_value;

  /*!
   * Current port value (never NULL)
   *
   * Tag stores a few bits of buffer reuse counter to avoid ABA problem
   * when retrieving buffer
   */
  std::atomic<typename tTaggedBufferPointer::tStorage> current_value;

  /*!
   * Is data assigned to port in standard way? Otherwise - for instance, when using queues -
   * the virtual method
   *
   * Hopefully compiler will optimize this, since it's final/const
   */
  const bool standard_assign;

  union
  {
    /*! FIFO Queue for ports with incoming value queue */
    tFifoPortQueue* queue_fifo;

    /*! Queue for ports with incoming value queue that dequeues all values at once */
    tDequeueAllPortQueue* queue_all;
  };

  /*!
   * Optimization - if this is not null that means:
   * - this port is an output port and has one active receiver (stored in this variable)
   * - both ports are standard-assigned
   */
  //public PortBase std11CaseReceiver; // should not need to be volatile

  /*! Object that handles pull requests - null if there is none (typical case) */
  tPullRequestHandlerRaw* pull_request_handler;

  /*! Listeners of port value changes */
  rrlib::concurrent_containers::tSet < common::tPortListenerRaw*, rrlib::concurrent_containers::tAllowDuplicates::NO, rrlib::thread::tNoMutex,
        rrlib::concurrent_containers::set::storage::ArrayChunkBased<1, 3, definitions::cSINGLE_THREADED >> port_listeners;

  /*!
   * Assigns new data to port.
   * Releases and unlocks old data.
   * Lock on new data has to be set before
   *
   * \param publishing_data Info on current publishing operation
   * \return Whether Assigning succeeded (always true)
   */
  inline bool Assign(tPublishingData& publishing_data)
  {
    assert(publishing_data.published_buffer->GetObject().GetType() == GetDataType());

    publishing_data.AddLock();
    tTaggedBufferPointer old = current_value.exchange(publishing_data.published_buffer_tagged_pointer);
    old->ReleaseLocks<typename tUnusedManagerPointer::deleter_type, tPortBufferManager>(1, old.GetStamp());
    if (!standard_assign)
    {
      NonStandardAssign(publishing_data);
    }
    return true;
  }

  /*!
   * Calls pull request handler
   *
   * \param publishing_data Info on current pull operation
   */
  void CallPullRequestHandler(tPublishingData& publishing_data, bool intermediate_assign);

  /*!
   * \return Created default value (if default value was set in creation_info; buffer comes from buffer pool)
   */
  static tPortBufferManager* CreateDefaultValue(const common::tAbstractDataPortCreationInfo& creation_info, tBufferPool& buffer_pool);

  virtual int GetMaxQueueLengthImplementation() const;

  // quite similar to publish
  virtual void InitialPushTo(tAbstractPort& target, bool reverse);

  /*!
   * \param publishing_data Info on current publish/pull operation
   */
  void LockCurrentValueForPublishing(tPublishingData& publishing_data);

  /*!
   * Lock current buffer for safe read access.
   *
   * \param add_locks number of locks to add (one will be released by unique_pointer returned by this method)
   * \return Locked buffer
   */
  inline tLockingManagerPointer LockCurrentValueForRead(int add_locks = 1) const
  {
    assert(add_locks > 0);

    while (true)
    {
      tTaggedBufferPointer current_buffer = current_value.load();
      if (current_buffer->TryLock(add_locks, current_buffer.GetStamp()))
      {
        // successful
        return tLockingManagerPointer(current_buffer.GetPointer());
      }
    }
  }

  /*!
   * Notify any port listeners of data change
   *
   * \param publishing_data Info on current publishing operation
   */
  inline void NotifyListeners(tPublishingData& publishing_data)
  {
    for (auto it = port_listeners.Begin(); it != port_listeners.End(); ++it)
    {
      (*it)->PortChangedRaw(*this, *publishing_data.published_buffer, publishing_data.published_buffer->GetTimestamp());
    }
  }

  virtual void PrintStructure(int indent, std::stringstream& output); // FIXME: add override with gcc 4.7

  /*!
   * Publish data
   *
   * \param data Data to publish
   * \param reverse Value received in reverse direction?
   * \param changed_constant changedConstant to use
   */
  inline void Publish(tUnusedManagerPointer& data, bool reverse, tChangeStatus changed_constant)
  {
    if (!reverse)
    {
      if (changed_constant == tChangeStatus::CHANGED)
      {
        PublishImplementation<false, tChangeStatus::CHANGED, false>(data);
      }
      else
      {
        PublishImplementation<false, tChangeStatus::CHANGED_INITIAL, false>(data);
      }
    }
    else
    {
      if (changed_constant == tChangeStatus::CHANGED)
      {
        PublishImplementation<true, tChangeStatus::CHANGED, false>(data);
      }
      else
      {
        PublishImplementation<true, tChangeStatus::CHANGED_INITIAL, false>(data);
      }
    }
  }

  /*!
   * (only for use by port classes)
   *
   * Publish Data Buffer. This data will be forwarded to any connected ports.
   * It should not be modified thereafter.
   * Should only be called on output ports
   *
   * \param data Data buffer
   * \tparam REVERSE Publish in reverse direction? (typical is forward)
   * \tparam CHANGE_CONSTANT changedConstant to use
   * \tparam BROWSER_PUBLISH Inform this port's listeners on change and also publish in reverse direction? (only set from BrowserPublish())
   */
  template <bool REVERSE, tChangeStatus CHANGE_CONSTANT, bool BROWSER_PUBLISH, typename TDeleter>
  inline void PublishImplementation(std::unique_ptr<tPortBufferManager, TDeleter>& data)
  {
    if (!(IsReady() || BROWSER_PUBLISH))
    {
      FINROC_LOG_PRINT(WARNING, "Port is not ready. Ignoring publishing request.");
      return;
    }

    common::tPublishOperation<tStandardPort, tPublishingData> publish_operation(data, 1000);
    publish_operation.Execute<REVERSE, CHANGE_CONSTANT, BROWSER_PUBLISH>(*this);
  }

  /*!
   * Pull/read current value from source port
   * When multiple source ports are available an arbitrary one of them is used.
   *
   * \param intermediate_assign Assign pulled value to ports in between?
   * \param ignore_pull_request_handler_on_this_port Ignore pull request handler on first port? (for network port pulling it's good if pullRequestHandler is not called on first port)
   * \return Locked port data
   */
  tLockingManagerPointer PullValueRaw(bool intermediate_assign = true, bool ignore_pull_request_handler_on_this_port = false);

  //virtual void SetMaxQueueLengthImplementation(int length);

  /*!
   * Update statistics if this is enabled
   *
   * \param publishing_data Info on current publishing operation
   * \param source Source port
   * \param target Target Port
   */
  inline void UpdateStatistics(tPublishingData& publishing_data, tStandardPort& source, tStandardPort& target)
  {
    if (definitions::cCOLLECT_EDGE_STATISTICS)    // const, so method can be optimized away completely
    {
      UpdateEdgeStatistics(source, target, publishing_data.published_buffer->GetObject());
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

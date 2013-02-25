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
/*!\file    plugins/data_ports/optimized/tCheapCopyPort.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 * \brief   Contains tCheapCopyPort
 *
 * \b tCheapCopyPort
 *
 * Port implementation with improved performance characteristics for 'cheaply copied' data types.
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__optimized__tCheapCopyPort_h__
#define __plugins__data_ports__optimized__tCheapCopyPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tChangeContext.h"
#include "plugins/data_ports/common/tAbstractDataPort.h"
#include "plugins/data_ports/common/tPortBufferPool.h"
#include "plugins/data_ports/common/tPortQueue.h"
#include "plugins/data_ports/common/tPublishOperation.h"
#include "plugins/data_ports/optimized/tGlobalBufferPools.h"
#include "plugins/data_ports/optimized/tPullRequestHandlerRaw.h"
#include "plugins/data_ports/optimized/tThreadLocalBufferPools.h"

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

namespace optimized
{
//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Optimized port implementation for 'cheaply copied' types
/*!
 * Port implementation with improved performance characteristics for 'cheaply copied' data types.
 *
 * Convention: Protected Methods do not perform any necessary synchronization
 * with respect to concurrency.
 * Methods in the public interface need to make sure they perform the necessary means.
 */
class tCheapCopyPort : public common::tAbstractDataPort
{
  template<typename T>
  friend class finroc::data_ports::tPort;

  typedef rrlib::util::tTaggedPointer<tCheaplyCopiedBufferManager, true, 3> tTaggedBufferPointer;

  struct tPortBufferUnlocker
  {
    void operator()(tCheaplyCopiedBufferManager* p) const
    {
      tThreadLocalBufferPools* origin = p->GetThreadLocalOrigin();
      if (origin)
      {
        if (origin == tThreadLocalBufferPools::Get()) // Is current thread the owner?
        {
          static_cast<tThreadLocalBufferManager*>(p)->ReleaseThreadLocalLocks<tThreadSpecificBufferPools<false>::tBufferPointer::deleter_type>(1);
        }
        else
        {
          static_cast<tThreadLocalBufferManager*>(p)->ReleaseLocksFromOtherThread(1);
        }
      }
      else
      {
        p->ReleaseLocks<typename tGlobalBufferPools::tBufferPointer::deleter_type, tCheaplyCopiedBufferManager>(1);
      }
    }

    void operator()(tThreadLocalBufferManager* p) const
    {
      assert(tThreadLocalBufferPools::Get());
      if (p->GetThreadLocalOrigin() == tThreadLocalBufferPools::Get()) // Is current thread the owner?
      {
        p->ReleaseThreadLocalLocks<typename tThreadLocalBufferPools::tBufferPointer::deleter_type>(1);
      }
      else
      {
        p->ReleaseLocksFromOtherThread(1);
      }
    }
  };

  struct tUnusedBufferRecycler
  {
    void operator()(tCheaplyCopiedBufferManager* p) const
    {
      if (p->GetThreadLocalOrigin())
      {
        operator()(static_cast<tThreadLocalBufferManager*>(p));
        return;
      }
      typename tGlobalBufferPools::tBufferPointer::deleter_type deleter;
      deleter(p);
    }

    void operator()(tThreadLocalBufferManager* p) const
    {
      typename tThreadLocalBufferPools::tBufferPointer::deleter_type deleter;
      deleter(p);
    }
  };

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! std::unique_ptr that automatically releases lock when deleted */
  typedef std::unique_ptr<tCheaplyCopiedBufferManager, tPortBufferUnlocker> tLockingManagerPointer;

  /*! std::unique_ptr that automatically recycles buffer when deleted */
  typedef std::unique_ptr<tCheaplyCopiedBufferManager, tUnusedBufferRecycler> tUnusedManagerPointer;

  /*! std::unique_ptr Pointer in a queue fragment */
  typedef typename common::tPortQueue<tLockingManagerPointer>::tPortBufferContainerPointer tPortBufferContainerPointer;


  /*!
   * \param creation_info PortCreationInformation
   */
  tCheapCopyPort(common::tAbstractDataPortCreationInfo creation_info);

  virtual ~tCheapCopyPort();

  /*!
   * Set current value to default value
   */
  void ApplyDefaultValue();

  /*!
   * Publish buffer through port
   * (not in normal operation, but from browser; difference: listeners on this port will be notified)
   *
   * \param buffer Buffer with data (must be owned by current thread)
   * \param notify_listener_on_this_port Notify listener on this port?
   * \param change_constant Change constant to use for publishing operation
   * \return Error message if something did not work
   */
  virtual std::string BrowserPublishRaw(tUnusedManagerPointer& buffer, bool notify_listener_on_this_port = true,
                                        common::tAbstractDataPort::tChangeStatus change_constant = common::tAbstractDataPort::tChangeStatus::CHANGED);

  /*!
   * \return Does port contain default value?
   */
  //bool ContainsDefaultValue();

  /*!
   * Copy current value to buffer (Most efficient get()-version)
   *
   * \param buffer Buffer to copy current data to
   * \param timestamp Buffer to copy attached time stamp to
   * \param dont_pull Do not attempt to pull data - even if port is on push strategy
   */
  void CopyCurrentValueToGenericObject(rrlib::rtti::tGenericObject& buffer, rrlib::time::tTimestamp& timestamp, bool dont_pull = false);

  /*!
   * Copy current value to buffer managed by manager (including time stamp)
   *
   * \param buffer Buffer to copy current data
   * \param dont_pull Do not attempt to pull data - even if port is on push strategy
   */
  template <typename TManager>
  inline void CopyCurrentValueToManager(TManager& buffer, bool dont_pull = false)
  {
    rrlib::time::tTimestamp timestamp;
    CopyCurrentValueToGenericObject(buffer.GetObject(), timestamp, dont_pull);
    buffer.SetTimestamp(timestamp);
  }

  /*!
   * Copy current value to buffer (Most efficient get()-version)
   *
   * \param buffer Buffer to copy current data to
   * \param never_pull Do not attempt to pull data - even if port is on push strategy
   */
  template <typename T>
  inline void CopyCurrentValue(T& buffer, bool never_pull = false)
  {
    if (never_pull || PushStrategy())
    {
      for (; ;)
      {
        tTaggedBufferPointer current = current_value.load();
        buffer = current->GetObject().GetData<T>();
        tTaggedBufferPointer::tStorage current_raw = current;
        if (current_raw == current_value.load())    // still valid??
        {
          return;
        }
      }
    }
    else
    {
      tLockingManagerPointer dc = PullValueRaw();
      rrlib::rtti::sStaticTypeInfo<T>::DeepCopy(dc->GetObject().GetData<T>(), buffer, NULL);
    }
  }

  /*!
   * Copy current value to buffer (Most efficient get()-version)
   *
   * \param buffer Buffer to copy current data to
   * \param timestamp Buffer to copy current timestamp to
   * \param never_pull Do not attempt to pull data - even if port is on push strategy
   */
  template <typename T>
  inline void CopyCurrentValue(T& buffer, rrlib::time::tTimestamp& timestamp, bool never_pull = false)
  {
    if (never_pull || PushStrategy())
    {
      for (; ;)
      {
        tTaggedBufferPointer current = current_value.load();
        buffer = current->GetObject().GetData<T>();
        timestamp = current.GetPointer()->GetTimestamp();
        tTaggedBufferPointer::tStorage current_raw = current;
        if (current_raw == current_value.load())    // still valid??
        {
          return;
        }
      }
    }
    else
    {
      tLockingManagerPointer dc = PullValueRaw();
      rrlib::rtti::sStaticTypeInfo<T>::DeepCopy(dc->GetObject().GetData<T>(), buffer, NULL);
      timestamp = dc->GetTimestamp();
    }
  }

  /*!
   * Dequeue all elements currently in port's input queue
   * (use only with ports that have input queue with tDequeueMode::ALL)
   *
   * \return fragment Fragment containing all dequeued values
   */
  rrlib::concurrent_containers::tQueueFragment<tPortBufferContainerPointer> DequeueAllRaw()
  {
    assert(GetFlag(tFlag::HAS_QUEUE) && GetFlag(tFlag::HAS_DEQUEUE_ALL_QUEUE));
    return input_queue->DequeueAll();
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
    return input_queue->Dequeue();
  }

  virtual void ForwardData(tAbstractPort& other);

  /*!
   * \return Returns data type's 'cheaply copyable type index'
   */
  inline uint32_t GetCheaplyCopyableTypeIndex() const
  {
    return cheaply_copyable_type_index;
  }

  /*!
   * \return Default value that has been assigned to port (NULL if no default value set)
   */
  const rrlib::rtti::tGenericObject* GetDefaultValue() const
  {
    return default_value.get();
  }

  /*!
   * \param never_pull Do not attempt to pull data - even if port is on push strategy
   * \return Current locked port data buffer
   */
//  inline tLockingManagerPointer GetCurrentValueRaw(bool never_pull = false)
//  {
//    if (PushStrategy() || never_pull)
//    {
//      return LockCurrentValueForRead();
//    }
//    else
//    {
//      return PullValueRaw();
//    }
//  }

  //TODO: GetCurrentValueUniqueRaw()

//  /*!
//   * \return Buffer with default value. Can be used to change default value
//   * for port. However, this should be done before the port is used.
//   */
//  inline rrlib::rtti::tGenericObject* GetDefaultBufferRaw()
//  {
//    assert(((!IsReady())) && "please set default value _before_ initializing port");
//    return default_value->GetObject();
//  }

//  /*!
//   * \param dont_pull Do not attempt to pull data - even if port is on push strategy
//   * \return Current data in CC Interthread-container. Needs to be recycled manually.
//   */
//  tCCPortDataManager* GetInInterThreadContainer(bool dont_pull = false);

//  /*!
//   * Pulls port data (regardless of strategy) and returns it in interhread container
//   * (careful: no auto-release of lock)
//   * \param intermediate_assign Assign pulled value to ports in between?
//   * \param ignore_pull_request_handler_on_this_port Ignore pull request handler on first port? (for network port pulling it's good if pullRequestHandler is not called on first port)
//   *
//   * \return Pulled locked data
//   */
//  tCCPortDataManager* GetPullInInterthreadContainerRaw(bool intermediate_assign, bool ignore_pull_request_handler_on_this_port);

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
   * \return Unit of port
   */
  inline tUnit GetUnit()
  {
    return unit;
  }

  virtual void NotifyDisconnect();

//  /*!
//   * Publish data
//   *
//   * \param tc ThreadLocalCache
//   * \param data Data to publish
//   */
//  inline void Publish(tThreadLocalCache* tc, tCCPortDataManagerTL* data)
//  {
//    PublishImpl<false, cCHANGED, false>(tc, data);
//  }
//
//  /*!
//   * Publish buffer through port
//   *
//   * \param read_object Buffer with data (must be owned by current thread)
//   */
//  inline void Publish(tCCPortDataManagerTL* buffer)
//  {
//    assert(buffer->GetOwnerThread() == rrlib::thread::tThread::CurrentThreadId());
//    Publish(tThreadLocalCache::GetFast(), buffer);
//  }

  /*!
   * \param New default value for port
   */
  void SetDefault(rrlib::rtti::tGenericObject& new_default);

  /*!
   * \param pull_request_handler Object that handles pull requests - null if there is none (typical case)
   */
  void SetPullRequestHandler(tPullRequestHandlerRaw* pull_request_handler);

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*!
   * Temporary set of variables with info on current publishing operation
   * (base class for both global and thread-local buffers)
   */
  struct tPublishingDataCommon
  {
    enum { cCOPY_ON_RECEIVE = 1 };

    /*! Tagged pointer to port data used in current publishing operation */
    tTaggedBufferPointer published_buffer_tagged_pointer;
  };

  /*!
   * Temporary set of variables with info on current publishing operation
   * (variant for global buffer)
   */
  struct tPublishingDataGlobalBuffer : public tPublishingDataCommon
  {
    /*! Number of locks already added to reference counter for current publishing operation */
    enum { cADD_LOCKS = 1000 };

    /*! Pointer to port data used in current publishing operation */
    tCheaplyCopiedBufferManager* published_buffer;

    /*! Number of locks that were required for assignments etc. */
    int used_locks;

    /*! Counter to use */
    int* used_locks_counter_to_use;

    tPublishingDataGlobalBuffer(tUnusedManagerPointer& published) :
      published_buffer(published.get()),
      used_locks(0),
      used_locks_counter_to_use(&used_locks)
    {
      int pointer_tag = published->InitReferenceCounter(cADD_LOCKS);
      published_buffer_tagged_pointer = tTaggedBufferPointer(published.release(), pointer_tag);
    }

    tPublishingDataGlobalBuffer() :
      published_buffer(NULL),
      used_locks(0),
      used_locks_counter_to_use(&used_locks)
    {
      published_buffer_tagged_pointer = 0;
    }

    // Removes obsolete locks
    ~tPublishingDataGlobalBuffer()
    {
      CheckRecycle();
    }

    inline void AddLock()
    {
      (*used_locks_counter_to_use)++;
    }

    inline bool AlreadyAssigned() const
    {
      return (*used_locks_counter_to_use);
    }

    inline void CheckRecycle()
    {
      if (published_buffer && (!IsCopy()))
      {
        assert((used_locks < cADD_LOCKS) && "Too many locks in this publishing operation");
        published_buffer->ReleaseLocks<tUnusedManagerPointer::deleter_type, tCheaplyCopiedBufferManager>(cADD_LOCKS - used_locks);
      }
    }

    /*!
     * Reinitializes publishing data with adjusted buffer
     */
    void Init(tUnusedManagerPointer& published)
    {
      CheckRecycle();
      used_locks = 0;
      used_locks_counter_to_use = &used_locks;
      published_buffer = published.get();
      int pointer_tag = published->InitReferenceCounter(cADD_LOCKS);
      published_buffer_tagged_pointer = tTaggedBufferPointer(published.release(), pointer_tag);
    }

    /*!
     * Reinitializes publishing data with buffer that already has cADD_LOCKS added
     */
    void InitSuccessfullyLocked(tCheaplyCopiedBufferManager* published)
    {
      CheckRecycle();
      used_locks = 0;
      used_locks_counter_to_use = &used_locks;
      published_buffer = published;
      published_buffer_tagged_pointer = tTaggedBufferPointer(published, published->GetPointerTag());
    }

    inline bool IsCopy()
    {
      return &used_locks != used_locks_counter_to_use;
    }

    /*!
     * \return Reference counter (If additional locks are required during publishing operation, adding to this counter is a safe and efficient way of doing this)
     */
    inline int& ReferenceCounter()
    {
      return (*used_locks_counter_to_use);
    }

  };

  /*!
   * Temporary set of variables with info on current publishing operation
   * (variant for thread-local buffer)
   */
  struct tPublishingDataThreadLocalBuffer : public tPublishingDataCommon
  {
    /*! Pointer to port data used in current publishing operation */
    tThreadLocalBufferManager* published_buffer;

    tPublishingDataThreadLocalBuffer(tUnusedManagerPointer& published) :
      published_buffer(static_cast<tThreadLocalBufferManager*>(published.get()))
    {
      int pointer_tag = published_buffer->IncrementReuseCounter();
      published_buffer_tagged_pointer = tTaggedBufferPointer(published.release(), pointer_tag);
    }

    tPublishingDataThreadLocalBuffer(tThreadLocalBufferManager* published, bool unused) :
      published_buffer(published)
    {
      int pointer_tag = unused ? published->IncrementReuseCounter() : published->GetPointerTag();
      published_buffer_tagged_pointer = tTaggedBufferPointer(published, pointer_tag);
    }

    tPublishingDataThreadLocalBuffer() : published_buffer(NULL)
    {
      published_buffer_tagged_pointer = 0;
    }

    ~tPublishingDataThreadLocalBuffer()
    {
      assert(((!published_buffer) || AlreadyAssigned()) && "Due to advantages w.r.t. computational overhead, buffers should always be assigned");
    }

    inline void AddLock()
    {
      published_buffer->AddThreadLocalLocks(1);
    }

    inline bool AlreadyAssigned() const
    {
      return published_buffer->GetThreadLocalReferenceCounter();
    }

    inline void CheckRecycle()
    {
      if (published_buffer && (!AlreadyAssigned()))
      {
        tUnusedBufferRecycler recycler;
        recycler(published_buffer);
      }
    }

    /*!
     * Reinitializes publishing data with adjusted buffer
     */
    void Init(tUnusedManagerPointer& published)
    {
      CheckRecycle();
      published_buffer = static_cast<tThreadLocalBufferManager*>(published.get());
      int pointer_tag = published_buffer->IncrementReuseCounter();
      published_buffer_tagged_pointer = tTaggedBufferPointer(published.release(), pointer_tag);
    }

    void Init(tThreadLocalBufferManager* published, bool unused)
    {
      CheckRecycle();
      published_buffer = published;
      int pointer_tag = unused ? published->IncrementReuseCounter() : published->GetPointerTag();
      published_buffer_tagged_pointer = tTaggedBufferPointer(published, pointer_tag);
    }

    /*!
     * \return Reference counter (If additional locks are required during publishing operation, adding to this counter is a safe and efficient way of doing this)
     */
    inline int& ReferenceCounter()
    {
      return published_buffer->ThreadLocalReferenceCounter();
    }
  };

  /*!
   * Convenience methods to retrieve unused buffer for specified publishing mode
   */
  tUnusedManagerPointer GetUnusedBuffer(tPublishingDataGlobalBuffer& publishing_data)
  {
    return tUnusedManagerPointer(tGlobalBufferPools::Instance().GetUnusedBuffer(cheaply_copyable_type_index).release());
  }
  tUnusedManagerPointer GetUnusedBuffer(tPublishingDataThreadLocalBuffer& publishing_data)
  {
    return tUnusedManagerPointer(tThreadLocalBufferPools::Get()->GetUnusedBuffer(cheaply_copyable_type_index).release());
  }

  /*!
   * \param publishing_data Info on current publish/pull operation
   */
  void LockCurrentValueForPublishing(tPublishingDataGlobalBuffer& publishing_data);
  void LockCurrentValueForPublishing(tPublishingDataThreadLocalBuffer& publishing_data);

  /*!
   * Custom special assignment to port.
   * Used, for instance, in queued ports.
   *
   * \param publishing_data Data on publishing operation for different types of buffers
   * \param change_contant Changed constant for current publishing operation (e.g. we do not want to enqueue values from initial pushing in port queues)
   * \return Whether setting value succeeded (fails, for instance, if bounded ports are set to discard values that are out of bounds)
   */
  virtual bool NonStandardAssign(tPublishingDataGlobalBuffer& publishing_data, tChangeStatus change_constant);
  virtual bool NonStandardAssign(tPublishingDataThreadLocalBuffer& publishing_data, tChangeStatus change_constant);

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  template <typename TPort, typename TPublishingData>
  friend class common::tPublishOperation;

  template <typename TPort, typename TPublishingData, typename TManager>
  friend class common::tPullOperation;

  /*! 'cheaply copyable type index' of type used in this port */
  uint32_t cheaply_copyable_type_index;

  /*! default value - invariant: must never be null if used (must always be copied, too) */
  std::unique_ptr<rrlib::rtti::tGenericObject> default_value;

  /*!
   * Current port value (never NULL)
   *
   * Tag stores a few bits of buffer reuse counter to avoid ABA problem
   * when retrieving buffer
   */
  std::atomic<typename tTaggedBufferPointer::tStorage> current_value;

  /*!
   * Data that is currently owned - used to belong to a terminated thread
   */
  //tCCPortDataManagerTL* owned_data;

  /*!
   * Is data assigned to port in standard way? Otherwise - for instance, when using queues -
   * the virtual method nonstandardassign will be invoked
   *
   * Hopefully compiler will optimize this, since it's final/const
   */
  const bool standard_assign;

  /*!
   * Optimization - if this is not null that means:
   * - this port is an output port and has one active receiver (stored in this variable)
   * - both ports are standard-assigned
   */
  //public CCPortBase std11CaseReceiver; // should not need to be volatile

  /*!
   * Port Index - derived from handle - for speed reasons
   */
  //const int port_index;

  /*! Queue for ports with incoming value queue */
  std::unique_ptr<common::tPortQueue<tLockingManagerPointer>> input_queue;

  /*! Object that handles pull requests - null if there is none (typical case) */
  tPullRequestHandlerRaw* pull_request_handler;

  /*! Unit of port (currently only used for numeric ports) */
  tUnit unit;


  /*!
   * Publishes new data to port.
   * Releases and unlocks old data.
   * Lock on new data has to be set before
   *
   * \param publishing_data Info on current publishing operation
   * \return Whether setting value succeeded (fails, for instance, if bounded ports are set to discard values that are out of bounds)
   */
  template <tChangeStatus CHANGE_CONSTANT, typename TPublishingData>
  inline bool Assign(TPublishingData& publishing_data)
  {
    assert(publishing_data.published_buffer->GetObject().GetType() == GetDataType());

    if (!standard_assign)
    {
      if (!NonStandardAssign(publishing_data, CHANGE_CONSTANT))
      {
        return false;
      }
    }

    // assign anyway
    publishing_data.AddLock();
    tTaggedBufferPointer old = current_value.exchange(publishing_data.published_buffer_tagged_pointer);
    tPortBufferUnlocker unlocker;
    unlocker(old.GetPointer());
    return true;
  }

  /*!
   * Calls pull request handler
   *
   * \param publishing_data Info on current pull operation
   */
  void CallPullRequestHandler(tPublishingDataGlobalBuffer& publishing_data, bool intermediate_assign);
  void CallPullRequestHandler(tPublishingDataThreadLocalBuffer& publishing_data, bool intermediate_assign);

//  /*!
//   * Get current data in container owned by this thread with a lock.
//   * Attention: User needs to take care of unlocking.
//   *
//   * \return Container (non-const - public wrapper should return it const)
//   */
//  tCCPortDataManagerTL* GetLockedUnsafeInContainer();

//  /*!
//   * Lock current buffer (or possibly copy it) for safe read access.
//   *
//   * \param add_locks number of locks to add (one will be released by unique_pointer returned by this method)
//   * \return Locked buffer
//   */
//  tLockingManagerPointer GetCurrentValueInGlobalBuffer(int add_locks = 1)
//  {
//    assert(add_locks > 0);
//
//    tTaggedBufferPointer current_buffer = current_value.load();
//    if (current_buffer->GetOwnerThreadId())
//    {
//      tGlobalBufferPools::tBufferPointer unused_manager = tGlobalBufferPools::Instance().GetUnusedBuffer();
//      GetCurrentValue(unused_manager->GetObject());
//
//    }
//
//
//    while(true)
//    {
//      else
//      {
//        if (current_buffer->TryLock(add_locks, current_buffer.GetStamp()))
//        {
//          // successful
//          return tLockingManagerPointer(current_buffer.GetPointer());
//        }
//      }
//    }
//  }
//
//  /*!
//   * Retrieves current value in thread local buffer
//   *
//   * \param add_locks number of locks to add (one will be released by unique_pointer returned by this method)
//   * \return Pointer to thread local buffer
//   * (this can the port's current buffer if the current thread is the owner - or a new buffer without any lock)
//   */
//  tThreadLocalBufferManager* GetCurrentValueInThreadLocalBuffer()
//  {
//    assert(add_locks > 0);
//
//    tTaggedBufferPointer current_buffer = current_value.load();
//    tCheaplyCopiedBufferManager* manager = current_buffer.GetPointer();
//    if (manager->IsOwnerThread())
//    {
//      return static_cast<tThreadLocalBufferManager*>(manager);
//    }
//
//    tThreadLocalBufferManager* unused_manager = tThreadLocalBufferPools::Get()->GetUnusedBuffer(cheaply_copyable_type_index);
//    for (; ;)
//    {
//      buffer.DeepCopyFrom(current_buffer->GetObject());
//      timestamp = current_buffer->GetTimestamp();
//      tTaggedBufferPointer::tStorage current_raw = current;
//      if (current_raw == current_value.load())    // still valid??
//      {
//        return;
//      }
//      current_buffer = current_value.load();
//    }
//  }

  virtual int GetMaxQueueLengthImplementation() const;

//  /*!
//   * (Meant for internal use)
//   *
//   * \param tc ThreadLocalCache
//   * \return Unused buffer for writing
//   */
//  inline tCCPortDataManagerTL* GetUnusedBuffer(tThreadLocalCache* tc)
//  {
//    return tc->GetUnusedBuffer(cc_type_index);
//  }

  virtual void InitialPushTo(tAbstractPort& target, bool reverse);

  /*!
   * Notify any port listeners of data change
   *
   * \param publishing_data Info on current publishing operation
   */
  template <tChangeStatus CHANGE_CONSTANT, typename TPublishingData>
  inline void NotifyListeners(TPublishingData& publishing_data)
  {
    if (GetPortListener())
    {
      tChangeContext change_context(*this, publishing_data.published_buffer->GetTimestamp(), CHANGE_CONSTANT);
      GetPortListener()->PortChangedRaw(change_context, publishing_data.ReferenceCounter(), *publishing_data.published_buffer);
    }
  }

//  /*!
//   * Publish data
//   *
//   * \param tc ThreadLocalCache
//   * \param data Data to publish
//   * \param reverse Value received in reverse direction?
//   * \param changed_constant changedConstant to use
//   */
//  inline void Publish(tThreadLocalCache* tc, tCCPortDataManagerTL* data, bool reverse, int8 changed_constant)
//  {
//    if (!reverse)
//    {
//      if (changed_constant == cCHANGED)
//      {
//        PublishImpl<false, cCHANGED, false>(tc, data);
//      }
//      else
//      {
//        PublishImpl<false, cCHANGED_INITIAL, false>(tc, data);
//      }
//    }
//    else
//    {
//      if (changed_constant == cCHANGED)
//      {
//        PublishImpl<true, cCHANGED, false>(tc, data);
//      }
//      else
//      {
//        PublishImpl<true, cCHANGED_INITIAL, false>(tc, data);
//      }
//    }
//  }

//  /*!
//   * Publish data
//   *
//   * \param tc ThreadLocalCache
//   * \param data Data to publish
//   * \param cREVERSE Value received in reverse direction?
//   * \param cCHANGE_CONSTANT changedConstant to use
//   * \param cBROWSER_PUBLISH Inform this port's listeners on change and also publish in reverse direction? (only set from BrowserPublish())
//   */
//  template <bool cREVERSE, int8 cCHANGE_CONSTANT, bool cBROWSER_PUBLISH>
//  inline void PublishImpl(tThreadLocalCache* tc, tCCPortDataManagerTL* data)
//  {
//    assert((data->GetObject()->GetType() != NULL) && "Port data type not initialized");
//    if (!(IsInitialized() || cBROWSER_PUBLISH))
//    {
//      PrintNotReadyMessage("Ignoring publishing request.");
//
//      // Possibly recycle
//      data->AddLock();
//      data->ReleaseLock();
//      return;
//    }
//
//    util::tArrayWrapper<tCheapCopyPort*>* dests = cREVERSE ? edges_dest.GetIterable() : edges_src.GetIterable();
//
//    // assign
//    tc->data = data;
//    tc->ref = data->GetCurrentRef();
//    Assign(tc);
//
//    // inform listeners?
//    if (cBROWSER_PUBLISH)
//    {
//      SetChanged(cCHANGE_CONSTANT);
//      NotifyListeners(tc);
//    }
//
//    // later optimization (?) - unroll loops for common short cases
//    for (size_t i = 0u; i < dests->Size(); i++)
//    {
//      tCheapCopyPort* dest = dests->Get(i);
//      bool push = (dest != NULL) && dest->WantsPush<cREVERSE, cCHANGE_CONSTANT>(cREVERSE, cCHANGE_CONSTANT);
//      if (push)
//      {
//        dest->Receive<cREVERSE, cCHANGE_CONSTANT>(tc, *this, cREVERSE, cCHANGE_CONSTANT);
//      }
//    }
//
//    if (cBROWSER_PUBLISH)
//    {
//      assert(!cREVERSE);
//
//      // reverse
//      dests = edges_dest.GetIterable();
//      for (int i = 0, n = dests->Size(); i < n; i++)
//      {
//        tCheapCopyPort* dest = dests->Get(i);
//        bool push = (dest != NULL) && dest->WantsPush<true, cCHANGE_CONSTANT>(true, cCHANGE_CONSTANT);
//        if (push)
//        {
//          dest->Receive<true, cCHANGE_CONSTANT>(tc, *this, true, cCHANGE_CONSTANT);
//        }
//      }
//    }
//  }

  /*!
   * Pull/read current value from source port
   * When multiple source ports are available, an arbitrary one of them is used.
   *
   * \param intermediate_assign Assign pulled value to ports in between?
   * \param ignore_pull_request_handler_on_this_port Ignore pull request handler on first port? (for network port pulling it's good if pullRequestHandler is not called on first port)
   * \return Locked port data (current thread is owner; there is one additional lock for caller; non-const(!))
   */
  tLockingManagerPointer PullValueRaw(bool intermediate_assign = true, bool ignore_pull_request_handler_on_this_port = false);

//  virtual void SetMaxQueueLengthImpl(int length);

  /*!
   * Update statistics if this is enabled
   *
   * \param publishing_data Info on current publishing operation
   * \param source Source port
   * \param target Target Port
   */
  template <typename TPublishingData>
  inline void UpdateStatistics(TPublishingData& publishing_data, tCheapCopyPort& source, tCheapCopyPort& target)
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

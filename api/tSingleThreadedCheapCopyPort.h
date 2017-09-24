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
/*!\file    plugins/data_ports/api/tSingleThreadedCheapCopyPort.h
 *
 * \author  Max Reichardt
 *
 * \date    2014-06-23
 *
 * \brief   Contains tSingleThreadedCheapCopyPort
 *
 * \b tSingleThreadedCheapCopyPort
 *
 * Single threaded port implementation for 'cheaply copied' data types
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__api__tSingleThreadedCheapCopyPort_h__
#define __plugins__data_ports__api__tSingleThreadedCheapCopyPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/tSingleThreadedCheapCopyPortGeneric.h"

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
template <typename T>
class tPortDataPointer;

namespace api
{
template <typename T, bool CHEAPLY_COPIED_TYPE>
class tPortDataPointerImplementation;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Single threaded port implementation for 'cheaply copied' data types
/*!
 * Single threaded port implementation for 'cheaply copied' data types
 */
template <typename T>
class tSingleThreadedCheapCopyPort : public optimized::tSingleThreadedCheapCopyPortGeneric
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  typedef std::pair<T, rrlib::time::tTimestamp> tQueueEntry;
  typedef std::deque<tQueueEntry> tQueue;
  typedef tPortDataPointer<const T> tLockingManagerPointer;

  tSingleThreadedCheapCopyPort(common::tAbstractDataPortCreationInfo creation_info) :
    tSingleThreadedCheapCopyPortGeneric(creation_info),
    input_queue()
  {
    if (GetFlag(tFlag::HAS_QUEUE))
    {
      max_queue_length = creation_info.max_queue_size;
      input_queue.reset(new std::deque<tQueueEntry>());
    }
  }

  /*!
   * \param current_timestamp Current timestamp is copied to this object
   * \return Current port value
   */
  inline const T& CurrentValue()
  {
    return *static_cast<T*>(current_value.data_pointer);
  }

  /*!
   * Copy current value to buffer (Most efficient get()-version)
   *
   * \param buffer Buffer to copy current data to
   * \param timestamp Buffer to copy attached time stamp to
   * \param strategy Strategy to use for get operation
   */
  void CopyCurrentValueToGenericObject(rrlib::rtti::tGenericObject& buffer, rrlib::time::tTimestamp& timestamp, tStrategy strategy = tStrategy::DEFAULT)
  {
    timestamp = CurrentValueTimestamp();
    buffer.GetData<T>() = CurrentValue();
  }

  /*!
   * \return fragment Reference to input queue
   */
  tQueue& DequeueAllRaw()
  {
    return *input_queue;
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
    if (input_queue->empty())
    {
      return tPortDataPointer<T>();
    }
    tQueueEntry entry = input_queue->front();
    input_queue->pop_front();
    return tPortDataPointerImplementation<T, true>(entry.first, entry.second);
  }

  /*!
   * Publish data
   *
   * \param data Data to publish
   * \param timestamp Timestamp belonging to data
   */
  void Publish(const T& data, rrlib::time::tTimestamp timestamp)
  {
    if (!GetFlag(tFlag::HIJACKED_PORT))
    {
      *static_cast<T*>(current_value.data_pointer) = data;
      current_value.timestamp = timestamp;
      common::tPublishOperation<tSingleThreadedCheapCopyPort<T>, tPublishingData> publish_operation(current_value);
      publish_operation.template Execute<tChangeStatus::CHANGED, false, false>(*this);
    }
  }

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  virtual bool NonStandardAssign(tPublishingData& publishing_data, tChangeStatus change_constant) override
  {
    if (GetFlag(tFlag::USES_QUEUE) && change_constant != tChangeStatus::CHANGED_INITIAL)
    {
      assert(input_queue);

      // enqueue
      while (max_queue_length > 0 && input_queue->size() >= static_cast<size_t>(max_queue_length))
      {
        input_queue->pop_front();
      }
      input_queue->emplace_back(publishing_data.Value<T>(), publishing_data.value->timestamp);
    }
    return true;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  template <typename TPort, typename TPublishingData>
  friend class common::tPublishOperation;

  /*! Queue for ports with incoming value queue */
  std::unique_ptr<std::deque<tQueueEntry>> input_queue;


  /*!
   * Publishes new data to port.
   * Releases and unlocks old data.
   * Lock on new data has to be set before
   *
   * \param publishing_data Info on current publishing operation
   * \return Whether setting value succeeded (fails, for instance, if bounded ports are set to discard values that are out of bounds)
   */
  template <tChangeStatus CHANGE_CONSTANT>
  inline bool Assign(tPublishingData& publishing_data)
  {
    if (!standard_assign)
    {
      if (!NonStandardAssign(publishing_data, CHANGE_CONSTANT))
      {
        return false;
      }
    }

    // assign anyway
    *static_cast<T*>(current_value.data_pointer) = publishing_data.Value<T>();
    current_value.timestamp = publishing_data.value->timestamp;
    return true;
  }

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

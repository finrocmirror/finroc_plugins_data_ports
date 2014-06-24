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
/*!\file    plugins/data_ports/optimized/tSingleThreadedCheapCopyPortGeneric.h
 *
 * \author  Max Reichardt
 *
 * \date    2014-06-22
 *
 * \brief   Contains tSingleThreadedCheapCopyPortGeneric
 *
 * \b tSingleThreadedCheapCopyPortGeneric
 *
 * Single threaded port implementation for 'cheaply copied' data types.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__optimized__tSingleThreadedCheapCopyPortGeneric_h__
#define __plugins__data_ports__optimized__tSingleThreadedCheapCopyPortGeneric_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tAbstractDataPort.h"
#include "plugins/data_ports/common/tPublishOperation.h"
#include "plugins/data_ports/tChangeContext.h"
#include "plugins/data_ports/optimized/tPullRequestHandlerRaw.h"

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
// Class declaration
//----------------------------------------------------------------------
//! Single threaded port implementation for 'cheaply copied' data types.
/*!
 * Single threaded port implementation for 'cheaply copied' data types
 * (generic base class - queues and bounds are only supported with typed
 *  subclasses: tSingleThreadedCheapCopyPort and tBoundedPort)
 */
class tSingleThreadedCheapCopyPortGeneric : public common::tAbstractDataPort
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  typedef std::unique_ptr<int> tLockingManagerPointer; // dummy

  /*! Stores current port's value */
  struct tCurrentValueBuffer : public rrlib::buffer_pools::tBufferManagementInfo
  {
    /*! Contains buffer with current value */
    std::unique_ptr<rrlib::rtti::tGenericObject> data;

    /*! 'cheaply copyable type index' of type used in this port */
    uint32_t cheaply_copyable_type_index;

    /*! Pointer to data buffer with current value (optimization - avoids one indirection) */
    void* data_pointer;

    /*! Timestamp of current port value */
    rrlib::time::tTimestamp timestamp;

  };

  struct tPublishingData
  {
    enum { cCOPY_ON_RECEIVE = 1 };

    const tCurrentValueBuffer* value;

    tPublishingData(const tCurrentValueBuffer& value) : value(&value) {}

    template <typename T>
    T Value()
    {
      assert(typeid(T).name() == value->data->GetType().GetRttiName());
      return *static_cast<T*>(value->data_pointer);
    }

    inline void CheckRecycle() {}
  };


  /*!
   * \param creation_info Port creation information
   */
  tSingleThreadedCheapCopyPortGeneric(common::tAbstractDataPortCreationInfo creation_info);

  virtual ~tSingleThreadedCheapCopyPortGeneric();

  /*!
   * Set current value to default value
   */
  void ApplyDefaultValue();

  /*!
   * Publish buffer through port
   * (not in normal operation, but from browser; difference: listeners on this port will be notified)
   *
   * \param buffer Buffer with data
   * \param timestamp Timestamp of data
   * \param notify_listener_on_this_port Notify listener on this port?
   * \param change_constant Change constant to use for publishing operation
   * \return Error message if something did not work
   */
  virtual std::string BrowserPublishRaw(const rrlib::rtti::tGenericObject& buffer, rrlib::time::tTimestamp timestamp,
                                        bool notify_listener_on_this_port = true, tChangeStatus change_constant = tChangeStatus::CHANGED);

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
    buffer.DeepCopyFrom(*current_value.data);
  }

  /*!
   * \return Pointer to data buffer with current value
   */
  inline const tCurrentValueBuffer& CurrentValueBuffer()
  {
    return current_value;
  }

  /*!
   * \return Pointer to data buffer with current value
   */
  inline void* CurrentValuePointer()
  {
    return current_value.data_pointer;
  }

  /*!
   * Timestamp of current port value
   */
  inline rrlib::time::tTimestamp CurrentValueTimestamp()
  {
    return current_value.timestamp;
  }

  virtual void ForwardData(tAbstractDataPort& other) override;

  /*!
   * \return Returns data type's 'cheaply copyable type index'
   */
  inline uint32_t GetCheaplyCopyableTypeIndex() const
  {
    return current_value.cheaply_copyable_type_index;
  }

  /*!
   * \return Default value that has been assigned to port (NULL if no default value set)
   */
  const rrlib::rtti::tGenericObject* GetDefaultValue() const
  {
    return default_value.get();
  }

  /*!
   * Publish data
   *
   * \param data Data to publish
   * \param timestamp Timestamp belonging to data
   */
  void Publish(const rrlib::rtti::tGenericObject& data, rrlib::time::tTimestamp timestamp);

  /*!
   * Use specified memory address to store current port value in
   *
   * \param address Memory address of buffer to use (must have size of at least GetDataType().GetSize()).
   */
  void SetCurrentValueBuffer(void* address);

  /*!
   * \param New default value for port
   */
  void SetDefault(rrlib::rtti::tGenericObject& new_default);

  /*!
   * \param pull_request_handler Object that handles pull requests - null if there is none (typical case)
   */
  void SetPullRequestHandler(tPullRequestHandlerRaw* pull_request_handler)
  {
    // not implemented (I guess we should remove that feature)
  }

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*! Contains buffer with current value */
  tCurrentValueBuffer current_value;

  /*! Contains buffer with default value */
  std::unique_ptr<rrlib::rtti::tGenericObject> default_value;

  /*! Maximum length of queue */
  int max_queue_length;

  /*!
   * Is data assigned to port in standard way? Otherwise - for instance, when using queues -
   * the virtual method nonstandardassign will be invoked
   *
   * Hopefully compiler will optimize this, since it's final/const
   */
  const bool standard_assign;

  /*!
   * Custom special assignment to port.
   * Used, for instance, in queued ports.
   *
   * \param publishing_data Data on publishing operation for different types of buffers
   * \param change_contant Changed constant for current publishing operation (e.g. we do not want to enqueue values from initial pushing in port queues)
   * \return Whether setting value succeeded (fails, for instance, if bounded ports are set to discard values that are out of bounds)
   */
  virtual bool NonStandardAssign(tPublishingData& publishing_data, tChangeStatus change_constant);

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  template <typename TPort, typename TPublishingData>
  friend class common::tPublishOperation;

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
    current_value.data->DeepCopyFrom(*publishing_data.value->data); // could be optimized (e.g. memcpy)
    current_value.timestamp = publishing_data.value->timestamp;
    return true;
  }

  virtual int GetMaxQueueLengthImplementation() const override;
  virtual void InitialPushTo(tAbstractPort& target, bool reverse) override;

  /*!
   * Notify any port listeners of data change
   *
   * \param publishing_data Info on current publishing operation
   */
  template <tChangeStatus CHANGE_CONSTANT>
  inline void NotifyListeners(tPublishingData& publishing_data)
  {
    if (GetPortListener())
    {
      tChangeContext change_context(*this, publishing_data.value->timestamp, CHANGE_CONSTANT);
      int lock_counter_unused = 0;
      GetPortListener()->PortChangedRaw(change_context, lock_counter_unused, const_cast<tCurrentValueBuffer&>(*publishing_data.value));
    }
  }

  inline void UpdateStatistics(tPublishingData& publishing_data, tSingleThreadedCheapCopyPortGeneric& source, tSingleThreadedCheapCopyPortGeneric& target) {}
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

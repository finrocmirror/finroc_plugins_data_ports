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
/*!\file    plugins/data_ports/tGenericPort.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 * \brief   Contains tGenericPort
 *
 * \b tGenericPort
 *
 * Wrapper class for ports whose type is not known at compile time.
 *
 * (note: Get and Publish are currently only implemented based on deep-copying data.
 *  Therefore, Generic ports are only efficient for data types that can be copied cheaply)
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tGenericPort_h__
#define __plugins__data_ports__tGenericPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/port/tPortWrapperBase.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tGenericPortImplementation.h"
#include "plugins/data_ports/tEvent.h"

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

namespace api
{
template <typename LISTENER, bool FIRST_LISTENER>
class tPortListenerAdapterGeneric;
template <typename LISTENER, bool FIRST_LISTENER>
class tPortListenerAdapterGenericForPointer;
}

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Generic port
/*!
 * Wrapper class for ports whose type is not known at compile time.
 *
 * (note: Get and Publish are currently only implemented based on deep-copying data.
 *  Therefore, Generic ports are only efficient for data types that can be copied cheaply)
 */
class tGenericPort : public core::tPortWrapperBase
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Empty constructor in case port is created later
   */
  tGenericPort() :
    implementation()
  {}

  /*!
   * Constructor takes variadic argument list... just any properties you want to assign to port.
   *
   * The first string is interpreted as port name, the second possibly as config entry (relevant for parameters only).
   * A framework element pointer is interpreted as parent.
   * unsigned int arguments are interpreted as flags.
   * A tQueueSettings argument creates an input queue with the specified settings.
   * tBounds<T> are port's bounds.
   * tPortCreationBase argument is copied. This is only allowed as first argument.
   */
  template <typename ... ARGS>
  tGenericPort(const ARGS&... args)
  {
    tConstructorArguments<common::tAbstractDataPortCreationInfo> creation_info(args...);
    if ((creation_info.data_type.GetTypeTraits() & rrlib::rtti::trait_flags::cIS_BINARY_SERIALIZABLE) == 0)
    {
      throw std::runtime_error("Only binary serializable types may be used in data ports");
    }
    implementation = api::tGenericPortImplementation::GetImplementation(creation_info.data_type);
    SetWrapped(implementation->CreatePort(creation_info));
  }

  /*!
   * \param listener Listener to add
   *
   * \tparam LISTENER Listener class needs to implement a method
   * void OnPortChange(const rrlib::rtti::tGenericObject& value, tChangeContext& change_context)
   *
   * (It's preferred to add listeners before port is initialized)
   * (Note: Buffer in 'value' always has data type of port backend (e.g. tNumber instead of double)
   */
  template <typename TListener>
  void AddPortListener(TListener& listener);

  /*!
   * \param listener Listener to add
   *
   * \tparam LISTENER Listener class needs to implement a method
   * void OnPortChange(tPortDataPointer<const rrlib::rtti::tGenericObject>& value, tChangeContext& change_context)
   *
   * (It's preferred to add listeners before port is initialized)
   * (Note: Buffer in 'value' always has data type of port backend (e.g. tNumber instead of double)
   */
  template <typename TListener>
  void AddPortListenerForPointer(TListener& listener);

  /*!
   * \param listener Listener to add
   *
   * \tparam LISTENER Listener class needs to implement a method
   * void OnPortChange(tChangeContext& change_context)
   *
   * (It's preferred to add listeners before port is initialized)
   */
  template <typename TListener>
  void AddPortListenerSimple(TListener& listener);

  /*!
   * Apply default value to port
   * (not particularly efficient)
   *
   * apply_type_default_if_no_port_default_defined If no default value was set in constructor, use default of data type? (otherwise prints a warning)
   */
  void ApplyDefault(bool apply_type_default_if_no_port_default_defined)
  {
    if (GetDefaultValue() || (!apply_type_default_if_no_port_default_defined))
    {
      static_cast<common::tAbstractDataPort&>(*GetWrapped()).ApplyDefaultValue();
    }
    else
    {
      tPortDataPointer<rrlib::rtti::tGenericObject> buffer = GetUnusedBuffer();
      std::unique_ptr<rrlib::rtti::tGenericObject> default_buffer(buffer->GetType().CreateInstanceGeneric());
      buffer->DeepCopyFrom(*default_buffer);
      BrowserPublish(buffer);
    }
  }

  /*!
   * Publish buffer through port
   * (not in normal operation, but from browser; difference: listeners on this port will be notified)
   *
   * \param pointer Buffer with data
   * \param notify_listener_on_this_port Notify listener on this port?
   * \param change_constant Change constant to use for publishing operation
   * \return Error message if something did not work
   */
  inline std::string BrowserPublish(tPortDataPointer<rrlib::rtti::tGenericObject>& pointer, bool notify_listener_on_this_port = true,
                                    tChangeStatus change_constant = tChangeStatus::CHANGED)
  {
    if (IsCheaplyCopiedType(pointer->GetType()))
    {
#ifndef RRLIB_SINGLE_THREADED
      typename optimized::tCheapCopyPort::tUnusedManagerPointer pointer2(static_cast<optimized::tCheaplyCopiedBufferManager*>(pointer.implementation.Release()));
      return static_cast<optimized::tCheapCopyPort*>(GetWrapped())->BrowserPublishRaw(pointer2, notify_listener_on_this_port, change_constant);
#else
      return static_cast<optimized::tSingleThreadedCheapCopyPortGeneric*>(GetWrapped())->BrowserPublishRaw(*pointer, pointer.GetTimestamp(), notify_listener_on_this_port, change_constant);
#endif
    }
    else
    {
      typename standard::tStandardPort::tUnusedManagerPointer pointer2(static_cast<standard::tPortBufferManager*>(pointer.implementation.Release()));
      static_cast<standard::tStandardPort*>(GetWrapped())->BrowserPublish(pointer2, notify_listener_on_this_port, change_constant);
    }
    return "";
  }

  /*!
   * Gets port's current value
   *
   * \param result Buffer to (deep) copy port's current value to
   * \param timestamp Buffer to copy timestamp attached to data to (optional)
   */
  inline void Get(rrlib::rtti::tGenericObject& result)
  {
    rrlib::time::tTimestamp timestamp;
    Get(result, timestamp);
  }
  inline void Get(rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp)
  {
    implementation->Get(*GetWrapped(), result, timestamp);
  }

  /*!
   * Gets Port's current value in buffer
   *
   * \param strategy Strategy to use for get operation
   * \return Buffer with port's current value with read lock.
   *
   * Note: Buffer always has data type of port backend (e.g. tNumber instead of double)
   * If this is not desired, use pass-by-value-Get Operation above.
   */
  inline tPortDataPointer<const rrlib::rtti::tGenericObject> GetPointer(tStrategy strategy = tStrategy::DEFAULT)
  {
    return implementation->GetPointer(*GetWrapped(), strategy);
  }

  /*!
   * \return Port's default value (NULL if none has been set)
   */
  const rrlib::rtti::tGenericObject* GetDefaultValue()
  {
    return implementation->GetDefaultValue(*GetWrapped());
  }

  /*!
   * Note: Buffer always has data type of port backend (e.g. tNumber instead of double)
   * If this is not desired, use pass-by-value-Publish Operation below.
   *
   * \return Unused buffer.
   * Buffers to be published using this port should be acquired using this function.
   * The buffer might contain old data, so it should be cleared prior to using.
   */
  inline tPortDataPointer<rrlib::rtti::tGenericObject> GetUnusedBuffer()
  {
    return implementation->GetUnusedBuffer(*GetWrapped());
  }

  /*!
   * \return Wrapped port. For rare case that someone really needs to access ports.
   */
  inline common::tAbstractDataPort* GetWrapped() const
  {
    return static_cast<common::tAbstractDataPort*>(tPortWrapperBase::GetWrapped());
  }

  /*!
   * \return Has port changed since last changed-flag-reset?
   */
  inline bool HasChanged() const
  {
    return this->GetWrapped()->HasChanged();
  }

  /*!
   * Publish Data Buffer. This data will be forwarded to any connected ports.
   * Should only be called on output ports.
   *
   * \param data Data to publish. It will be deep-copied.
   * \param timestamp Timestamp to attach to data.
   */
  inline void Publish(const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp = rrlib::time::cNO_TIME)
  {
    implementation->Publish(*GetWrapped(), data, timestamp);
  }

  /*!
   * Set new bounds
   * (This is not thread-safe and must only be done in "pause mode")
   *
   * \param b New Bounds
   */
  inline void SetBounds(const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max)
  {
    implementation->SetBounds(*GetWrapped(), min, max);
  }

  /*!
   * \param pull_request_handler Object that handles any incoming pull requests - null if there is none (typical case)
   */
  void SetPullRequestHandler(tPullRequestHandler<rrlib::rtti::tGenericObject>* pull_request_handler)
  {
    implementation->SetPullRequestHandler(*GetWrapped(), pull_request_handler);
  }

  /*!
   * Wraps raw port
   * Throws std::runtime_error if port to wrap has invalid type.
   *
   * \param wrap Type-less tAbstractPort to wrap as tGenericPort
   * \param use_backend_type_only Use only the internal data type that used in the port backend? (e.g. tNumber instead of double; relevant e.g. for Get() and Publish() methods)
   */
  static tGenericPort Wrap(core::tAbstractPort& wrap, bool use_backend_type_only = false)
  {
    if (!IsDataFlowType(wrap.GetDataType()))
    {
      throw rrlib::util::tTraceableException<std::runtime_error>(wrap.GetDataType().GetName() + " is no data flow type and cannot be wrapped.");
    }
    tGenericPort port;
    port.SetWrapped(&wrap);
    port.implementation = api::tGenericPortImplementation::GetImplementation(
                            ((!use_backend_type_only) && wrap.GetWrapperDataType() != NULL) ? wrap.GetWrapperDataType() : wrap.GetDataType());
    return port;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  template <typename LISTENER, bool FIRST_LISTENER>
  friend class api::tPortListenerAdapterGeneric;
  template <typename LISTENER, bool FIRST_LISTENER>
  friend class api::tPortListenerAdapterGenericForPointer;

  /** Implementation of port functionality */
  api::tGenericPortImplementation* implementation;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

#include "plugins/data_ports/tInputPort.h"
#include "plugins/data_ports/tGenericPort.hpp"
#include "plugins/data_ports/api/tPortListenerAdapter.h"

#endif

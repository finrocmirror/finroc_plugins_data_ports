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
/*!\file    plugins/data_ports/api/tGenericPortImplementation.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 * \brief   Contains tGenericPortImplementation
 *
 * \b tGenericPortImplementation
 *
 * Implementations for tGenericPort.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__api__tGenericPortImplementation_h__
#define __plugins__data_ports__api__tGenericPortImplementation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tPortImplementation.h"
#include "plugins/data_ports/common/tAbstractDataPortCreationInfo.h"
#include "plugins/data_ports/common/tAbstractDataPort.h"

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
class tPullRequestHandler;

namespace api
{

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Generic port implementation
/*!
 * Implementations for tGenericPort.
 */
class tGenericPortImplementation
{
//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  typedef typename std::conditional<definitions::cSINGLE_THREADED, optimized::tSingleThreadedCheapCopyPortGeneric, optimized::tCheapCopyPort>::type tCheapCopyPort;

  /*!
   * Creates port backend for specified port creation info
   *
   * \param creation_info Information for port creation
   * \return Created port
   */
  virtual core::tAbstractPort* CreatePort(const common::tAbstractDataPortCreationInfo& creation_info) = 0;

  /*!
   * Gets Port's current value
   *
   * \param port Wrapped port to operate on
   * \param result Buffer to (deep) copy port's current value to
   * \param timestamp Object to store timestamp in
   *
   * (Using this get()-variant is more efficient when using cheaply-copied types, but can be costly with large data types)
   */
  virtual void Get(core::tAbstractPort& port, rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp) = 0;

  /*!
   * Gets Port's current value buffer
   *
   * \param port Wrapped port to operate on
   * \param strategy Strategy to use for get operation
   * \return Buffer with port's current value with read lock.
   */
  virtual tPortDataPointer<const rrlib::rtti::tGenericObject> GetPointer(core::tAbstractPort& port, tStrategy strategy) = 0;

  /*!
   * \param port Wrapped port to operate on
   * \return Port's default value (NULL if none has been set)
   */
  const rrlib::rtti::tGenericObject* GetDefaultValue(core::tAbstractPort& port)
  {
    return IsCheaplyCopiedType(port.GetDataType()) ? static_cast<tCheapCopyPort&>(port).GetDefaultValue() : static_cast<standard::tStandardPort&>(port).GetDefaultValue();
  }

  /*!
   * \param Data type to get implementation for
   * \return Implementation for specified data type
   */
  static tGenericPortImplementation* GetImplementation(const rrlib::rtti::tType& type)
  {
    return IsCheaplyCopiedType(type) ? cIMPLEMENTATION_CHEAP_COPY : cIMPLEMENTATION_STANDARD;
  }

  /*!
   * \param port Wrapped port to operate on
   * \return Unused buffer.
   *
   * Buffers to be published using this port should be acquired using this function.
   * The buffer might contain old data, so it should be cleared prior to using.
   */
  inline tPortDataPointer<rrlib::rtti::tGenericObject> GetUnusedBuffer(core::tAbstractPort& port)
  {
    if (IsCheaplyCopiedType(port.GetDataType()))
    {
      tCheapCopyPort& cc_port = static_cast<tCheapCopyPort&>(port);
      if (optimized::tThreadLocalBufferPools::Get())
      {
        return tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false>(optimized::tThreadLocalBufferPools::Get()->GetUnusedBuffer(cc_port.GetCheaplyCopyableTypeIndex()).release(), true);
      }
      else
      {
        return tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false>(optimized::tGlobalBufferPools::Instance().GetUnusedBuffer(cc_port.GetCheaplyCopyableTypeIndex()).release(), true);
      }
    }
    else
    {
      return tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false>(static_cast<standard::tStandardPort&>(port).GetUnusedBufferRaw().release(), true);
    }
  }

  /*!
   * Publish Data Buffer. This data will be forwarded to any connected ports.
   * Should only be called on output ports.
   *
   * \param port Wrapped port to operate on
   * \param data Data to publish. It will be deep-copied.
   * \param timestamp Timestamp to attach to data
   *
   * This publish()-variant is efficient when using cheaply-copied types, but can be costly with large data types)
   */
  virtual void Publish(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp) = 0;

  /*!
   * Publish Data Buffer. This data will be forwarded to any connected ports.
   * Should only be called on output ports.
   *
   * \param port Wrapped port to operate on
   * \param data_buffer Data to publish.
   *
   * This publish()-variant is efficient with all data types.
   */
  inline void Publish(core::tAbstractPort& port, tPortDataPointer<rrlib::rtti::tGenericObject>& data_buffer)
  {
    if (IsCheaplyCopiedType(port.GetDataType()))
    {
      tCheapCopyPort& cc_port = static_cast<tCheapCopyPort&>(port);
#ifndef RRLIB_SINGLE_THREADED
      if (optimized::tThreadLocalBufferPools::Get())
      {
        common::tPublishOperation<optimized::tCheapCopyPort, typename optimized::tCheapCopyPort::tPublishingDataThreadLocalBuffer> publish_operation(static_cast<optimized::tThreadLocalBufferManager*>(data_buffer.implementation.Release()), true);
        publish_operation.Execute<tChangeStatus::CHANGED, false, false>(cc_port);
      }
      else
      {
        optimized::tCheapCopyPort::tUnusedManagerPointer pointer(static_cast<optimized::tCheaplyCopiedBufferManager*>(data_buffer.implementation.Release()));
        common::tPublishOperation<optimized::tCheapCopyPort, typename optimized::tCheapCopyPort::tPublishingDataGlobalBuffer> publish_operation(pointer);
        publish_operation.Execute<tChangeStatus::CHANGED, false, false>(cc_port);
      }
#else
      cc_port.Publish(*data_buffer, data_buffer.GetTimestamp());
#endif
    }
    else
    {
      standard::tStandardPort::tUnusedManagerPointer buffer(static_cast<standard::tPortBufferManager*>(data_buffer.implementation.Release()));
      assert(buffer->IsUnused());
      static_cast<standard::tStandardPort&>(port).Publish(buffer);
    }
  }

  /*!
   * Set new bounds
   * (This is not thread-safe and must only be done in "pause mode")
   *
   * \param port Wrapped port to operate on
   * \param min Minimum value
   * \param max Maximum value
   */
  virtual void SetBounds(core::tAbstractPort& port, const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max) = 0;

  /*!
   * \param port Wrapped port to operate on
   * \param pull_request_handler Object that handles any incoming pull requests - null if there is none (typical case)
   */
  void SetPullRequestHandler(core::tAbstractPort& port, tPullRequestHandler<rrlib::rtti::tGenericObject>* pull_request_handler);

private:

  /*! The two available implementations */
  static tGenericPortImplementation* cIMPLEMENTATION_STANDARD, *cIMPLEMENTATION_CHEAP_COPY;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

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
/*!\file    plugins/data_ports/api/tPortListenerAdapter.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-21
 *
 * \brief   Contains tPortListenerAdapter
 *
 * \b tPortListenerAdapter
 *
 * Adapts tPortListenerRaw to tPortListener<T>.
 * Therefore, type T must be extracted from tGenericObjectManager reference.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__api__tPortListenerAdapter_h__
#define __plugins__data_ports__api__tPortListenerAdapter_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tPortImplementation.h"
#include "plugins/data_ports/api/tPortImplementationTypeTrait.h"
#include "plugins/data_ports/common/tPortListenerRaw.h"

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
class tInputPort;
class tGenericPort;

namespace api
{

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------

// Adapter base class: first listener
template <typename LISTENER, bool FIRST_LISTENER>
class tPortListenerAdapterBase : public common::tPortListenerRaw
{
public:

  tPortListenerAdapterBase(LISTENER& listener) :
    listener(listener)
  {}

  inline void PortChangedRawBase(common::tAbstractDataPort& origin, int& lock_counter,
                                 rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {}

  virtual void PortDeleted()
  {
    delete this;
  }

  /*! Listener */
  LISTENER& listener;
};

// Adapter base class: chained listener
template <typename LISTENER>
class tPortListenerAdapterBase<LISTENER, false> : public common::tPortListenerRaw
{
public:

  tPortListenerAdapterBase(LISTENER& listener, tPortListenerRaw& previous_listener) :
    listener(listener),
    previous_listener(previous_listener)
  {}

  inline void PortChangedRawBase(common::tAbstractDataPort& origin, int& lock_counter,
                                 rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    previous_listener.PortChangedRaw(origin, lock_counter, value, timestamp);
  }

  virtual void PortDeleted()
  {
    previous_listener.PortDeleted();
    delete this;
  }

  /*! Listener */
  LISTENER& listener;
  tPortListenerRaw& previous_listener;
};

// Normal adapter class for cheaply copied type
template <typename LISTENER, typename T, tPortImplementationType TPortImplementationType, bool FIRST_LISTENER>
class tPortListenerAdapter : public tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>
{
public:

  template <typename ... TArgs>
  tPortListenerAdapter(LISTENER& listener, TArgs& ... args) : tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>(listener, args...) {}

private:

  typedef tPortImplementation<T, TPortImplementationType> tImplementation;

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, int& lock_counter,
                              rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    this->PortChangedRawBase(origin, lock_counter, value, timestamp);
    T v = tImplementation::ToValue(
            static_cast<optimized::tCheaplyCopiedBufferManager&>(value).GetObject().GetData<typename tImplementation::tPortBuffer>(),
            static_cast<optimized::tCheapCopyPort&>(origin).GetUnit());
    tInputPort<T> port;
    port.SetWrapped(&origin);
    this->listener.PortChanged(port, v, timestamp);
  }
};

// Normal adapter class for standard type
template <typename LISTENER, typename T, bool FIRST_LISTENER>
class tPortListenerAdapter<LISTENER, T, tPortImplementationType::STANDARD, FIRST_LISTENER> : public tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>
{
public:

  template <typename ... TArgs>
  tPortListenerAdapter(LISTENER& listener, TArgs& ... args) : tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>(listener, args...) {}

private:

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, int& lock_counter,
                              rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    this->PortChangedRawBase(origin, lock_counter, value, timestamp);
    tInputPort<T> port;
    port.SetWrapped(&origin);
    this->listener.PortChanged(port, static_cast<standard::tPortBufferManager&>(value).GetObject().GetData<T>(), timestamp);
  }
};

// Normal adapter class for generic port
template <typename LISTENER, bool FIRST_LISTENER>
class tPortListenerAdapterGeneric : public tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>
{
public:

  template <typename ... TArgs>
  tPortListenerAdapterGeneric(LISTENER& listener, TArgs& ... args) : tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>(listener, args...) {}

private:

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, int& lock_counter,
                              rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    this->PortChangedRawBase(origin, lock_counter, value, timestamp);
    tGenericPort port;
    port.SetWrapped(&origin);
    if (IsCheaplyCopiedType(origin.GetDataType()))
    {
      this->listener.PortChanged(port, static_cast<optimized::tCheaplyCopiedBufferManager&>(value).GetObject(), timestamp);
    }
    else
    {
      this->listener.PortChanged(port, static_cast<standard::tPortBufferManager&>(value).GetObject(), timestamp);
    }
  }
};

// Variant for smart pointer
template <typename LISTENER, typename T, bool FIRST_LISTENER>
class tPortListenerAdapterForPointer : public tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>
{
public:

  template <typename ... TArgs>
  tPortListenerAdapterForPointer(LISTENER& listener, TArgs& ... args) : tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>(listener, args...) {}

private:

  typedef typename std::conditional<tIsCheaplyCopiedType<T>::value, optimized::tCheaplyCopiedBufferManager, standard::tPortBufferManager>::type tBufferManager;
  typedef tPortImplementation<T, tPortImplementationTypeTrait<T>::type> tImplementation;

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, int& lock_counter,
                              rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    this->PortChangedRawBase(origin, lock_counter, value, timestamp);
    tBufferManager& manager = static_cast<tBufferManager&>(value);
    PortChangedRawImplementation(origin, lock_counter, manager, timestamp);
  }

  inline void PortChangedRawImplementation(common::tAbstractDataPort& origin, int& lock_counter,
      standard::tPortBufferManager& value, const rrlib::time::tTimestamp& timestamp)
  {
    lock_counter++;
    tPortDataPointer<const T> pointer(typename standard::tStandardPort::tLockingManagerPointer(&value), static_cast<standard::tStandardPort&>(origin));
    tInputPort<T> port;
    port.SetWrapped(&origin);
    this->listener.PortChanged(port, pointer, timestamp);
  }

  inline void PortChangedRawImplementation(common::tAbstractDataPort& origin, int& lock_counter,
      optimized::tCheaplyCopiedBufferManager& value, const rrlib::time::tTimestamp& timestamp)
  {
    T data = tImplementation::ToValue(value.GetObject().GetData<typename tImplementation::tPortBuffer>(), static_cast<optimized::tCheapCopyPort&>(origin).GetUnit());
    tPortDataPointer<const T> pointer(tPortDataPointerImplementation<T, true>(data, timestamp));
    tInputPort<T> port;
    port.SetWrapped(&origin);
    this->listener.PortChanged(port, pointer, timestamp);
  }
};

// Variant for smart pointer with generic port
template <typename LISTENER, bool FIRST_LISTENER>
class tPortListenerAdapterGenericForPointer : public tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>
{
public:

  template <typename ... TArgs>
  tPortListenerAdapterGenericForPointer(LISTENER& listener, TArgs& ... args) : tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>(listener, args...) {}

private:

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, int& lock_counter,
                              rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    this->PortChangedRawBase(origin, lock_counter, value, timestamp);
    lock_counter++;
    tGenericPort port;
    port.SetWrapped(&origin);
    if (IsCheaplyCopiedType(origin.GetDataType()))
    {
      tPortDataPointer<const rrlib::rtti::tGenericObject> pointer(
        api::tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false>(static_cast<optimized::tCheaplyCopiedBufferManager*>(&value), false));
      this->listener.PortChanged(port, pointer, timestamp);
    }
    else
    {
      tPortDataPointer<const rrlib::rtti::tGenericObject> pointer(
        api::tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false>(static_cast<standard::tPortBufferManager*>(&value), false));
      this->listener.PortChanged(port, pointer, timestamp);
    }
  }
};

// Simple port adapter
template <typename LISTENER, bool FIRST_LISTENER>
class tPortListenerAdapterSimple : public tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>
{
public:

  template <typename ... TArgs>
  tPortListenerAdapterSimple(LISTENER& listener, TArgs& ... args) : tPortListenerAdapterBase<LISTENER, FIRST_LISTENER>(listener, args...) {}

private:

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, int& lock_counter,
                              rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    this->PortChangedRawBase(origin, lock_counter, value, timestamp);
    this->listener.PortChanged(origin);
  }
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

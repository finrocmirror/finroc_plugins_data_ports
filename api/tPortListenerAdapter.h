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
namespace api
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Adapts tPortListenerRaw to tPortListener<T>
/*!
 * Adapts tPortListenerRaw to tPortListener<T>.
 * Therefore, type T must be extracted from tGenericObjectManager reference.
 */
template <typename LISTENER, typename T, tPortImplementationType TPortImplementationType, bool FIRST_LISTENER>
class tPortListenerAdapter : public common::tPortListenerRaw
{
public:

  tPortListenerAdapter(LISTENER& listener) :
    listener(listener)
  {}

private:

  typedef tPortImplementation<T, TPortImplementationType> tImplementation;

  virtual void PortDeleted()
  {
    delete this;
  }

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    T v = tImplementation::ToValue(
            static_cast<optimized::tCheaplyCopiedBufferManager&>(value).GetObject().GetData<typename tImplementation::tPortBuffer>(),
            static_cast<optimized::tCheapCopyPort&>(origin).GetUnit());
    tInputPort<T> port;
    port.SetWrapped(&origin);
    listener.PortChanged(port, v, timestamp);
  }

  LISTENER& listener;
};

// Chained, CC type
template <typename LISTENER, typename T, tPortImplementationType TPortImplementationType>
class tPortListenerAdapter<LISTENER, T, TPortImplementationType, false> : public common::tPortListenerRaw
{
public:

  tPortListenerAdapter(LISTENER& listener, tPortListenerRaw& previous_listener) :
    listener(listener),
    previous_listener(previous_listener)
  {}

private:

  typedef tPortImplementation<T, TPortImplementationType> tImplementation;

  virtual void PortDeleted()
  {
    previous_listener.PortDeleted();
    delete this;
  }

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    previous_listener.PortChangedRaw(origin, value, timestamp);
    T v = tImplementation::ToValue(
            static_cast<optimized::tCheaplyCopiedBufferManager&>(value).GetObject().GetData<typename tImplementation::tPortBuffer>(),
            static_cast<optimized::tCheapCopyPort&>(origin).GetUnit());
    tInputPort<T> port;
    port.SetWrapped(&origin);
    listener.PortChanged(port, v, timestamp);
  }

  LISTENER& listener;
  tPortListenerRaw& previous_listener;
};

// First, Standard type
template <typename LISTENER, typename T>
class tPortListenerAdapter<LISTENER, T, tPortImplementationType::STANDARD, true> : public common::tPortListenerRaw
{
public:

  tPortListenerAdapter(LISTENER& listener) :
    listener(listener)
  {}

private:

  typedef tPortImplementation<T, tPortImplementationType::STANDARD> tImplementation;

  virtual void PortDeleted()
  {
    delete this;
  }

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    tInputPort<T> port;
    port.SetWrapped(&origin);
    listener.PortChanged(port, static_cast<standard::tPortBufferManager&>(value).GetObject().GetData<T>(), timestamp);
  }

  LISTENER& listener;
};

// Chained, Standard type
template <typename LISTENER, typename T>
class tPortListenerAdapter<LISTENER, T, tPortImplementationType::STANDARD, false> : public common::tPortListenerRaw
{
public:

  tPortListenerAdapter(LISTENER& listener, tPortListenerRaw& previous_listener) :
    listener(listener),
    previous_listener(previous_listener)
  {}

private:

  typedef tPortImplementation<T, tPortImplementationType::STANDARD> tImplementation;

  virtual void PortDeleted()
  {
    previous_listener.PortDeleted();
    delete this;
  }

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    previous_listener.PortChangedRaw(origin, value, timestamp);
    tInputPort<T> port;
    port.SetWrapped(&origin);
    listener.PortChanged(port, static_cast<standard::tPortBufferManager&>(value).GetObject().GetData<T>(), timestamp);
  }

  LISTENER& listener;
  tPortListenerRaw& previous_listener;
};

// Variant for smart pointer with auto-lock-release
template <typename LISTENER, typename T, bool FIRST_LISTENER>
class tPortListenerAdapterForPointer : public common::tPortListenerRaw
{
public:

  tPortListenerAdapterForPointer(LISTENER& listener) : listener(listener) {}

private:

  typedef typename std::conditional<tIsCheaplyCopiedType<T>::value, optimized::tCheaplyCopiedBufferManager, standard::tPortBufferManager>::type tBufferManager;
  typedef tPortImplementation<T, tPortImplementationTypeTrait<T>::type> tImplementation;

  virtual void PortDeleted()
  {
    delete this;
  }

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    tBufferManager& manager = static_cast<tBufferManager&>(value);
    PortChangedRawImplementation(origin, manager, timestamp);
  }

  inline void PortChangedRawImplementation(common::tAbstractDataPort& origin, standard::tPortBufferManager& value, const rrlib::time::tTimestamp& timestamp)
  {
    value.AddLocks(1);
    tPortDataPointer<const T> pointer(typename standard::tStandardPort::tLockingManagerPointer(&value), static_cast<standard::tStandardPort&>(origin));
    tInputPort<T> port;
    port.SetWrapped(&origin);
    listener.PortChanged(port, pointer, timestamp);
  }

  inline void PortChangedRawImplementation(common::tAbstractDataPort& origin, optimized::tCheaplyCopiedBufferManager& value, const rrlib::time::tTimestamp& timestamp)
  {
    T data = tImplementation::ToValue(value.GetObject().GetData<typename tImplementation::tPortBuffer>(), static_cast<optimized::tCheapCopyPort&>(origin).GetUnit());
    tPortDataPointer<const T> pointer(tPortDataPointerImplementation<T, true>(data, timestamp));
    tInputPort<T> port;
    port.SetWrapped(&origin);
    listener.PortChanged(port, pointer, timestamp);
  }

  LISTENER& listener;
};

// Variant for smart pointer with auto-lock-release, chained
template <typename LISTENER, typename T>
class tPortListenerAdapterForPointer<LISTENER, T, false> : public common::tPortListenerRaw
{
public:

  tPortListenerAdapterForPointer(LISTENER& listener, tPortListenerRaw& previous_listener) : listener(listener), previous_listener(previous_listener) {}

private:

  typedef typename std::conditional<tIsCheaplyCopiedType<T>::value, optimized::tCheaplyCopiedBufferManager, standard::tPortBufferManager>::type tBufferManager;
  typedef tPortImplementation<T, tPortImplementationTypeTrait<T>::type> tImplementation;

  virtual void PortDeleted()
  {
    previous_listener.PortDeleted();
    delete this;
  }

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    previous_listener.PortChangedRaw(origin, value, timestamp);
    tBufferManager& manager = static_cast<tBufferManager&>(value);
    PortChangedRawImplementation(origin, manager, timestamp);
  }

  inline void PortChangedRawImplementation(common::tAbstractDataPort& origin, standard::tPortBufferManager& value, const rrlib::time::tTimestamp& timestamp)
  {
    value.AddLocks(1);
    tPortDataPointer<const T> pointer(typename standard::tStandardPort::tLockingManagerPointer(&value), static_cast<standard::tStandardPort&>(origin));
    tInputPort<T> port;
    port.SetWrapped(&origin);
    listener.PortChanged(port, pointer, timestamp);
  }

  inline void PortChangedRawImplementation(common::tAbstractDataPort& origin, optimized::tCheaplyCopiedBufferManager& value, const rrlib::time::tTimestamp& timestamp)
  {
    T data = tImplementation::ToValue(value.GetObject().GetData<typename tImplementation::tPortBuffer>(), static_cast<optimized::tCheapCopyPort&>(origin).GetUnit());
    tPortDataPointer<const T> pointer(tPortDataPointerImplementation<T, true>(data, timestamp));
    tInputPort<T> port;
    port.SetWrapped(&origin);
    listener.PortChanged(port, pointer, timestamp);
  }

  LISTENER& listener;
  tPortListenerRaw& previous_listener;
};

// simple, first
template <typename LISTENER, bool FIRST_LISTENER>
class tPortListenerAdapterSimple : public common::tPortListenerRaw
{
public:

  tPortListenerAdapterSimple(LISTENER& listener) :
    listener(listener)
  {}

private:

  virtual void PortDeleted()
  {
    delete this;
  }

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    listener.PortChanged(origin);
  }

  LISTENER& listener;
};

// simple, chained
template <typename LISTENER>
class tPortListenerAdapterSimple<LISTENER, false> : public common::tPortListenerRaw
{
public:

  tPortListenerAdapterSimple(LISTENER& listener, tPortListenerRaw& previous_listener) :
    listener(listener),
    previous_listener(previous_listener)
  {}

private:

  virtual void PortDeleted()
  {
    previous_listener.PortDeleted();
    delete this;
  }

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    previous_listener.PortChangedRaw(origin, value, timestamp);
    listener.PortChanged(origin);
  }

  LISTENER& listener;
  tPortListenerRaw& previous_listener;
};



//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

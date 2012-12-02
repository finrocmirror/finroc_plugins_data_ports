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
template <typename T, tPortImplementationType TPortImplementationType>
class tPortListenerAdapter : public common::tPortListenerRaw
{
  typedef tPortImplementation<T, TPortImplementationType> tImplementation;

  virtual void PortChanged(common::tAbstractDataPort& origin, const T& value, const rrlib::time::tTimestamp& timestamp) = 0;

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    T v;
    tImplementation::Assign(v,
                            static_cast<optimized::tCheaplyCopiedBufferManager&>(value).GetObject().GetData<typename tImplementation::tPortBuffer>(),
                            static_cast<optimized::tCheapCopyPort&>(origin));
    PortChanged(origin, v, timestamp);
  }
};

template <typename T>
class tPortListenerAdapter<T, tPortImplementationType::STANDARD> : public common::tPortListenerRaw
{
  virtual void PortChanged(common::tAbstractDataPort& origin, const T& value, const rrlib::time::tTimestamp& timestamp) = 0;

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    PortChanged(origin, static_cast<standard::tPortBufferManager&>(value).GetObject().GetData<T>(), timestamp);
  }
};

/*! variants for const void* */
template <tPortImplementationType TPortImplementationType>
class tPortListenerAdapter<const void*, TPortImplementationType> : public common::tPortListenerRaw
{
  virtual void PortChanged(common::tAbstractDataPort& origin, const void* const& value, const rrlib::time::tTimestamp& timestamp) = 0;

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    PortChanged(origin, static_cast<optimized::tCheaplyCopiedBufferManager&>(value).GetObject().GetRawDataPointer(), timestamp);
  }
};

class tVoidPointerPortListenerAdapter : public common::tPortListenerRaw
{
  virtual void PortChanged(common::tAbstractDataPort& origin, const void* value, const rrlib::time::tTimestamp& timestamp) = 0;

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    PortChanged(origin, static_cast<standard::tPortBufferManager&>(value).GetObject().GetRawDataPointer(), timestamp);
  }
};

/*! Variant for smart pointer with auto-lock-release */
template <typename T>
class tPointerPortListenerAdapter : public common::tPortListenerRaw
{
  typedef typename std::conditional<tIsCheaplyCopiedType<T>::value, optimized::tCheaplyCopiedBufferManager, standard::tPortBufferManager>::type tBufferManager;
  typedef tPortImplementation<T, tPortImplementationTypeTrait<T>::type> tImplementation;

  virtual void PortChanged(common::tAbstractDataPort& origin, tPortDataPointer<const T>& value) = 0;

  virtual void PortChangedRaw(common::tAbstractDataPort& origin, rrlib::buffer_pools::tBufferManagementInfo& value, const rrlib::time::tTimestamp& timestamp)
  {
    tBufferManager& manager = static_cast<tBufferManager&>(value);
    PortChangedRawImplementation(origin, manager, timestamp);
  }

  inline void PortChangedRawImplementation(common::tAbstractDataPort& origin, standard::tPortBufferManager& value, const rrlib::time::tTimestamp& timestamp)
  {
    value.AddLocks(1);
    tPortDataPointer<const T> pointer(typename standard::tStandardPort::tLockingManagerPointer(&value), static_cast<standard::tStandardPort&>(origin));
    PortChanged(origin, pointer, timestamp);
  }

  inline void PortChangedRawImplementation(common::tAbstractDataPort& origin, optimized::tCheaplyCopiedBufferManager& value, const rrlib::time::tTimestamp& timestamp)
  {
    T data;
    tImplementation::Assign(data, value.GetObject().template GetData<typename tImplementation::tPortBuffer>(), static_cast<optimized::tCheapCopyPort&>(origin));
    tPortDataPointer<const T> pointer(tPortDataPointerImplementation<T, true>(data, timestamp));
    PortChanged(origin, pointer, timestamp);
  }
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

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
/*!\file    plugins/data_ports/api/tPortDataPointerImplementation.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-17
 *
 * \brief   Contains tPortDataPointerImplementation
 *
 * \b tPortDataPointerImplementation
 *
 * Actual implementation of smart pointer class.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__api__tPortDataPointerImplementation_h__
#define __plugins__data_ports__api__tPortDataPointerImplementation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/standard/tStandardPort.h"

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
//! tPortDataPointer implementation
/*!
 * Actual implementation of smart pointer class.
 */
template <typename T, bool CHEAPLY_COPIED_TYPE>
class tPortDataPointerImplementation : boost::noncopyable
{
  typedef typename std::remove_const<T>::type tPortData;

  /*! Port-related functionality for type T */
  typedef api::tPortImplementation<tPortData, api::tPortImplementationTypeTrait<tPortData>::type> tPortImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  inline tPortDataPointerImplementation() : buffer_manager(NULL)
  {
  }

  inline tPortDataPointerImplementation(typename tPortImplementation::tPortBase::tLockingManagerPointer& pointer, typename tPortImplementation::tPortBase& port) :
    buffer_manager(pointer.release())
  {
  }

  inline tPortDataPointerImplementation(typename tPortImplementation::tPortBase::tUnusedManagerPointer& pointer) :
    buffer_manager(pointer.release())
  {
  }

  // Move constructor
  inline tPortDataPointerImplementation(tPortDataPointerImplementation && other) :
    buffer_manager(NULL)
  {
    std::swap(buffer_manager, other.buffer_manager);
  }

  // Move assignment
  inline tPortDataPointerImplementation& operator=(tPortDataPointerImplementation && other)
  {
    std::swap(buffer_manager, other.buffer_manager);
    return *this;
  }

  inline ~tPortDataPointerImplementation()
  {
    if (buffer_manager)
    {
      if (buffer_manager->IsUnused())
      {
        // recycle unused buffer
        typename standard::tStandardPort::tUnusedManagerPointer::deleter_type deleter;
        deleter(buffer_manager);
      }
      else
      {
        // reduce reference counter
        typename standard::tStandardPort::tLockingManagerPointer::deleter_type deleter;
        deleter(buffer_manager);
      }
    }
  }

  inline T* Get() const
  {
    return buffer_manager->GetObject().GetData<T>();
  }

  inline rrlib::time::tTimestamp GetTimestamp() const
  {
    return buffer_manager->timestamp;
  }

  inline standard::tPortBufferManager* Release()
  {
    standard::tPortBufferManager* temp = buffer_manager;
    buffer_manager = NULL;
    return temp;
  }

  inline void SetTimestamp(const rrlib::time::tTimestamp& timestamp)
  {
    buffer_manager->SetTimestamp(timestamp);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Locked buffer */
  standard::tPortBufferManager* buffer_manager;
};

template <typename T>
class tPortDataPointerImplementation<T, true> : boost::noncopyable
{
  /*! Port-related functionality for type T */
  typedef api::tPortImplementation<T, api::tPortImplementationTypeTrait<T>::type> tPortImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  inline tPortDataPointerImplementation() :
    buffer(), // Initializes even fundamental types
    timestamp(rrlib::time::cNO_TIME)
  {
  }

  inline tPortDataPointerImplementation(typename tPortImplementation::tPortBase::tLockingManagerPointer& pointer, optimized::tCheapCopyPort& port) :
    buffer(tPortImplementation::ToValue(pointer->GetObject().template GetData<typename tPortImplementation::tPortBuffer>(), port.GetUnit())),
    timestamp(pointer->GetTimestamp())
  {
  }

  inline tPortDataPointerImplementation(const T& value, const rrlib::time::tTimestamp timestamp) :
    buffer(value),
    timestamp(timestamp)
  {
  }

  // Move constructor
  inline tPortDataPointerImplementation(tPortDataPointerImplementation && other) :
    buffer(), // Initializes even fundamental types
    timestamp(rrlib::time::cNO_TIME)
  {
    std::swap(buffer, other.buffer);
    std::swap(timestamp, other.timestamp);
  }

  // Move assignment
  inline tPortDataPointerImplementation& operator=(tPortDataPointerImplementation && other)
  {
    std::swap(buffer, other.buffer);
    std::swap(timestamp, other.timestamp);
    return *this;
  }

  inline T* Get()
  {
    return &buffer;
  }

  inline rrlib::time::tTimestamp GetTimestamp() const
  {
    return timestamp;
  }

  inline void SetTimestamp(const rrlib::time::tTimestamp& timestamp)
  {
    this->timestamp = timestamp;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Wrapped buffer */
  T buffer;

  /*! Wrapped time stamp */
  rrlib::time::tTimestamp timestamp;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

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
#include "plugins/data_ports/api/tDeserializationScope.h"
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
template <typename U, bool>
class tPullRequestHandlerAdapter;
class tPullRequestHandlerAdapterGeneric;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! tPortDataPointer implementation
/*!
 * Actual implementation of smart pointer class.
 */
template <typename T, bool CHEAPLY_COPIED_TYPE>
class tPortDataPointerImplementation : private rrlib::util::tNoncopyable
{
  typedef typename std::remove_const<T>::type tPortData;

  /*! Port-related functionality for type T */
  typedef api::tPortImplementation<tPortData, api::tPortImplementationTypeTrait<tPortData>::type> tPortImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  inline tPortDataPointerImplementation() : buffer_manager()
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

  inline tPortDataPointerImplementation(typename tPortImplementation::tPortBase::tLockingManagerPointer& pointer) :
    buffer_manager(pointer.release())
  {
  }

  // Move constructor
  inline tPortDataPointerImplementation(tPortDataPointerImplementation && other) :
    buffer_manager()
  {
    std::swap(buffer_manager, other.buffer_manager);
  }

  // Move assignment
  inline tPortDataPointerImplementation& operator=(tPortDataPointerImplementation && other)
  {
    std::swap(buffer_manager, other.buffer_manager);
    return *this;
  }

  void Deserialize(rrlib::serialization::tInputStream& stream)
  {
    if (stream.ReadBoolean())
    {
      if (!Get())
      {
        *this = tPortDataPointerImplementation();
        buffer_manager.reset(tDeserializationScope::GetBufferSource().GetUnusedBuffer(rrlib::rtti::tDataType<T>()).release());
      }
      stream >> *Get();
      rrlib::time::tTimestamp timestamp;
      stream >> timestamp;
      SetTimestamp(timestamp);
    }
    else
    {
      *this = tPortDataPointerImplementation();
    }
  }

  inline T* Get() const
  {
    return buffer_manager ? (&buffer_manager->GetObject().template GetData<T>()) : NULL;
  }

  inline rrlib::time::tTimestamp GetTimestamp() const
  {
    return buffer_manager->GetTimestamp();
  }

  inline standard::tPortBufferManager* Release()
  {
    standard::tPortBufferManager* temp = buffer_manager.release();
    buffer_manager = NULL;
    return temp;
  }

  void Serialize(rrlib::serialization::tOutputStream& stream) const
  {
    stream.WriteBoolean(Get());
    if (Get())
    {
      stream << *Get();
      stream << GetTimestamp();
    }
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
  typename standard::tStandardPort::tUniversalManagerPointer buffer_manager;
};

template <typename T>
class tPortDataPointerImplementation<T, true> : private rrlib::util::tNoncopyable
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

  void Deserialize(rrlib::serialization::tInputStream& stream)
  {
    stream.ReadBoolean();
    stream >> *Get();
    stream >> this->timestamp;
  }

  inline T* Get()
  {
    return &buffer;
  }

  inline const T* Get() const
  {
    return &buffer;
  }

  inline rrlib::time::tTimestamp GetTimestamp() const
  {
    return timestamp;
  }

  void Serialize(rrlib::serialization::tOutputStream& stream) const
  {
    stream.WriteBoolean(true);
    stream << *Get();
    stream << GetTimestamp();
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

template <>
class tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false> : private rrlib::util::tNoncopyable
{
  typedef rrlib::rtti::tGenericObject tPortData;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  inline tPortDataPointerImplementation() : buffer_manager(NULL), object(NULL), unused_buffer(false)
  {
  }

  template <typename TManager>
  inline tPortDataPointerImplementation(TManager* manager, bool unused) :
    buffer_manager(manager),
    object(&manager->GetObject()),
    unused_buffer(unused)
  {
  }

  // Move constructor
  inline tPortDataPointerImplementation(tPortDataPointerImplementation && other) :
    buffer_manager(NULL), object(NULL), unused_buffer(false)
  {
    std::swap(buffer_manager, other.buffer_manager);
    std::swap(object, other.object);
    std::swap(unused_buffer, other.unused_buffer);
  }

  // Move assignment
  inline tPortDataPointerImplementation& operator=(tPortDataPointerImplementation && other)
  {
    std::swap(buffer_manager, other.buffer_manager);
    std::swap(object, other.object);
    std::swap(unused_buffer, other.unused_buffer);
    return *this;
  }

  inline ~tPortDataPointerImplementation()
  {
    if (buffer_manager)
    {
      if (unused_buffer)
      {
        if (IsCheaplyCopiedType(object->GetType()))
        {
          // recycle unused buffer
          typename optimized::tCheapCopyPort::tUnusedManagerPointer::deleter_type deleter;
          deleter(static_cast<optimized::tCheaplyCopiedBufferManager*>(buffer_manager));
        }
        else
        {
          // recycle unused buffer
          typename standard::tStandardPort::tUnusedManagerPointer::deleter_type deleter;
          deleter(static_cast<standard::tPortBufferManager*>(buffer_manager));
        }
      }
      else
      {
        if (IsCheaplyCopiedType(object->GetType()))
        {
          // reduce reference counter
          typename optimized::tCheapCopyPort::tLockingManagerPointer::deleter_type deleter;
          deleter(static_cast<optimized::tCheaplyCopiedBufferManager*>(buffer_manager));
        }
        else
        {
          // reduce reference counter
          typename standard::tStandardPort::tLockingManagerPointer::deleter_type deleter;
          deleter(static_cast<standard::tPortBufferManager*>(buffer_manager));
        }
      }
    }
  }

  void Deserialize(rrlib::serialization::tInputStream& stream)
  {
    if (stream.ReadBoolean())
    {
      rrlib::rtti::tType type;
      stream >> type;
      if ((!Get()) || Get()->GetType() != type)
      {
        tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false> buffer = tDeserializationScope::GetUnusedBuffer(type);
        std::swap(*this, buffer);
      }
      Get()->Deserialize(stream);
      rrlib::time::tTimestamp timestamp;
      stream >> timestamp;
      SetTimestamp(timestamp);
    }
    else
    {
      *this = tPortDataPointerImplementation();
    }
  }

  inline rrlib::rtti::tGenericObject* Get() const
  {
    return object;
  }

  inline rrlib::time::tTimestamp GetTimestamp() const
  {
    return buffer_manager->GetTimestamp();
  }

  bool IsUnused() const
  {
    return unused_buffer;
  }

  inline common::tReferenceCountingBufferManager* Release()
  {
    common::tReferenceCountingBufferManager* temp = buffer_manager;
    buffer_manager = NULL;
    return temp;
  }

  void Serialize(rrlib::serialization::tOutputStream& stream) const
  {
    stream.WriteBoolean(Get());
    if (Get())
    {
      stream << Get()->GetType();
      Get()->Serialize(stream);
      stream << GetTimestamp();
    }
  }

  inline void SetTimestamp(const rrlib::time::tTimestamp& timestamp)
  {
    buffer_manager->SetTimestamp(timestamp);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Locked buffer - if standard type */
  common::tReferenceCountingBufferManager* buffer_manager;

  /*! Generic object this pointer points to */
  rrlib::rtti::tGenericObject* object;

  /*! Unused buffer? */
  bool unused_buffer;
};
//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

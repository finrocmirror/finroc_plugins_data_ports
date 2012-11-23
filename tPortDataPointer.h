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
/*!\file    plugins/data_ports/tPortDataPointer.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-25
 *
 * \brief   Contains tPortDataPointer
 *
 * \b tPortDataPointer
 *
 * Smart pointer class for data buffers obtained from ports.
 * Works similarly to std::unique_ptr
 * (efficient, no internal memory allocation, can only be moved)
 * As long as the smart pointer exists, accessing the object it points to is
 * safe (as it locked for reading and - if T is non-const - also writing)
 *
 * For cheaply copied types, contains an internal buffer of Type T.
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tPortDataPointer_h__
#define __plugins__data_ports__tPortDataPointer_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tPortDataPointerImplementation.h"

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

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Smart port data buffer pointer
/*!
 * Smart pointer class for data buffers obtained from ports.
 * Works similarly to std::unique_ptr
 * (efficient, no internal memory allocation, can only be moved)
 * As long as the smart pointer exists, accessing the object it points to is
 * safe (as it locked for reading and - if T is non-const - also writing)
 *
 * For cheaply copied types, contains an internal buffer of Type T.
 */
template <typename T>
class tPortDataPointer : boost::noncopyable
{
  typedef typename std::remove_const<T>::type tPortData;

  /*! Port-related functionality for type T */
  typedef api::tPortImplementation<tPortData, api::tPortImplementationTypeTrait<tPortData>::type> tPortImplementation;

  /*! Actual implementation of smart pointer class */
  typedef api::tPortDataPointerImplementation<tPortData, tIsCheaplyCopiedType<tPortData>::value> tImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  inline tPortDataPointer() : implementation()
  {}

  inline tPortDataPointer(typename tPortImplementation::tPortBase::tLockingManagerPointer& pointer, typename tPortImplementation::tPortBase& port) :
    implementation(pointer, port)
  {
  }

  // Move constructor
  inline tPortDataPointer(tPortDataPointer && other) : implementation()
  {
    std::swap(implementation, other.implementation);
  }

  inline tPortDataPointer(tImplementation && impl) : implementation()
  {
    std::swap(implementation, impl);
  }

  // Move assignment
  inline tPortDataPointer& operator=(tPortDataPointer && other)
  {
    std::swap(implementation, other.implementation);
    return *this;
  }

  /*!
   * \return Pointer to port data
   */
  inline T* Get()
  {
    return implementation.Get();
  }
  inline T* get()
  {
    return Get();
  }

  /*!
   * \return Timestamp attached to data
   */
  rrlib::time::tTimestamp GetTimestamp()
  {
    return implementation.GetTimestamp();
  }

  /*!
   * \param timestamp Timestamp to attach to data
   */
  void SetTimestamp(const rrlib::time::tTimestamp& timestamp)
  {
    implementation.SetTimestamp(timestamp);
  }

  inline T& operator*()
  {
    assert(Get() != NULL);
    return *Get();
  }

  inline T* operator->()
  {
    assert(Get() != NULL);
    return Get();
  }

  inline operator const void*()
  {
    return Get();
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend struct api::tPortImplementation<tPortData, api::tPortImplementationTypeTrait<tPortData>::type>;

  /*! Actual implementation of smart pointer class */
  tImplementation implementation;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

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
/*!\file    plugins/data_ports/api/tPortBufferReturnCustomization.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-18
 *
 * \brief   Contains tPortBufferReturnCustomization
 *
 * \b tPortBufferReturnCustomization
 *
 * Extension to tPortImplementation:
 * Returns the desired type for reading constant port buffer.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__api__tPortBufferReturnCustomization_h__
#define __plugins__data_ports__api__tPortBufferReturnCustomization_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tPortImplementationTypeTrait.h"
#include "plugins/data_ports/api/tPortDataPointerImplementation.h"

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
//! Customized read access to port buffers
/*!
 * Extension to tPortImplementation:
 * Returns the desired type for reading constant port buffer.
 *
 * \tparam T Can be plain type to get buffers by value (only available for cheaply-copied types)
 *           Can alternatively be tPortDataPointer<const ...> to get pointer to buffer.
 */
// plain-type implementation
template <typename T>
struct tPortBufferReturnCustomization : public tPortImplementation<T, tPortImplementationTypeTrait<T>::type>
{
  static_assert(tIsCheaplyCopiedType<T>::value, "Only available for cheaply-copied types. Use tPortBufferReturnCustomization<tPortDataPointer<T>> instead.");

  typedef typename std::remove_const<T>::type tPortDataType;
  typedef tPortImplementation<T, tPortImplementationTypeTrait<T>::type> tBase;
  typedef typename tBase::tPortBuffer tPortBuffer;
  typedef typename tBase::tPortBase::tPortBufferContainerPointer tPortBufferContainerPointer;

  static T ToDesiredType(const tPortBufferContainerPointer& locked_buffer, optimized::tCheapCopyPort& port)
  {
    const tPortBuffer& value_buffer = locked_buffer->locked_buffer->GetObject().template GetData<tPortBuffer>();
    locked_buffer->locked_buffer.reset();
    return tBase::ToValue(value_buffer, port.GetUnit());
  }
};

// tPortDataPointer<T> implementation
template <typename T>
struct tPortBufferReturnCustomization<tPortDataPointer<T>> : public tPortImplementation<typename std::remove_const<T>::type, tPortImplementationTypeTrait<typename std::remove_const<T>::type>::type>
{
  typedef typename std::remove_const<T>::type tPortDataType;
  typedef tPortImplementation<tPortDataType, tPortImplementationTypeTrait<tPortDataType>::type> tBase;
  typedef typename tBase::tPortBase tPortBase;
  typedef typename tBase::tPortBase::tPortBufferContainerPointer tPortBufferContainerPointer;
  typedef typename tBase::tPortBase::tLockingManagerPointer tLockingManagerPointer;

  static tPortDataPointer<T> ToDesiredType(const tPortBufferContainerPointer& locked_buffer, tPortBase& port)
  {
    return tPortDataPointer<T>(std::move(locked_buffer->locked_buffer), port);
  }
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

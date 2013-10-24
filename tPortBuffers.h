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
/*!\file    plugins/data_ports/tPortBuffers.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-25
 *
 * \brief   Contains tPortBuffers
 *
 * \b tPortBuffers
 *
 * A list of port buffers obtained from an input port's input queue.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tPortBuffers_h__
#define __plugins__data_ports__tPortBuffers_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tPortBufferReturnCustomization.h"

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
//! List of port buffer
/*!
 * A list of port buffers obtained from an input port's input queue.
 *
 * \tparam T Can be plain type to get buffers by value (only available for cheaply-copied types)
 *           Can alternatively be tPortDataPointer<const ...> to get pointer to buffer.
 */
template <typename T>
class tPortBuffers : private rrlib::util::tNoncopyable
{
  /*! Class that contains implementation of buffer access */
  typedef api::tPortBufferReturnCustomization<T> tImplementation;

  /*! Typeless port class used in backend */
  typedef typename tImplementation::tPortBase tPortBase;

  /*! Type of wrapped queue fragment */
  typedef rrlib::concurrent_containers::tQueueFragment<typename tPortBase::tPortBufferContainerPointer> tQueueFragment;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tPortBuffers(tQueueFragment && fragment, tPortBase& port) :
    queue_fragment(),
    port(&port)
  {
    std::swap(queue_fragment, fragment);
  }

  /*! Move constructor */
  tPortBuffers(tPortBuffers && other) :
    queue_fragment(),
    port(NULL)
  {
    std::swap(queue_fragment, other.queue_fragment);
  }

  /*! Move assignment */
  tPortBuffers& operator=(tPortBuffers && other)
  {
    std::swap(queue_fragment, other.queue_fragment);
    return *this;
  }

  /*!
   * \return True, if there are no elements (left) in this fragment.
   */
  bool Empty()
  {
    return queue_fragment.Empty();
  }

  /*!
   * Returns and removes the element from queue fragment that was enqueued first
   * (the first call possibly involves reverting the element order => a little overhead)
   *
   * \return Element that was removed
   */
  T PopFront()
  {
    return tImplementation::ToDesiredType(queue_fragment.PopFront(), *port);
  }

  /*!
   * Returns and removes the element from queue fragment that was enqueued last
   * (the first call possibly involves reverting the element order => a little overhead)
   *
   * \return Element that was removed
   */
  T PopBack()
  {
    return tImplementation::ToDesiredType(queue_fragment.PopBack(), *port);
  }

  /*!
   * Returns and removes an element from the queue fragment
   *
   * \return Element that was removed
   */
  T PopAny()
  {
    return tImplementation::ToDesiredType(queue_fragment.PopAny(), *port);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Wrapped queue fragment */
  tQueueFragment queue_fragment;

  /*! Port that values were dequeued from */
  tPortBase* port;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

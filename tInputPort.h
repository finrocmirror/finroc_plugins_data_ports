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
/*!\file    plugins/data_ports/tInputPort.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-25
 *
 * \brief   Contains tInputPort
 *
 * \b tInputPort
 *
 * This port class is used in applications.
 * It provides a convenient API for the type-less port implementation classes.
 * Derived from tPort, this class provides some additional functions that
 * are only relevant for input ports.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tInputPort_h__
#define __plugins__data_ports__tInputPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tPort.h"
#include "plugins/data_ports/api/tPortImplementationTypeTrait.h"

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

template <typename LISTENER, typename T, tPortImplementationType TPortImplementationType, bool FIRST_LISTENER>
class tPortListenerAdapter;

template <typename LISTENER, typename T, bool FIRST_LISTENER>
class tPortListenerAdapterForPointer;

}

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Input port
/*!
 * This port class is used in applications.
 * It provides a convenient API for the type-less port implementation classes.
 * Derived from tPort, this class provides some additional functions that
 * are only relevant for input ports.
 *
 * \tparam T T is the data type of the port (see tPort)
 */
template <typename T>
class tInputPort : public tPort<T>
{
  typedef typename tPort<T>::tImplementation tImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Creates no wrapped port */
  tInputPort() {}

  /*!
   * Constructor takes variadic argument list... just any properties you want to assign to port.
   *
   * The first string is interpreted as port name, the second possibly as config entry (relevant for parameters only).
   * A framework element pointer is interpreted as parent.
   * tFrameworkElement::tFlags arguments are interpreted as flags.
   * int argument is interpreted as queue length.
   * tBounds<T> are port's bounds.
   * tUnit argument is port's unit.
   * int16/short argument is interpreted as minimum network update interval.
   * const T& is interpreted as port's default value.
   * tPortCreationInfo<T> argument is copied. This is only allowed as first argument.
   *
   * This becomes a little tricky when port has numeric or string type.
   * There we have these rules:
   *
   * string type: The second string argument is interpreted as default_value. The third as config entry.
   * numeric type: The first numeric argument is interpreted as default_value.
   */
  template <typename ... ARGS>
  tInputPort(const ARGS& ... args) :
    tPort<T>(args..., core::tFrameworkElement::tFlag::ACCEPTS_DATA | core::tFrameworkElement::tFlag::PUSH_STRATEGY)
  {}

  /*!
   * \param listener Listener to add
   *
   * \tparam LISTENER Listener class needs to implement a method
   * void PortChanged(tInputPort<T>& port, const T& value, const rrlib::time::tTimestamp& timestamp)
   *
   * (It's preferred to add listeners before port is initialized)
   */
  template <typename TListener>
  void AddPortListener(TListener& listener);

  /*!
   * \param listener Listener to add
   *
   * \tparam LISTENER Listener class needs to implement a method
   * void PortChanged(tInputPort<T>& port, tPortDataPointer<const T>& value, const rrlib::time::tTimestamp& timestamp)
   *
   * (It's preferred to add listeners before port is initialized)
   */
  template <typename TListener>
  void AddPortListenerForPointer(TListener& listener);

  /*!
   * \param listener Listener to add
   *
   * \tparam LISTENER Listener class needs to implement a method
   * void PortChanged(common::tAbstractDataPort& origin)
   *
   * (It's preferred to add listeners before port is initialized)
   */
  template <typename TListener>
  void AddPortListenerSimple(TListener& listener);

  /*!
   * Dequeue first/oldest element in queue.
   * Because queue is bounded, continuous dequeueing may skip some values.
   * Use DequeueAll if a continuous set of values is required.
   *
   * (Use only with ports that have an appropriate input queue)
   *
   * \return Dequeued first/oldest element in queue (NULL if no element is left in queue)
   */
  inline tPortDataPointer<const T> Dequeue()
  {
    auto buffer_pointer = this->GetWrapped()->DequeueSingleRaw();
    if (buffer_pointer)
    {
      return tPortDataPointer<const T>(buffer_pointer, *this->GetWrapped());
    }
    return tPortDataPointer<const T>();
  }

  /*!
   * Dequeue first/oldest element in queue.
   * Because queue is bounded, continuous dequeueing may skip some values.
   * Use DequeueAll if a continuous set of values is required.
   *
   * (Use only with ports that have a appropriate input queue)
   * (only available for 'cheaply copied' types)
   *
   * \param result Buffer to (deep) copy dequeued value to
   * \param timestamp Buffer to store time stamp of data in (optional)
   * \return true if element was dequeued - false if queue was empty
   */
  template <bool AVAILABLE = tPort<T>::cPASS_BY_VALUE>
  inline typename std::enable_if<AVAILABLE, bool>::type Dequeue(T& result)
  {
    rrlib::time::tTimestamp unused;
    return Dequeue(result, unused);
  }

  template <bool AVAILABLE = tPort<T>::cPASS_BY_VALUE>
  inline typename std::enable_if<AVAILABLE, bool>::type Dequeue(T& result, rrlib::time::tTimestamp& timestamp)
  {
    typename tImplementation::tLockingManagerPointer buffer = this->GetWrapped()->DequeueSingleRaw();
    if (buffer)
    {
      result = tImplementation::ToValue(buffer->GetObject().template GetData<tPort<T>::tPortBuffer>(), this->GetWrapped()->GetUnit());
    }
    return buffer;
  }

  /*!
   * Dequeue all elements currently in input queue
   * (The variant that returns buffers by-value is only available for 'cheaply copied' types.)
   *
   * \return Set of dequeued buffers.
   */
  template <bool AVAILABLE = tPort<T>::cPASS_BY_VALUE>
  inline typename std::enable_if<AVAILABLE, tPortBuffers<T>>::type DequeueAll()
  {
    return tPortBuffers<T>(this->GetWrapped()->DequeueAllRaw(), *this->GetWrapped());
  }

  inline tPortBuffers<tPortDataPointer<const T>> DequeueAllBuffers()
  {
    return tPortBuffers<tPortDataPointer<const T>>(this->GetWrapped()->DequeueAllRaw(), *this->GetWrapped());
  }

  /*!
   * (relevant for input ports only)
   *
   * \return Has port changed since last changed-flag-reset?
   */
  inline bool HasChanged() const
  {
    return this->GetWrapped()->HasChanged();
  }

  /*!
   * Is data to this port pushed or pulled?
   *
   * \return Answer
   */
  inline bool PushStrategy() const
  {
    return this->GetWrapped()->PushStrategy();
  }

//  /*!
//   * \param listener Listener to remove
//   */
//  void RemovePortListener(tPortListener<tPortDataPointer<const T>>& listener)
//  {
//    this->GetWrapped()->RemovePortListenerRaw(listener);
//  }
//  void RemovePortListener(tPortListener<T>& listener)
//  {
//    this->GetWrapped()->RemovePortListenerRaw(listener);
//  }
//  void RemovePortListener(tPortListener<const void*>& listener)
//  {
//    this->GetWrapped()->RemovePortListenerRaw(listener);
//  }

  /*!
   * (relevant for input ports only)
   *
   * Reset changed flag.
   */
  inline void ResetChanged()
  {
    this->GetWrapped()->ResetChanged();
  }

  /*!
   * Set whether data should be pushed or pulled
   *
   * \param push Push data?
   */
  inline void SetPushStrategy(bool push)
  {
    this->GetWrapped()->SetPushStrategy(push);
  }

  /*!
   * Wraps raw port
   * Throws std::runtime_error if port to wrap has invalid flags.
   *
   * \param wrap Type-less port to wrap as tPort<T>
   */
  static tInputPort Wrap(core::tAbstractPort& wrap)
  {
    tPort<T>::Wrap(wrap); // checks types
    if (!(wrap.GetFlag(core::tFrameworkElement::tFlag::ACCEPTS_DATA)))
    {
      std::runtime_error("tInputPort can only wrap ports that accept data.");
    }
    tInputPort port;
    port.SetWrapped(&wrap);
    return port;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  template <typename LISTENER, typename U, api::tPortImplementationType TPortImplementationType, bool FIRST_LISTENER>
  friend class api::tPortListenerAdapter;

  template <typename LISTENER, typename U, bool FIRST_LISTENER>
  friend class api::tPortListenerAdapterForPointer;


};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

#include "plugins/data_ports/tInputPort.hpp"

#endif

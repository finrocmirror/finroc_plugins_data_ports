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
/*!\file    plugins/data_ports/tPort.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-21
 *
 * \brief   Contains tPort
 *
 * \b tPort
 *
 * This port class is used in applications.
 * It provides a convenient API for the type-less tPortBase backend.
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tPort_h__
#define __plugins__data_ports__tPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tPortCreationInfo.h"
#include "plugins/data_ports/tPortListener.h"
#include "plugins/data_ports/tPortWrapperBase.h"
#include "plugins/data_ports/api/tPortImplementation.h"
#include "plugins/data_ports/tPortBuffers.h"

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
//! Data port
/*!
 * This port class is used in applications.
 * It provides a convenient API for the type-less tPortBase backend.
 *
 * \tparam T T is the data type of the port.
 *           Any data type that is binary serializable by rrlib_serialization
 *           can be used in Finroc data ports - even non-copyable ones.
 *           Get and dequeue operations will return T. Publish operations accept T.
 *           TODO ?: Depending on the intended port use, T can be a plain, reference or smart pointer type:
 *           1) If T is a plain type, Get() will return the port's current value  by value.
 *              This is - naturally - not suitable for non-copyable types.
 *           TODO ?: 2) If T is a reference type, Get() will return a const reference to the port's current value buffer.
 *              This is suitable and efficient for all kinds of data types.
 *              Data buffers are locked (and can be accessed safely) as long as inside the relevant
 *              auto-lock scope (created by instantiating tAutoLockScope)
 *              Such scopes are automatically created by Finroc for Update()/Sense()/Control() methods of modules.
 *           TODO ?: 3) If T is tPortDataPtr<U>, the port returns a tPortDataPtr smart pointer -
 *              used similarly as a standard unique pointer (movable, unique).
 *              On smart pointer destruction, the lock on the buffer is released.
 *              Smart pointers do not support numeric types (due to conversion overhead).
 */
template<typename T>
class tPort : public tPortWrapperBase
{
  static_assert(rrlib::serialization::tIsBinarySerializable<T>::value, "Type T needs to be binary serializable for use in ports.");

  /*! Class that contains actual implementation of most functionality */
  typedef api::tPortImplementation<T, api::tPortImplementationTypeTrait<T>::type> tImplementation;

  /*! Typeless port class used in backend */
  typedef typename tImplementation::tPortBase tPortBackend;

  /*! Port buffers used in backend */
  typedef typename tImplementation::tPortBuffer tPortBuffer;

  /*! Should methods passing buffers by-value be available? */
  enum { cPASS_BY_VALUE = tIsCheaplyCopiedType<T>::value };

  /*! Should methods dealing with bounds be available? */
  enum { cBOUNDABLE = tIsBoundable<T>::value };

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Smart pointer class returned by various methods
   */
  typedef tPortDataPointer<T> tDataPointer;

  //typedef typename tPortTypeMap<T>::tPortBackend tPortBackend;

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
  tPort(const ARGS&... args)
  {
    tPortCreationInfo<T> creation_info(args...);
    creation_info.data_type = rrlib::rtti::tDataType<T>();
    SetWrapped(tImplementation::CreatePort(creation_info));
    GetWrapped()->SetWrapperDataType(rrlib::rtti::tDataType<T>());
    if (creation_info.DefaultValueSet())
    {
      T t = rrlib::rtti::sStaticTypeInfo<T>::CreateByValue();
      creation_info.GetDefault(t);
      SetDefault(t);
    }
  }

  /*!
   * Wraps raw port
   */
  tPort(core::tAbstractPort& wrap)
  {
    if (wrap.GetDataType().GetRttiName() != typeid(tPortBuffer).name())
    {
      FINROC_LOG_PRINT(ERROR, "tPort<", rrlib::rtti::Demangle(typeid(T).name()), "> cannot wrap port with buffer type '", wrap.GetDataType().GetName(), "'.");
      return;
    }
    SetWrapped(&wrap);
  }

  /*!
   * \param listener Listener to add
   */
  void AddPortListener(tPortListener<tPortDataPointer<const T>>& listener)
  {
    GetWrapped()->AddPortListenerRaw(listener);
  }
  void AddPortListener(tPortListener<const void*>& listener)
  {
    GetWrapped()->AddPortListenerRaw(listener);
  }
  inline void AddPortListener(tPortListener<T>& listener)
  {
    GetWrapped()->AddPortListenerRaw(listener);
  }

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
    auto buffer_pointer = GetWrapped()->DequeueSingleRaw();
    return tPortDataPointer<const T>(buffer_pointer, *GetWrapped());
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
  template <bool AVAILABLE = cPASS_BY_VALUE>
  inline typename std::enable_if<AVAILABLE, bool>::type Dequeue(T& result)
  {
    rrlib::time::tTimestamp unused;
    return Dequeue(result, unused);
  }

  template <bool AVAILABLE = cPASS_BY_VALUE>
  inline typename std::enable_if<AVAILABLE, bool>::type Dequeue(T& result, rrlib::time::tTimestamp& timestamp)
  {
    typename tImplementation::tLockingManagerPointer buffer = GetWrapped()->DequeueSingleRaw();
    if (buffer)
    {
      result = tImplementation::ToValue(buffer->GetObject().template GetData<tPortBuffer>(), GetWrapped()->GetUnit());
    }
    return buffer;
  }

  /*!
   * Dequeue all elements currently in input queue
   * (The variant that returns buffers by-value is only available for 'cheaply copied' types.)
   *
   * \return Set of dequeued buffers.
   */
  template <bool AVAILABLE = cPASS_BY_VALUE>
  inline typename std::enable_if<AVAILABLE, tPortBuffers<T>>::type DequeueAll()
  {
    return tPortBuffers<T>(GetWrapped()->DequeueAllRaw(), *GetWrapped());
  }

  inline tPortBuffers<tPortDataPointer<const T>> DequeueAllBuffers()
  {
    return tPortBuffers<tPortDataPointer<const T>>(GetWrapped()->DequeueAllRaw(), *GetWrapped());
  }

  /*!
   * Gets Port's current value.
   * (only available for 'cheaply copied' types)
   *
   * \param v unused dummy parameter for std::enable_if technique
   * \param timestamp Buffer to store time stamp of data in (optional)
   * \return Port's current value by value.
   */
  template <bool AVAILABLE = cPASS_BY_VALUE>
  inline T Get(typename std::enable_if<AVAILABLE, void>::type* v = NULL) const
  {
    tPortBuffer t;
    GetWrapped()->CopyCurrentValue(t);
    return tImplementation::ToValue(t, GetWrapped()->GetUnit());
  }

  template <bool AVAILABLE = cPASS_BY_VALUE>
  inline T Get(typename std::enable_if<AVAILABLE, rrlib::time::tTimestamp&>::type& timestamp) const
  {
    tPortBuffer t;
    GetWrapped()->CopyCurrentValue(t, timestamp);
    return tImplementation::ToValue(t, GetWrapped()->GetUnit());
  }

  /*!
   * Gets Port's current value
   *
   * (Using this Get()-variant is efficient when using 'cheaply copied' types,
   * but can be extremely costly with large data types)
   *
   * \param result Buffer to (deep) copy port's current value to
   * \param timestamp Buffer to store time stamp of data in (optional)
   */
  inline void Get(T& result) const
  {
    rrlib::time::tTimestamp unused;
    Get(result, unused);
  }
  inline void Get(T& result, rrlib::time::tTimestamp& timestamp) const
  {
    tImplementation::CopyCurrentPortValue(*GetWrapped(), result, timestamp);
  }

  /*!
   * (throws a std::runtime_error if port is not bounded)
   *
   * \return Bounds as they are currently set
   */
  template < bool AVAILABLE = cBOUNDABLE >
  inline typename std::enable_if<AVAILABLE, tBounds<T>>::type GetBounds() const
  {
    typedef api::tBoundedPort<T, api::tPortImplementationTypeTrait<T>::type> tBoundedPort;
    if ((!GetWrapped()) || typeid(*GetWrapped()) != typeid(tBoundedPort))
    {
      throw std::runtime_error("This is not a bounded port");
    }
    return static_cast<tBoundedPort*>(GetWrapped())->GetBounds();
  }

//  /*!
//   * \return Buffer with default value. Can be used to change default value
//   * for port. However, this should be done before the port is used.
//   */
//  inline T* GetDefaultBuffer()
//  {
//    rrlib::rtti::tGenericObject* go = static_cast<tPortBackend*>(wrapped)->GetDefaultBufferRaw();
//    return go->GetData<T>();
//  }

  /*!
   * Gets Port's current value
   *
   * \return Port's current value with read lock.
   * (in Java lock will need to be released manually, in C++ tPortDataPtr takes care of this)
   * (Using get with parameter T& is more efficient when using CC types - shouldn't matter usually)
   */
  inline tPortDataPointer<const T> GetPointer() const
  {
    return tImplementation::GetPointer(*GetWrapped());
  }

  /*!
   * Pulls port data (regardless of strategy)
   * (careful: no auto-release of lock in Java)
   *
   * \param intermediate_assign Assign pulled value to ports in between?
   *
   * \return Pulled locked data
   */
  inline tPortDataPointer<const T> GetPull(bool intermediate_assign)
  {
    auto buffer_pointer = GetWrapped()->GetPullRaw(intermediate_assign, false);
    return tPortDataPointer<const T>(buffer_pointer, *GetWrapped());
  }

  /*!
   * \return Unused buffer of type T.
   * Buffers to be published using this port (non-CC-types),
   * should be acquired using this function. The buffer might contain old data, so it should
   * be cleared prior to using. Using this method with CC-types is not required and less
   * efficient than publishing values directly (factor 2, shouldn't matter usually).
   */
  inline tPortDataPointer<T> GetUnusedBuffer()
  {
    return tImplementation::GetUnusedBuffer(*GetWrapped());
  }

  /*!
   * \return Wrapped port. For rare case that someone really needs to access ports.
   */
  inline tPortBackend* GetWrapped() const
  {
    return static_cast<tPortBackend*>(tPortWrapperBase::GetWrapped());
  }

  /*!
   * (relevant for input ports only)
   *
   * \return Has port changed since last changed-flag-reset?
   */
  inline bool HasChanged() const
  {
    return GetWrapped()->HasChanged();
  }

  /*!
   * \return Does port have "cheaply copied" type?
   */
  inline bool HasCheaplyCopiedType() const
  {
    return tIsCheaplyCopiedType<T>::value;
  }

  /*!
   * Publish Data Buffer. This data will be forwarded to any connected ports.
   * Should only be called on output ports.
   *
   * (This pass-by-value Publish()-variant is efficient when using 'cheaply copied' types,
   *  but can be computationally expensive with large data types)
   *
   * \param data Data to publish. It will be deep-copied.
   * \param teimstamp Timestamp for attached data (optional)
   */
  inline void Publish(const T& data, const rrlib::time::tTimestamp& timestamp = rrlib::time::cNO_TIME)
  {
    tImplementation::CopyAndPublish(*GetWrapped(), data, timestamp);
  }

  /*!
   * Publish Data Buffer. This data will be forwarded to any connected ports.
   * It should not be modified thereafter (tPortDataPointer will be reset).
   * Should only be called on output ports.
   *
   * \param data Data buffer acquired from a port using getUnusedBuffer (or locked data received from another port)
   */
  inline void Publish(tPortDataPointer<T> && data)
  {
    tImplementation::Publish(*GetWrapped(), std::forward<tPortDataPointer<T>>(data));
  }
  inline void Publish(tPortDataPointer<const T> && data)
  {
    tImplementation::PublishConstBuffer(*GetWrapped(), std::forward<tPortDataPointer<const T>>(data));
  }

  /*!
   * Is data to this port pushed or pulled?
   *
   * \return Answer
   */
  inline bool PushStrategy() const
  {
    return GetWrapped()->PushStrategy();
  }

  /*!
   * \param listener Listener to remove
   */
  void RemovePortListener(tPortListener<tPortDataPointer<const T>>& listener)
  {
    GetWrapped()->RemovePortListenerRaw(listener);
  }
  inline void RemovePortListener(tPortListener<T>& listener)
  {
    GetWrapped()->RemovePortListenerRaw(listener);
  }
  inline void RemovePortListener(tPortListener<const void*>& listener)
  {
    GetWrapped()->RemovePortListenerRaw(listener);
  }

  /*!
   * (relevant for input ports only)
   *
   * Reset changed flag.
   */
  inline void ResetChanged()
  {
    GetWrapped()->ResetChanged();
  }

  /*!
   * Is data to this port pushed in reverse direction?
   *
   * \return Answer
   */
  inline bool ReversePushStrategy() const
  {
    return GetWrapped()->ReversePushStrategy();
  }

  /*!
   * Set new bounds
   * (This is not thread-safe and must only be done in "pause mode")
   *
   * \param new_bounds New Bounds
   */
  template < bool AVAILABLE = cBOUNDABLE >
  inline void SetBounds(const typename std::enable_if<AVAILABLE, tBounds<T>>::type& new_bounds)
  {
    typedef api::tBoundedPort<T, api::tPortImplementationTypeTrait<T>::type> tBoundedPort;
    if ((!GetWrapped()) || typeid(*GetWrapped()) != typeid(tBoundedPort))
    {
      throw std::runtime_error("This is not a bounded port");
    }
    return static_cast<tBoundedPort*>(GetWrapped())->SetBounds(new_bounds);
  }

  /*!
   * Set default value
   * This must be done before the port is used/initialized.
   *
   * \param new_default new default
   */
  void SetDefault(const T& new_default)
  {
    tImplementation::SetDefault(*GetWrapped(), new_default);
  }

  /*!
   * \param interval Minimum Network Update Interval
   */
  inline void SetMinNetUpdateInterval(rrlib::time::tDuration new_interval)
  {
    GetWrapped()->SetMinNetUpdateInterval(new_interval);
  }

  /*!
   * Set whether data should be pushed or pulled
   *
   * \param push Push data?
   */
  inline void SetPushStrategy(bool push)
  {
    GetWrapped()->SetPushStrategy(push);
  }

  /*!
   * Set whether data should be pushed or pulled in reverse direction
   *
   * \param push Push data?
   */
  inline void SetReversePushStrategy(bool push)
  {
    GetWrapped()->SetReversePushStrategy(push);
  }

//  /*!
//   * Set default value
//   * This must be done before the port is used/initialized.
//   *
//   * \param source Source from which default value is deserialized
//   */
//  void SetDefault(const rrlib::serialization::tConstSource& source)
//  {
//    assert(!this->IsReady() && "please set default value _before_ initializing port");
//    rrlib::serialization::tInputStream is(&source);
//    tPortUtil<T>::SetDefault(static_cast<tPortBackend*>(wrapped), is);
//  }

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*!
   * (Constructor for derived classes)
   * (wrapped must be set in constructor!)
   */
  tPort() {}

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

};

extern template class tPort<int>;
extern template class tPort<long long int>;
extern template class tPort<float>;
extern template class tPort<double>;
extern template class tPort<numeric::tNumber>;
extern template class tPort<std::string>;
extern template class tPort<bool>;
extern template class tPort<rrlib::serialization::tMemoryBuffer>;

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

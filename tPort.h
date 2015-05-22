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
 * It provides a convenient API for the type-less port implementation classes.
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tPort_h__
#define __plugins__data_ports__tPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/port/tPortWrapperBase.h"

#include "rrlib/util/demangle.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tPortCreationInfo.h"
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
 * It provides a convenient API for the type-less port implementation classes.
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
class tPort : public core::tPortWrapperBase
{
protected:

  static_assert(rrlib::serialization::IsBinarySerializable<T>::value, "Type T needs to be binary serializable for use in ports.");

  /*! Class that contains actual implementation of most functionality */
  typedef api::tPortImplementation<T, api::tPortImplementationTypeTrait<T>::type> tImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Typeless port class used in backend */
  typedef typename tImplementation::tPortBase tPortBackend;

  /*! Type T */
  typedef T tDataType;

  /*! Port buffers used in backend */
  typedef typename tImplementation::tPortBuffer tPortBuffer;

  /*! Should methods passing buffers by-value be available? */
  enum { cPASS_BY_VALUE = tIsCheaplyCopiedType<T>::value };

  /*! Should methods dealing with bounds be available? */
  enum { cBOUNDABLE = IsBoundable<T>::value };

  /*! Smart pointer class returned by various methods */
  typedef tPortDataPointer<T> tDataPointer;

  /*! Bundles all possible constructor parameters of tPort */
  typedef tPortCreationInfo<T> tConstructorParameters;


  /*!
   * Creates no wrapped port
   */
  tPort() {}

  /*!
   * Constructor takes variadic argument list... just any properties you want to assign to port.
   *
   * The first string is interpreted as port name, the second possibly as config entry (relevant for parameters only).
   * A framework element pointer is interpreted as parent.
   * tFrameworkElement::tFlags arguments are interpreted as flags.
   * A tQueueSettings argument creates an input queue with the specified settings.
   * tBounds<T> are port's bounds.
   * tUnit argument is port's unit.
   * const T& is interpreted as port's default value.
   * tPortCreationInfo<T> argument is copied.
   *
   * This becomes a little tricky when T is a string type. There we have these rules:
   * The second string argument is interpreted as default_value. The third as config entry.
   */
  template <typename TArg1, typename TArg2, typename ... TRest>
  tPort(const TArg1& arg1, const TArg2& arg2, const TRest&... args)
  {
    tConstructorArguments<tPortCreationInfo<T>> creation_info(arg1, arg2, args...);
    creation_info.data_type = rrlib::rtti::tDataType<tPortBuffer>();
    if (!(creation_info.flags.Raw() & core::tFrameworkElementFlags(core::tFrameworkElementFlag::DELETED).Raw())) // do not create port, if deleted flag is set
    {
      SetWrapped(tImplementation::CreatePort(creation_info));
      GetWrapped()->SetWrapperDataType(rrlib::rtti::tDataType<T>());
      if (creation_info.DefaultValueSet())
      {
        T t(rrlib::serialization::DefaultInstantiation<T>::Create());
        creation_info.GetDefault(t);
        SetDefault(t);
      }
    }
  }

  // with a single argument, we do not want catch calls for copy construction
  template < typename TArgument1, bool ENABLE = !std::is_base_of<tPort, TArgument1>::value >
  tPort(const TArgument1& argument1, typename std::enable_if<ENABLE, tNoArgument>::type no_argument = tNoArgument())
  {
    // Call the above constructor
    *this = tPort(tFlags(), argument1);
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
#ifndef RRLIB_SINGLE_THREADED
    tPortBuffer t;
    GetWrapped()->CopyCurrentValue(t);
    return tImplementation::ToValue(t, GetWrapped()->GetUnit());
#else
    return GetWrapped()->CurrentValue();
#endif
  }

  template <bool AVAILABLE = cPASS_BY_VALUE>
  inline T Get(typename std::enable_if<AVAILABLE, rrlib::time::tTimestamp&>::type& timestamp) const
  {
#ifndef RRLIB_SINGLE_THREADED
    tPortBuffer t;
    GetWrapped()->CopyCurrentValue(t, timestamp);
    return tImplementation::ToValue(t, GetWrapped()->GetUnit());
#else
    timestamp = GetWrapped()->CurrentTimestamp();
    return GetWrapped()->CurrentValue();
#endif
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
  template <bool AVAILABLE = cBOUNDABLE>
  inline typename std::enable_if<AVAILABLE, tBounds<T>>::type GetBounds() const
  {
    typedef api::tBoundedPort<T, api::tPortImplementationTypeTrait<T>::type> tBoundedPort;
    if ((!GetWrapped()) || typeid(*GetWrapped()) != typeid(tBoundedPort))
    {
      throw std::runtime_error("This is not a bounded port");
    }
    return static_cast<tBoundedPort*>(GetWrapped())->GetBounds();
  }

//  /*
//   * \return Buffer with default value. Can be used to change default value
//   * for port. However, this should be done before the port is used.
//   */
//  inline T* GetDefaultBuffer()
//  {
//    rrlib::rtti::tGenericObject* go = static_cast<tPortBackend*>(wrapped)->GetDefaultBufferRaw();
//    return go->GetData<T>();
//  }

  /*!
   * Gets Port's current value in buffer
   *
   * \return Buffer with port's current value with read lock.
   */
  inline tPortDataPointer<const T> GetPointer() const
  {
    return tImplementation::GetPointer(*GetWrapped());
  }

  /*!
   * \return Wrapped port. For rare case that someone really needs to access ports.
   */
  inline tPortBackend* GetWrapped() const
  {
    return static_cast<tPortBackend*>(tPortWrapperBase::GetWrapped());
  }

  /*!
   * \return Does port have "cheaply copied" type?
   */
  inline bool HasCheaplyCopiedType() const
  {
    return tIsCheaplyCopiedType<T>::value;
  }

  /*!
   * Set new bounds
   * (This is not thread-safe and must only be done in "pause mode")
   *
   * \param new_bounds New Bounds
   */
  template <bool AVAILABLE = cBOUNDABLE>
  inline void SetBounds(const typename std::enable_if<AVAILABLE, tBounds<T>>::type& new_bounds)
  {
    typedef api::tBoundedPort<T, api::tPortImplementationTypeTrait<T>::type> tBoundedPort;
    if ((!GetWrapped()) || typeid(*GetWrapped()) != typeid(tBoundedPort))
    {
      throw std::runtime_error("This is not a bounded port");
    }
    static_cast<tBoundedPort*>(GetWrapped())->SetBounds(new_bounds);
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

//  /*
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

  /*!
   * Wraps raw port
   * Throws std::runtime_error if port to wrap has invalid type.
   *
   * \param wrap Type-less port to wrap as tPort<T>
   */
  static tPort Wrap(core::tAbstractPort& wrap)
  {
    if (wrap.GetDataType().GetRttiName() != typeid(typename rrlib::rtti::NormalizedType<tPortBuffer>::type).name())
    {
      //FINROC_LOG_PRINT(ERROR, "tPort<", rrlib::rtti::Demangle(typeid(T).name()), "> cannot wrap port with buffer type '", wrap.GetDataType().GetName(), "'.");
      throw std::runtime_error("tPort<" + rrlib::util::Demangle(typeid(T).name()) + "> cannot wrap port with buffer type '" + wrap.GetDataType().GetName() + "'.");
    }
    tPort port;
    port.SetWrapped(&wrap);
    return port;
  }

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

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
/*!\file    plugins/data_ports/tPortCreationInfo.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 * \brief   Contains tPortCreationInfo
 *
 * \b tPortCreationInfo
 *
 * This class contains various information for the creation of ports.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tPortCreationInfo_h__
#define __plugins__data_ports__tPortCreationInfo_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/port/tPortWrapperBase.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tAbstractDataPortCreationInfo.h"

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
//! Bundle of port creation parameters
/*!
 * This class bundles various parameters for the creation of data ports.
 *
 * Instead of providing suitable constructors for all types of sensible
 * combinations of the numerous (often optional) construction parameters,
 * there is only one constructor taking a single argument of this class.
 */
template <typename T>
class tPortCreationInfo : public common::tAbstractDataPortCreationInfo
{
  enum { cBOUNDABLE = tIsBoundable<T>::value };

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tPortCreationInfo() :
    common::tAbstractDataPortCreationInfo()
  {
  }

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
  template <typename ARG1, typename ... TArgs>
  explicit tPortCreationInfo(const ARG1& arg1, const TArgs&... rest) :
    common::tAbstractDataPortCreationInfo()
  {
    ProcessFirstArg<ARG1>(arg1);
    ProcessArgs(rest...);
  }

  /*!
   * \return Bounds for port
   */
  template < bool AVAILABLE = cBOUNDABLE >
  typename std::enable_if<AVAILABLE, tBounds<T>>::type GetBounds() const
  {
    T t = T();
    tBounds<T> result(t, t);
    if (!BoundsSet())
    {
      FINROC_LOG_PRINT_STATIC(DEBUG_WARNING, "Bounds were not set");
      return result;
    }
    rrlib::serialization::tInputStream is(bounds);
    is >> result;
    return result;
  }

  /*!
   * \return Default value
   */
  T GetDefault() const
  {
    T t;
    GetDefault(t);
    return t;
  }

  /*!
   * \param buffer Buffer to store result in
   */
  void GetDefault(T& buffer) const
  {
    if (!DefaultValueSet())
    {
      FINROC_LOG_PRINT_STATIC(DEBUG_WARNING, "Default value was not set");
      return;
    }
    rrlib::serialization::tInputStream is(default_value);
    is >> buffer;
  }

  using common::tAbstractDataPortCreationInfo::Set;

  template < bool DISABLE = (std::is_integral<T>::value && (!std::is_same<T, bool>::value) && sizeof(T) <= 4) || tIsString<T>::value >
  void Set(const typename std::enable_if < !DISABLE, T >::type& default_value)
  {
    SetDefault(default_value);
  }

  template < bool AVAILABLE = cBOUNDABLE >
  void Set(const typename std::enable_if<AVAILABLE, tBounds<T>>::type& bounds)
  {
    rrlib::serialization::tOutputStream os(this->bounds);
    os << bounds;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Process first constructor argument (tPortCreationInfo allowed) */
  template <typename A>
  void ProcessFirstArg(const typename std::enable_if<std::is_same<A, tPortCreationInfo>::value, A>::type& a)
  {
    *this = a;
  }
  template <typename A>
  void ProcessFirstArg(const typename std::enable_if<std::is_same<A, common::tAbstractDataPortCreationInfo>::value, A>::type& a)
  {
    static_cast<common::tAbstractDataPortCreationInfo&>(*this) = a;
  }

  template <typename A>
  void ProcessFirstArg(const typename std::enable_if < !(std::is_same<A, tPortCreationInfo>::value || std::is_same<A, common::tAbstractDataPortCreationInfo>::value), A >::type& a)
  {
    ProcessArg<A>(a);
  }

  /*! Process constructor arguments */
  void ProcessArgs() {}

  template <typename A, typename ... TRest>
  void ProcessArgs(const A& arg, const TRest&... args)
  {
    ProcessArg<A>(arg);
    ProcessArgs(args...);
  }

  /*! Process single argument */
  template <typename A>
  void ProcessArg(const typename std::enable_if < !(tIsString<A>::value || (tIsNumeric<T>::value && tIsNumeric<A>::value)), A >::type& arg)
  {
    // standard case
    Set(arg);
  }

  template <typename A>
  void ProcessArg(const typename std::enable_if<tIsString<A>::value, A>::type& arg)
  {
    // string argument, handling it here (no method overloading), produces some nicer compiler error messages
    SetString(arg);
  }

  template <typename A>
  void ProcessArg(const typename std::enable_if < tIsNumeric<T>::value && tIsNumeric<A>::value, A >::type& arg)
  {
    // numeric type and numeric argument => first numeric argument is default value
    if (!DefaultValueSet())
    {
      SetDefault((T)arg);
    }
    else
    {
      Set(arg);
    }
  }

  /*!
   * This exists so that the copy construction works/compiles with the varargs constructor.
   * At runtime, however, tPortWrapperBase::CopyConstruction should return true - and this is never called
   */
  void Set(const core::tPortWrapperBase& base)
  {
    throw std::logic_error("This should never be called");
  }

  // various helper methods
  template < bool STRING = tIsString<T>::value >
  void SetString(const typename std::enable_if < !STRING, tString >::type& s)
  {
    common::tAbstractDataPortCreationInfo::SetString(s);
  }

  template < bool STRING = tIsString<T>::value >
  void SetString(const typename std::enable_if<STRING, tString>::type& s)
  {
    if (!name_set)
    {
      name = s;
      name_set = true;
    }
    else if (!DefaultValueSet())
    {
      SetDefault(s);
    }
    else
    {
      config_entry = s;
    }
  }

  void SetDefault(const T& default_val)
  {
    if (DefaultValueSet())
    {
      FINROC_LOG_PRINT_STATIC(DEBUG_WARNING, "Default value already set");
    }
    rrlib::serialization::tOutputStream os(default_value);
    os << default_val;
  }
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

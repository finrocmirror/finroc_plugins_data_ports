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
  enum { cBOUNDABLE = IsBoundable<T>::value };

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Base class */
  typedef common::tAbstractDataPortCreationInfo tBase;

  /*!
   * \return Bounds for port
   */
  template <bool AVAILABLE = cBOUNDABLE>
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
    T t = T();
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

  /*! Various Set methods for different port properties */
  template <bool DISABLE = IsString<T>::value>
  void Set(const typename std::enable_if < !DISABLE, T >::type& default_value)
  {
    SetDefault(default_value);
  }

  template <bool AVAILABLE = cBOUNDABLE>
  void Set(const typename std::enable_if<AVAILABLE, tBounds<T>>::type& bounds)
  {
    rrlib::serialization::tOutputStream os(this->bounds);
    os << bounds;
  }

  void Set(const tPortCreationInfo& other)
  {
    *this = other;
  }

  void Set(const std::string& string)
  {
    SetString(string);
  }

  // we replicate this here, since Set() for default values catches pointers if T is bool
  void Set(core::tFrameworkElement* parent)
  {
    this->parent = parent;
  }
  void Set(const char* string)
  {
    SetString(string);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  template <bool STRING = IsString<T>::value>
  void SetString(const typename std::enable_if < !STRING, tString >::type& s)
  {
    common::tAbstractDataPortCreationInfo::SetString(s);
  }

  template <bool STRING = IsString<T>::value>
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

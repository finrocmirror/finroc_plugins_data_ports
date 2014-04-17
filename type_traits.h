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
/*!\file    plugins/data_ports/type_traits.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-21
 *
 * \brief
 *
 * Various type traits relevant for data ports.
 * May be specialized for certain types.
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__type_traits_h__
#define __plugins__data_ports__type_traits_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/port/tEdgeAggregator.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

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
namespace numeric
{
class tNumber;
}

//----------------------------------------------------------------------
// Function declarations
//----------------------------------------------------------------------

/*!
 * This type-trait is used to determine whether a type is a 'cheaply copied' type.
 *
 * In this case 'value' is true.
 *
 * Cheaply copied never block or allocate memory during copying.
 * As a rule of thumb, all types that could be copied by using memcpy and that are not too big (max. 256 bytes)
 * are 'cheaply copied' types.
 *
 * Interestingly, std::has_trivial_destructor is a pretty good heuristic whether a type is a 'cheaply copied' type.
 */
template <typename T>
struct tIsCheaplyCopiedType
{
  enum { value = std::is_trivially_destructible<T>::value && (sizeof(T) <= 256) };
};

/*!
 * Equivalent for runtime "cheaply copied" type identification
 */
inline bool IsCheaplyCopiedType(const rrlib::rtti::tType& dt)
{
  return dt.GetSize() <= 256 && ((dt.GetTypeTraits() & rrlib::rtti::trait_flags::cHAS_TRIVIAL_DESTRUCTOR) != 0);
}

/*!
 * \return True, if the provided type is a data flow type
 */
inline static bool IsDataFlowType(const rrlib::rtti::tType& type)
{
  return core::tEdgeAggregator::IsDataFlowType(type);
}

/*!
 * Type-trait for numeric types.
 * This includes all built-in numeric types, as well as all
 * types that can be implicitly casted to and from a built-in numeric type.
 * For the latter, this template needs to be specialized.
 */
template <typename T>
struct IsNumeric
{
  enum { value = std::is_integral<T>::value || std::is_floating_point<T>::value || std::is_same<T, numeric::tNumber>::value };
};
template <>
struct IsNumeric<bool>
{
  enum { value = 0 };
};
static_assert(!IsNumeric<bool>::value, "Bool should not be handled as numeric type");

/*!
 * This type-trait is used to determine whether a type supports operator '<' .
 */
template <typename T>
struct HasSmallerThanOperator
{
  template <typename U = T>
  static int16_t Test(decltype((*(U*)(NULL)) < (*(U*)(NULL))))
  {
    return 0;
  }

  static int32_t Test(...)
  {
    return 0;
  }

  enum { value = sizeof(Test(true)) == sizeof(int16_t) };
};

/*!
 * This type-trait determines whether a type is boundable in ports
 */
template <typename T>
struct IsBoundable
{
  enum { value = tIsCheaplyCopiedType<T>::value && HasSmallerThanOperator<T>::value && (!std::is_same<bool, T>::value) };
};

/*!
 * This type-trait determines whether a type is a string type
 */
template <typename T>
struct IsString
{
  enum { value = std::is_same<T, std::string>::value || std::is_same<T, tString>::value || std::is_same<T, char*>::value || std::is_same<T, const char*>::value || std::is_same<typename std::remove_extent<T>::type, char>::value };
};

template <size_t Tsize>
struct IsString<char(&) [Tsize]>
{
  enum { value = 1 };
};

template <size_t Tsize>
struct IsString<const char(&) [Tsize]>
{
  enum { value = 1 };
};

static_assert(IsString<char const(&) [9]>::value, "Error in trait implementation");

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

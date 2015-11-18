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
/*!\file    plugins/data_ports/numeric/tNumber.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-14
 *
 * \brief   Contains tNumber
 *
 * \b tNumber
 *
 * Type used in backend of all numeric ports.
 * Can store integer as well as floating point values.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__numeric__tNumber_h__
#define __plugins__data_ports__numeric__tNumber_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/serialization/serialization.h"

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
namespace numeric
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Numeric value
/*!
 * Type used in backend of all numeric ports.
 * Can store integer as well as floating point values.
 */
class tNumber
{
//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Type of value stored in union */
  enum class tType
  {
    INT64, FLOAT, DOUBLE
  };

  inline tNumber() :
    integer_value(0),
    number_type(tType::INT64)
  {}

  /*! constructor for all integral types. */
  template<typename T>
  inline tNumber(T value, typename std::enable_if<std::is_integral<T>::value, void*>::type unused = nullptr) :
    integer_value(value),
    number_type(tType::INT64)
  {}

  inline tNumber(double value) :
    double_value(value),
    number_type(tType::DOUBLE)
  {}

  inline tNumber(float value) :
    float_value(value),
    number_type(tType::FLOAT)
  {}

  /*!
   * \return What kind of value is stored in this object?
   */
  inline tType GetNumberType() const
  {
    return number_type;
  }

  // All kinds of variations of setters
  template<typename T>
  inline void SetValue(T value, typename std::enable_if<std::is_integral<T>::value, void*>::type unused = nullptr)
  {
    this->integer_value = value;
    number_type = tType::INT64;
  }

  inline void SetValue(float value)
  {
    this->float_value = value;
    number_type = tType::FLOAT;
  }

  inline void SetValue(double value)
  {
    this->double_value = value;
    number_type = tType::DOUBLE;
  }

  // returns raw numeric value
  template <typename T>
  T Value() const
  {
    switch (number_type)
    {
    case tType::INT64:
      return static_cast<T>(integer_value);
    case tType::DOUBLE:
      return static_cast<T>(double_value);
    case tType::FLOAT:
      return static_cast<T>(float_value);
    default:
      assert(false && "Not a tNumber at this memory address");
      return 0;
    }
  }

  bool operator==(const tNumber& other) const
  {
    return integer_value == other.integer_value && number_type == other.number_type;
  }

  bool operator!=(const tNumber& other) const
  {
    return !operator==(other);
  }

  bool operator<(const tNumber& other) const;

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tNumber& number);
  friend rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tNumber& number);

  /*! Current numeric value */
  union
  {
    int64_t integer_value;
    double double_value;
    float float_value;
  };

  /*!
   * Determines what kind of value is stored in this object
   * (and which value of the above union is used)
   */
  tType number_type;

};

rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tNumber& number);
rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tNumber& number);
rrlib::serialization::tStringOutputStream &operator << (rrlib::serialization::tStringOutputStream& stream, const tNumber& number);
rrlib::serialization::tStringInputStream &operator >> (rrlib::serialization::tStringInputStream& stream, tNumber& number);
std::ostream &operator << (std::ostream &stream, const tNumber& number);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

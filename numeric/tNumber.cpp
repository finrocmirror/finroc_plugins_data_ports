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
/*!\file    plugins/data_ports/numeric/tNumber.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-14
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/numeric/tNumber.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/rtti/rtti.h"
#include "rrlib/rtti_conversion/tStaticCastOperation.h"
#include "core/definitions.h"
#include "core/log_messages.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
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
// Const values
//----------------------------------------------------------------------

/*! Constants for value type encoding in serialization */
static const int8_t cINT64 = -64, cINT32 = -63, cINT16 = -62, cFLOAT64 = -61, cFLOAT32 = -60, cCONST = -59, cMIN_BARRIER = -58;

/*! Initializes tNumber data type */
static rrlib::rtti::tDataType<tNumber> cINIT_DATA_TYPE("Number");

auto& cNUMBER_TYPE_CASTS = rrlib::rtti::conversion::tStaticCastOperation::
                           Register<tNumber, int8_t, true>()
                           .Register<tNumber, int16_t, true>()
                           .Register<tNumber, int32_t, true>()
                           .Register<tNumber, int64_t, true>()
                           .Register<tNumber, uint8_t, true>()
                           .Register<tNumber, uint16_t, true>()
                           .Register<tNumber, uint32_t, true>()
                           .Register<tNumber, uint64_t, true>()
                           .Register<tNumber, float, true>()
                           .Register<tNumber, double, true>();

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------
static inline int8_t PrepareFirstByte(int8_t value2)
{
  return static_cast<int8_t>((value2 << 1));
}

bool tNumber::operator<(const tNumber& other) const
{
  switch (number_type)
  {
  case tType::INT64:
    return integer_value < other.Value<int64_t>();
  case tType::DOUBLE:
    return double_value < other.Value<double>();
  case tType::FLOAT:
    return float_value < other.Value<float>();
  default:
    FINROC_LOG_PRINT(ERROR, "Memory error: Invalid enum.");
    abort();
    return 0;
  }
}

rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tNumber& number)
{
  if (number.GetNumberType() == tNumber::tType::INT64)
  {
    int64_t value = number.integer_value;
    if (value >= cMIN_BARRIER && value <= 63)
    {
      stream.WriteByte(PrepareFirstByte(static_cast<int8_t>(value)));
    }
    else if (value >= std::numeric_limits<int16_t>::min() && value <= std::numeric_limits<int16_t>::max())
    {
      stream.WriteByte(PrepareFirstByte(cINT16));
      stream.WriteShort(static_cast<int16_t>(value));
    }
    else if (value >= std::numeric_limits<int32_t>::min() && value <= std::numeric_limits<int32_t>::max())
    {
      stream.WriteByte(PrepareFirstByte(cINT32));
      stream.WriteInt(static_cast<int32_t>(value));
    }
    else
    {
      stream.WriteByte(PrepareFirstByte(cINT64));
      stream.WriteLong(value);
    }
  }
  else if (number.GetNumberType() == tNumber::tType::DOUBLE)
  {
    stream.WriteByte(PrepareFirstByte(cFLOAT64));
    stream.WriteDouble(number.double_value);
  }
  else if (number.GetNumberType() == tNumber::tType::FLOAT)
  {
    stream.WriteByte(PrepareFirstByte(cFLOAT32));
    stream.WriteFloat(number.float_value);
  }
  return stream;
}

rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tNumber& number)
{
  int8_t first_byte = stream.ReadByte();
  bool has_unit = (first_byte & 1) > 0;
  switch (first_byte >> 1)
  {
  case cINT64:
    number.SetValue(stream.ReadLong());
    break;
  case cFLOAT64:
    number.SetValue(stream.ReadDouble());
    break;
  case cINT32:
    number.SetValue(stream.ReadInt());
    break;
  case cFLOAT32:
    number.SetValue(stream.ReadFloat());
    break;
  case cINT16:
    number.SetValue(stream.ReadShort());
    break;
  case cCONST:
    stream.ReadByte();
    FINROC_LOG_PRINT_STATIC(WARNING, "Constants no longer supported. Ignoring.");
    break;
  default:
    number.SetValue(static_cast<int>(first_byte) >> 1);
    break;
  }
  if (has_unit)
  {
    stream.ReadByte(); // for backward compatibility
  }
  return stream;
}

rrlib::serialization::tStringOutputStream &operator << (rrlib::serialization::tStringOutputStream& stream, const tNumber& number)
{
  switch (number.GetNumberType())
  {
  case tNumber::tType::INT64:
    stream << number.Value<int64_t>();
    break;
  case tNumber::tType::FLOAT:
    stream << number.Value<float>();
    break;
  case tNumber::tType::DOUBLE:
    stream << number.Value<double>();
    break;
  default:
    FINROC_LOG_PRINT_STATIC(ERROR, "Memory error: Invalid enum.");
    abort();
  }
  return stream;
}

// TODO: This implementation could be nicer
rrlib::serialization::tStringInputStream &operator >> (rrlib::serialization::tStringInputStream& stream, tNumber& number)
{
  // scan for unit string (for backward compatibility)
  tString complete_string = stream.ReadWhile("-./", rrlib::serialization::tStringInputStream::cDIGIT | rrlib::serialization::tStringInputStream::cWHITESPACE | rrlib::serialization::tStringInputStream::cLETTER, true);
  tString number_string = complete_string;
  for (size_t i = 0; i < complete_string.length(); i++)
  {
    char c = complete_string[i];
    if (isalpha(c))
    {
      if ((c == 'e' || c == 'E') && (complete_string.length() > i + 1) && (complete_string[i + 1] == '-' || isdigit(complete_string[i + 1])))
      {
        continue;  // exponent in decimal notation
      }
      number_string = complete_string.substr(0, i); // trimming not necessary, as ato* functions do this
      //tString unit_string = complete_string.substr(i); // first character is letter
      break;
    }
  }
  if (number_string.find('.') != std::string::npos || number_string.find('e') != std::string::npos || number_string.find('E') != std::string::npos)
  {
    double d = atof(number_string.c_str());
    number.SetValue(d);
    if (d == 0.0)
    {
      for (size_t i = 0; i < number_string.length(); i++)
      {
        if (number_string[i] != ' ' && number_string[i] != '.' && number_string[i] != '0')
        {
          throw std::runtime_error("Could not parse value '" + number_string + "'");
        }
      }
    }
  }
  else
  {
    int64_t d = atoll(number_string.c_str());
    number.SetValue(d);
    if (d == 0LL)
    {
      for (size_t i = 0; i < number_string.length(); i++)
      {
        if (number_string[i] != ' ' && number_string[i] != '.' && number_string[i] != '0')
        {
          throw std::runtime_error("Could not parse value '" + number_string + "'");
        }
      }
    }
  }
  return stream;
}

std::ostream &operator << (std::ostream &stream, const tNumber& number)
{
  rrlib::serialization::tStringOutputStream sos;
  sos << number;
  stream << sos.ToString();
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

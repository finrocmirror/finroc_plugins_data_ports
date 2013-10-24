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
/*!\file    plugins/data_ports/tUnit.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-01
 *
 * \brief   Contains tUnit
 *
 * \b tUnit
 *
 * Class for supporting measurement units (such as m, cm, mm for distance)
 * Constants for various measurement units are defined in
 * this class and can be used in ports.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tUnit_h__
#define __plugins__data_ports__tUnit_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <vector>
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

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Measurement unit
/*!
 * Class for supporting measurement units (such as m, cm, mm for distance)
 * Constants for various measurement units are defined in
 * this class and can be used in ports.
 */
class tUnit
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  const static tUnit cNO_UNIT;

  /*! distance units */
  const static tUnit nm, um, mm, cm, dm, m, km;

  /*! speed units */
  const static tUnit km_h, m_s;

  /*! weight units */
  const static tUnit mg, g, kg, t, mt;

  /*! time units */
  const static tUnit ns, us, ms, s, min, h, day;

  /*! angle units */
  const static tUnit deg, rad;

  /*! Hertz */
  const static tUnit Hz;

  /*! Pixel (display) */
  const static tUnit Pixel;

  tUnit()
  {
    *this = cNO_UNIT;
  }

  /*!
   * Is Unit convertible to other Unit?
   *
   * \param other_unit other Unit
   * \return True if it is convertible.
   */
  inline bool ConvertibleTo(const tUnit& other_unit) const
  {
    return other_unit.wrapped - other_unit.wrapped->index == wrapped - wrapped->index; // This is quite efficient :-)
  }

  /*!
   * Converts value from this unit to other unit.
   *
   * \param value Value
   * \param to_unit Other Unit
   * \return Result
   */
  double ConvertTo(double value, const tUnit& to_unit) const;

  /*!
   * Get conversion factor from this unit to other unit
   *
   * \param to_unit other Unit
   * \return Factor
   */
  double GetConversionFactor(const tUnit& to_unit) const;

  /*!
   * \return Unit name
   */
  const char* GetName() const
  {
    return wrapped->name;
  }

  /*!
   * \return Unit with specified name
   */
  static tUnit GetUnit(const std::string& name);

  bool operator==(const tUnit& other) const
  {
    return wrapped == other.wrapped;
  }

  bool operator!=(const tUnit& other) const
  {
    return wrapped != other.wrapped;
  }

//----------------------------------------------------------------------
// Public struct and constructor not meant to be used externally (required for initialization in .cpp file)
//----------------------------------------------------------------------

  struct tUnitData
  {
    /*! Unit name */
    const char* name;

    /*! index in unit group */
    int index;

    /*! Factor regarding base unit */
    double factor;

    /*! factors for conversion to other units in group */
    double* factors;
  };

  tUnit(const tUnitData& data) : wrapped(&data) {}

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tUnit& unit);
  friend rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tUnit& unit);

  /*! Wrapped unit data (separated for more efficient copying) */
  const tUnitData* wrapped;

};

rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tUnit& unit);
rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tUnit& unit);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

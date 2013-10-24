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
/*!\file    plugins/data_ports/tUnit.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-01
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/tUnit.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
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
namespace internal
{

static double single_unit_factor = 1;
static double distance_unit_factors[7][7];
static double speed_unit_factors[2][2];
static double weight_unit_factors[5][5];
static double time_unit_factors[7][7];
static double angle_unit_factors[2][2];

static const tUnit::tUnitData unit_data[] =
{
  { "", 0, 1, &single_unit_factor },

  { "nm", 0, 0.000000001, distance_unit_factors[0] },
  { "um", 1, 0.000001, distance_unit_factors[1] },
  { "mm", 2, 0.001, distance_unit_factors[2] },
  { "cm", 3, 0.01, distance_unit_factors[3] },
  { "dm", 4, 0.1, distance_unit_factors[4] },
  { "m", 5, 1, distance_unit_factors[5] },
  { "km", 6, 1000, distance_unit_factors[6] },

  { "km/h", 0, 3.6, speed_unit_factors[0] },
  { "m/s", 1, 1, speed_unit_factors[1] },

  { "mg", 0, 0.001, weight_unit_factors[0] },
  { "g", 1, 1, weight_unit_factors[1] },
  { "kg", 2, 1000, weight_unit_factors[2] },
  { "t", 3, 1000000, weight_unit_factors[3] },
  { "mt", 4, 1000000000000.0, weight_unit_factors[4] },

  { "ns", 0, 0.000000001, time_unit_factors[0] },
  { "us", 1, 0.000001, time_unit_factors[1] },
  { "ms", 2, 0.001, time_unit_factors[2] },
  { "s", 3, 1, time_unit_factors[3] },
  { "min", 4, 60, time_unit_factors[4] },
  { "h", 5, 3600, time_unit_factors[5] },
  { "day", 6, 86400, time_unit_factors[6] },

  { "deg", 0, 0.017453292, angle_unit_factors[0] },
  { "rad", 1, 1, angle_unit_factors[1] },

  { "Hz", 0, 1, &single_unit_factor },

  { "Pixel", 0, 1, &single_unit_factor }
};

enum { cUNIT_COUNT = sizeof(unit_data) / sizeof(tUnit::tUnitData) };

static int InitFactors()
{
  for (const tUnit::tUnitData* unit = unit_data; unit < unit_data + cUNIT_COUNT; ++unit)
  {
    if (unit->factors != &single_unit_factor)
    {
      int i = 0;
      for (const tUnit::tUnitData* other_unit = unit - unit->index; other_unit - other_unit->index == unit - unit->index; ++other_unit)
      {
        unit->factors[i] = other_unit->factor / unit->factor;
        i++;
      }
    }
  }
  return 0;
}

static __attribute__((unused)) const int cINIT_FACTORS = InitFactors();

} // namespace internal

const tUnit tUnit::cNO_UNIT(internal::unit_data[0]);
const tUnit tUnit::nm(internal::unit_data[1]);
const tUnit tUnit::um(internal::unit_data[2]);
const tUnit tUnit::mm(internal::unit_data[3]);
const tUnit tUnit::cm(internal::unit_data[4]);
const tUnit tUnit::dm(internal::unit_data[5]);
const tUnit tUnit::m(internal::unit_data[6]);
const tUnit tUnit::km(internal::unit_data[7]);
const tUnit tUnit::km_h(internal::unit_data[8]);
const tUnit tUnit::m_s(internal::unit_data[9]);
const tUnit tUnit::mg(internal::unit_data[10]);
const tUnit tUnit::g(internal::unit_data[11]);
const tUnit tUnit::kg(internal::unit_data[12]);
const tUnit tUnit::t(internal::unit_data[13]);
const tUnit tUnit::mt(internal::unit_data[14]);
const tUnit tUnit::ns(internal::unit_data[15]);
const tUnit tUnit::us(internal::unit_data[16]);
const tUnit tUnit::ms(internal::unit_data[17]);
const tUnit tUnit::s(internal::unit_data[18]);
const tUnit tUnit::min(internal::unit_data[19]);
const tUnit tUnit::h(internal::unit_data[20]);
const tUnit tUnit::day(internal::unit_data[21]);
const tUnit tUnit::deg(internal::unit_data[22]);
const tUnit tUnit::rad(internal::unit_data[23]);
const tUnit tUnit::Hz(internal::unit_data[24]);
const tUnit tUnit::Pixel(internal::unit_data[25]);

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

double tUnit::ConvertTo(double value, const tUnit& to_unit) const
{
  if (*this == tUnit::cNO_UNIT || to_unit == tUnit::cNO_UNIT)
  {
    return value;
  }
  return GetConversionFactor(to_unit) * value;
}

double tUnit::GetConversionFactor(const tUnit& to_unit) const
{
  if (ConvertibleTo(to_unit))
  {
    return wrapped->factor / to_unit.wrapped->factor;
  }
  FINROC_LOG_PRINT(WARNING, "Unit ", GetName(), " cannot be converted to ", to_unit.GetName(), ". Not converting.");
  return 1;
}

tUnit tUnit::GetUnit(const std::string& name)
{
  for (size_t i = 0; i < internal::cUNIT_COUNT; i++)
  {
    if (name.compare(internal::unit_data[i].name) == 0)
    {
      return tUnit(internal::unit_data[i]);
    }
  }
  return tUnit();
}

rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tUnit& unit)
{
  stream.WriteByte(static_cast<int>(unit.wrapped - internal::unit_data));
  return stream;
}

rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tUnit& unit)
{
  uint32_t index = stream.ReadByte();
  if (index >= internal::cUNIT_COUNT)
  {
    FINROC_LOG_PRINT(ERROR, "Invalid unit index in stream: ", index);
    unit = tUnit();
  }
  else
  {
    unit = tUnit(internal::unit_data[index]);
  }
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

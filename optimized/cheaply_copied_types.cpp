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
/*!\file    plugins/data_ports/optimized/cheaply_copied_types.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-03
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/cheaply_copied_types.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/thread/tLock.h"
#include "core/definitions.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/type_traits.h"
#include "plugins/data_ports/numeric/tNumber.h"

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
namespace optimized
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------
namespace internal
{

/*! Register with types */
struct tRegister
{
  struct tEntry
  {
    rrlib::rtti::tType type;
    size_t port_count;

    tEntry(rrlib::rtti::tType type) : type(type), port_count(1)
    {}
  };

  /*! Cheaply copied types used in ports */
  std::vector<tEntry> used_types;

  tRegister() :
    used_types()
  {
    used_types.reserve(cMAX_CHEAPLY_COPYABLE_TYPES + 1); // avoids reallocation which would be a thread-safety issue

    // Put number at position zero - as this is the most frequently used type
    used_types.emplace_back(rrlib::rtti::tDataType<numeric::tNumber>());
    used_types[0].port_count = 0;
  }
};

}

/*!
 * \return Register singleton
 */
static internal::tRegister& GetRegister()
{
  static internal::tRegister the_register;
  return the_register;
}

size_t GetPortCount(uint32_t cheaply_copied_type_index)
{
  return GetRegister().used_types[cheaply_copied_type_index].port_count;
}

size_t GetRegisteredTypeCount()
{
  return GetRegister().used_types.size();
}

rrlib::rtti::tType GetType(uint32_t cheaply_copied_type_index)
{
  return GetRegister().used_types[cheaply_copied_type_index].type;
}

uint32_t RegisterPort(const rrlib::rtti::tType& type)
{
  static rrlib::thread::tMutex mutex;
  rrlib::thread::tLock lock(mutex);
  if (!IsCheaplyCopiedType(type))
  {
    FINROC_LOG_PRINT(ERROR, "Invalid type registered");
    abort();
  }

  for (auto it = GetRegister().used_types.begin(); it != GetRegister().used_types.end(); ++it)
  {
    if (it->type == type)
    {
      it->port_count++;
      return it - GetRegister().used_types.begin();
    }
  }

  uint32_t result = GetRegister().used_types.size();
  GetRegister().used_types.emplace_back(type);
  if (result >= cMAX_CHEAPLY_COPYABLE_TYPES)
  {
    FINROC_LOG_PRINT(ERROR, "Maximum number of cheaply copyable types exceeded");
    abort();
  }
  return result;
}

void UnregisterPort(uint32_t cheaply_copied_type_index)
{
  GetRegister().used_types[cheaply_copied_type_index].port_count--;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

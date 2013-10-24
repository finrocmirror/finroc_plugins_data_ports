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
#include "rrlib/rtti/tTypeAnnotation.h"
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

class tIndexAnnotation : public rrlib::rtti::tTypeAnnotation
{
public:

  tIndexAnnotation(uint32_t index) : index(index) {}

  /*! Cheaply copied typed index */
  uint32_t index;
};

/*! Register with types */
struct tRegister
{
  struct tEntry
  {
    rrlib::rtti::tType type;
    std::atomic<size_t> port_count;

    tEntry() : type(), port_count(0)
    {}
  };

  /*! Cheaply copied types used in ports */
  std::array<tEntry, cMAX_CHEAPLY_COPYABLE_TYPES> used_types;

  /*! Number of registered types */
  size_t registered_types;

  tRegister() :
    used_types(),
    registered_types(1)
  {
    // Put number at position zero - as this is the most frequently used type
    used_types[0].type = rrlib::rtti::tDataType<numeric::tNumber>();
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


uint32_t GetCheaplyCopiedTypeIndex(const rrlib::rtti::tType& type)
{
  internal::tIndexAnnotation* annotation = type.GetAnnotation<internal::tIndexAnnotation>();
  if (annotation)
  {
    return annotation->index;
  }

  static rrlib::thread::tMutex mutex;
  rrlib::thread::tLock lock(mutex);
  if (!IsCheaplyCopiedType(type))
  {
    FINROC_LOG_PRINT(ERROR, "Invalid type registered");
    abort();
  }

  // check again - now synchronized - as type could have been added
  internal::tRegister& reg = GetRegister();
  for (size_t i = 0; i < reg.registered_types; i++)
  {
    if (reg.used_types[i].type == type)
    {
      return i;
    }
  }

  uint32_t result = reg.registered_types;
  reg.used_types[result].type = type;
  reg.registered_types++;
  rrlib::rtti::tType type_copy = type;
  type_copy.AddAnnotation(new internal::tIndexAnnotation(result));
  if (result >= cMAX_CHEAPLY_COPYABLE_TYPES)
  {
    FINROC_LOG_PRINT(ERROR, "Maximum number of cheaply copyable types exceeded");
    abort();
  }
  return result;
}

size_t GetPortCount(uint32_t cheaply_copied_type_index)
{
  return GetRegister().used_types[cheaply_copied_type_index].port_count;
}

size_t GetRegisteredTypeCount()
{
  return GetRegister().registered_types;
}

rrlib::rtti::tType GetType(uint32_t cheaply_copied_type_index)
{
  return GetRegister().used_types[cheaply_copied_type_index].type;
}

uint32_t RegisterPort(const rrlib::rtti::tType& type)
{
  uint32_t result = GetCheaplyCopiedTypeIndex(type);
  GetRegister().used_types[result].port_count++;
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

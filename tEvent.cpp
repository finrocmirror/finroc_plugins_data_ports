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
/*!\file    plugins/data_ports/tEvent.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2017-09-22
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/tEvent.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/tRegisteredConversionOperation.h"
#include "rrlib/rtti_conversion/tCompiledConversionOperation.h"

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

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

/*! Initializes tEvent data type */
static rrlib::rtti::tDataType<tEvent> cINIT_EVENT_DATA_TYPE("Event");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

namespace
{
class tAnyToEvent : public rrlib::rtti::conversion::tRegisteredConversionOperation
{
public:
  tAnyToEvent() : tRegisteredConversionOperation(rrlib::util::tManagedConstCharPointer("ToEvent", false), rrlib::rtti::conversion::tSupportedTypeFilter::ALL, rrlib::rtti::tDataType<tEvent>())
  {}

  virtual rrlib::rtti::conversion::tConversionOption GetConversionOption(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type, const rrlib::rtti::tGenericObject* parameter) const override
  {
    if (destination_type == rrlib::rtti::tDataType<tEvent>())
    {
      return rrlib::rtti::conversion::tConversionOption(source_type, destination_type, 0);
    }
    return rrlib::rtti::conversion::tConversionOption();
  }
} cANY_TO_EVENT;

class tEventToDefault : public rrlib::rtti::conversion::tRegisteredConversionOperation
{
public:
  tEventToDefault() : tRegisteredConversionOperation(rrlib::util::tManagedConstCharPointer("PublishDefaultValue", false), rrlib::rtti::tDataType<tEvent>(), rrlib::rtti::conversion::tSupportedTypeFilter::ALL, nullptr, rrlib::rtti::tParameterDefinition(), &cANY_TO_EVENT)
  {}

  virtual rrlib::rtti::conversion::tConversionOption GetConversionOption(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type, const rrlib::rtti::tGenericObject* parameter) const override
  {
    if (source_type == rrlib::rtti::tDataType<tEvent>())
    {
      return rrlib::rtti::conversion::tConversionOption(source_type, destination_type, false, &FirstConversionFunction, &FinalConversionFunction);
    }
    return rrlib::rtti::conversion::tConversionOption();
  }

  static void FirstConversionFunction(const rrlib::rtti::tTypedConstPointer& source_object, const rrlib::rtti::tTypedPointer& destination_object, const rrlib::rtti::conversion::tCurrentConversionOperation& operation)
  {
    rrlib::rtti::tType inter_type = operation.compiled_operation.IntermediateType();
    char intermediate_memory[inter_type.GetSize(true)];
    auto intermediate_object = inter_type.EmplaceGenericObject(intermediate_memory);
    operation.Continue(*intermediate_object, destination_object);
  }

  static void FinalConversionFunction(const rrlib::rtti::tTypedConstPointer& source_object, const rrlib::rtti::tTypedPointer& destination_object, const rrlib::rtti::conversion::tCurrentConversionOperation& operation)
  {
    rrlib::rtti::tType inter_type = operation.compiled_operation.IntermediateType();
    char intermediate_memory[inter_type.GetSize(true)];
    auto intermediate_object = inter_type.EmplaceGenericObject(intermediate_memory);
    destination_object.DeepCopyFrom(*intermediate_object);
  }
} cEVENT_TO_DEFAULT;

}

const rrlib::rtti::conversion::tRegisteredConversionOperation& cANY_TO_EVENT_OPERATION = cANY_TO_EVENT;
const rrlib::rtti::conversion::tRegisteredConversionOperation& cEVENT_TO_DEFAULT_OPERATION = cEVENT_TO_DEFAULT;

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

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
/*!\file    plugins/data_ports/optimized/tSingleThreadedCheapCopyPortGeneric.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2014-06-22
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/tSingleThreadedCheapCopyPortGeneric.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/cheaply_copied_types.h"

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

tSingleThreadedCheapCopyPortGeneric::tSingleThreadedCheapCopyPortGeneric(common::tAbstractDataPortCreationInfo creation_info) :
  common::tAbstractDataPort(creation_info),
  current_value(),
  default_value(),
  max_queue_length(-1),
  standard_assign(!GetFlag(tFlag::NON_STANDARD_ASSIGN) && (!GetFlag(tFlag::HAS_QUEUE)))
{
  current_value.data.reset(creation_info.data_type.CreateGenericObject());
  current_value.data_pointer = current_value.data->GetRawDataPointer();
  current_value.cheaply_copyable_type_index = RegisterPort(creation_info.data_type);
  current_value.timestamp = rrlib::time::cNO_TIME;

  if ((!IsDataFlowType(GetDataType())) || (!IsCheaplyCopiedType(GetDataType())))
  {
    FINROC_LOG_PRINT(ERROR, "Data type ", GetDataType().GetName(), " is not suitable for cheap copy port implementation.");
    abort();
  }

  // set initial value to default?
  if (creation_info.DefaultValueSet() || creation_info.flags.Get(core::tFrameworkElement::tFlag::DEFAULT_ON_DISCONNECT))
  {
    default_value.reset(creation_info.data_type.CreateGenericObject());
    if (creation_info.DefaultValueSet())
    {
      rrlib::serialization::tInputStream stream(creation_info.GetDefaultGeneric());
      default_value->Deserialize(stream);
    }
    current_value.data->DeepCopyFrom(*default_value);
  }

  // Queues are only supported in subclass
}

tSingleThreadedCheapCopyPortGeneric::~tSingleThreadedCheapCopyPortGeneric()
{}

void tSingleThreadedCheapCopyPortGeneric::ApplyDefaultValue()
{
  if (!default_value)
  {
    FINROC_LOG_PRINT(ERROR, "No default value has been set. Doing nothing.");
    return;
  }
  BrowserPublishRaw(*default_value, rrlib::time::cNO_TIME, true);
}

std::string tSingleThreadedCheapCopyPortGeneric::BrowserPublishRaw(const rrlib::rtti::tGenericObject& buffer, rrlib::time::tTimestamp timestamp,
    bool notify_listener_on_this_port, tChangeStatus change_constant)
{
  current_value.data->DeepCopyFrom(buffer);
  current_value.timestamp = timestamp;
  common::tPublishOperation<tSingleThreadedCheapCopyPortGeneric, tPublishingData> publish_operation(current_value);
  if (notify_listener_on_this_port)
  {
    if (change_constant == tChangeStatus::CHANGED_INITIAL)
    {
      publish_operation.Execute<tChangeStatus::CHANGED_INITIAL, true, true>(*this);
    }
    else
    {
      publish_operation.Execute<tChangeStatus::CHANGED, true, true>(*this);
    }
  }
  else
  {
    if (change_constant == tChangeStatus::CHANGED_INITIAL)
    {
      publish_operation.Execute<tChangeStatus::CHANGED_INITIAL, true, false>(*this);
    }
    else
    {
      publish_operation.Execute<tChangeStatus::CHANGED, true, false>(*this);
    }
  }
  return "";
}

void tSingleThreadedCheapCopyPortGeneric::ForwardData(tAbstractDataPort& other)
{
  assert(IsDataFlowType(other.GetDataType()) && (IsCheaplyCopiedType(other.GetDataType())));

  common::tPublishOperation<tSingleThreadedCheapCopyPortGeneric, tPublishingData> publish_operation(current_value);
  publish_operation.Execute<tChangeStatus::CHANGED, false, false>(static_cast<tSingleThreadedCheapCopyPortGeneric&>(other));
}

int tSingleThreadedCheapCopyPortGeneric::GetMaxQueueLengthImplementation() const
{
  return max_queue_length;
}

void tSingleThreadedCheapCopyPortGeneric::InitialPushTo(core::tConnector& connector)
{
  if (typeid(connector) == typeid(common::tConversionConnector))
  {
    static_cast<common::tConversionConnector&>(connector).Publish(*current_value.data, tChangeStatus::CHANGED_INITIAL);
  }
  else
  {
    common::tPublishOperation<tSingleThreadedCheapCopyPortGeneric, tPublishingData> data(current_value);
    tSingleThreadedCheapCopyPortGeneric& target_port = static_cast<tSingleThreadedCheapCopyPortGeneric&>(connector.Destination());
    common::tPublishOperation<tSingleThreadedCheapCopyPortGeneric, tPublishingData>::Receive<tChangeStatus::CHANGED_INITIAL>(data, target_port, *this);
  }
}

bool tSingleThreadedCheapCopyPortGeneric::NonStandardAssign(tPublishingData& publishing_data, tChangeStatus change_constant)
{
  throw std::logic_error("Only implemented for/in subclasses");
}

void tSingleThreadedCheapCopyPortGeneric::Publish(const rrlib::rtti::tGenericObject& data, rrlib::time::tTimestamp timestamp)
{
  if (!GetFlag(tFlag::HIJACKED_PORT))
  {
    current_value.data->DeepCopyFrom(data);
    current_value.timestamp = timestamp;
    common::tPublishOperation<tSingleThreadedCheapCopyPortGeneric, tPublishingData> publish_operation(current_value);
    publish_operation.Execute<tChangeStatus::CHANGED, false, false>(*this);
  }
}

void tSingleThreadedCheapCopyPortGeneric::SetCurrentValueBuffer(void* address)
{
  std::unique_ptr<rrlib::rtti::tGenericObject> new_buffer(GetDataType().CreateGenericObject(address));
  new_buffer->DeepCopyFrom(*current_value.data);
  std::swap(new_buffer, current_value.data);
  current_value.data_pointer = current_value.data->GetRawDataPointer();
}

void tSingleThreadedCheapCopyPortGeneric::SetDefault(rrlib::rtti::tGenericObject& new_default)
{
  default_value.reset(new_default.GetType().CreateGenericObject());
  default_value->DeepCopyFrom(new_default);
  current_value.data->DeepCopyFrom(new_default);
}


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

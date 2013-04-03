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
/*!\file    plugins/data_ports/api/tDeserializationScope.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-12-19
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/api/tDeserializationScope.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tPort.h"

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
namespace api
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

__thread tDeserializationScope* tDeserializationScope::current_scope;

tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false> tDeserializationScope::GetUnusedBuffer(const rrlib::rtti::tType& type)
{
  if (!IsDataFlowType(type))
  {
    throw std::runtime_error("Not a data flow type");
  }
  if (IsCheaplyCopiedType(type))
  {
    uint32_t index = optimized::GetCheaplyCopiedTypeIndex(type);
    if (optimized::tThreadLocalBufferPools::Get())
    {
      return tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false>(optimized::tThreadLocalBufferPools::Get()->GetUnusedBuffer(index).release(), true);
    }
    else
    {
      return tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false>(optimized::tGlobalBufferPools::Instance().GetUnusedBuffer(index).release(), true);
    }
  }
  else
  {
    if (!current_scope)
    {
      throw std::runtime_error("No scope created");
    }
    if (!current_scope->buffer_source)
    {
      current_scope->buffer_source = &current_scope->ObtainBufferPool();
    }
    auto buffer = current_scope->buffer_source->GetUnusedBuffer(type);
    return tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false>(buffer.release(), true);
  }
}

standard::tMultiTypePortBufferPool& tDeserializationScope::ObtainBufferPool()
{
  if (!buffer_source)
  {
    throw std::logic_error("This must be implemented if scope class can be created without providing buffer pool.");
  }
  return *buffer_source;
}



//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

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
/*!\file    plugins/data_ports/standard/tMultiTypePortBufferPool.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/standard/tMultiTypePortBufferPool.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

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
namespace standard
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
tMultiTypePortBufferPool::tMultiTypePortBufferPool() :
  tMutex(),
  pools(),
  first_external(false)
{
}

tMultiTypePortBufferPool::tMultiTypePortBufferPool(tBufferPool& first, const rrlib::rtti::tType& first_data_type) :
  tMutex(),
  pools(),
  first_external(true)
{
  pools.push_back(tPoolsEntry(first_data_type, std::unique_ptr<tBufferPool>(&first)));
}

tMultiTypePortBufferPool::~tMultiTypePortBufferPool()
{
  if (first_external)
  {
    pools[0].second.release();
  }
}

tMultiTypePortBufferPool::tPointer tMultiTypePortBufferPool::PossiblyCreatePool(const rrlib::rtti::tType& data_type)
{
  rrlib::thread::tLock lock(*this);

  // search for correct pool
  for (auto it = pools.begin(); it != pools.end(); ++it)
  {
    if (it->first == data_type)
    {
      return it->second->GetUnusedBuffer(data_type);
    }
  }

  // create new pool
  tBufferPool* new_pool = new tBufferPool(data_type, 2);
  pools.emplace_back(tPoolsEntry(data_type, std::unique_ptr<tBufferPool>(new_pool)));
  return new_pool->GetUnusedBuffer(data_type);
}

void tMultiTypePortBufferPool::PrintStructure(int indent, std::stringstream& output)
{
  for (int i = 0; i < indent; i++)
  {
    output << " ";
  }
  output << "MultiTypePortDataBufferPool:" << std::endl;
  for (auto it = pools.begin(); it != pools.end(); ++it)
  {
    for (int i = 0; i < indent + 2; i++)
    {
      output << " ";
    }
    output << "PortDataBufferPool (" << it->first.GetName() << ")" << std::endl;
  }
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

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
/*!\file    plugins/data_ports/standard/tMultiTypePortBufferPool.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 * \brief   Contains tMultiTypePortBufferPool
 *
 * \b tMultiTypePortBufferPool
 *
 * Buffer pool for specific port and thread.
 * Special version that supports buffers of multiple types.
 * This list is not real-time capable if new types are used.
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__standard__tMultiTypePortBufferPool_h__
#define __plugins__data_ports__standard__tMultiTypePortBufferPool_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tPortBufferPool.h"
#include "plugins/data_ports/standard/tPortBufferManager.h"

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
// Class declaration
//----------------------------------------------------------------------
//! Buffer pool for multiple data types
/*!
 * Buffer pool for specific port and thread.
 * Special version that supports buffers of multiple types.
 * This list is not real-time capable if new types are used.
 */
class tMultiTypePortBufferPool : public rrlib::thread::tMutex
{

  /*! Buffer pool used by standard port implementation */
  typedef common::tPortBufferPool<tPortBufferManager, rrlib::concurrent_containers::tConcurrency::FULL> tBufferPool;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! std::unique_ptr returned by this class that will automatically recycle buffer when out of scope */
  typedef typename tBufferPool::tPointer tPointer;

  /*!
   * \param first (Buffer pool of first type (optional, not deleted by this class))
   * \param first_data_type Data type of first pool
   */
  tMultiTypePortBufferPool(tBufferPool& first, const rrlib::rtti::tType& first_data_type);
  tMultiTypePortBufferPool();

  ~tMultiTypePortBufferPool();

  /*!
   * \param data_type DataType of returned buffer.
   * \return Returns unused buffer. If there are no buffers that can be reused, a new buffer is allocated.
   */
  inline tPointer GetUnusedBuffer(const rrlib::rtti::tType& data_type)
  {
    // search for correct pool
    for (auto it = pools.begin(); it != pools.end(); ++it)
    {
      if (it->first == data_type)
      {
        return it->second->GetUnusedBuffer(data_type);
      }
    }

    return PossiblyCreatePool(data_type);
  }

  /*!
   * Prints all pools including elements of multi-type pool
   *
   * \param indent Current indentation
   */
  void PrintStructure(int indent, std::stringstream& output);

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  typedef std::pair<rrlib::rtti::tType, std::unique_ptr<tBufferPool>> tPoolsEntry;

  /*! list contains pools for different data types... new pools are added when needed */
  std::vector<tPoolsEntry> pools;

  /*! Has first buffer pool been obtained externally? */
  bool first_external;

  /*!
   * \param data_type DataType of buffer to create
   * \return Returns unused buffer of possibly newly created pool
   */
  tPointer PossiblyCreatePool(const rrlib::rtti::tType& data_type);
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

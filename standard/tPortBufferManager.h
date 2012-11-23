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
/*!\file    plugins/data_ports/standard/tPortBufferManager.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-30
 *
 * \brief   Contains tPortBufferManager
 *
 * \b tPortBufferManager
 *
 * This class manages a single port buffer.
 * It handles information on locks, data type, timestamp etc.
 *
 * If it possible to derive a port buffer manager from another port buffer manager.
 * They will share the same reference counter.
 * This makes sense, when an object contained in the original port buffer
 * shall be used in another port.
 * This way, it does not need to copied.
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__standard__tPortBufferManager_h__
#define __plugins__data_ports__standard__tPortBufferManager_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tReferenceCountingBufferManager.h"

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
//! Manages a port buffer
/*!
 * This class manages a single port buffer.
 * It handles information on locks, data type, timestamp etc.
 *
 * TODO: If it possible to derive a port buffer manager from another port buffer manager.
 * They will share the same reference counter.
 * This makes sense, when an object contained in the original port buffer
 * shall be used in another port.
 * This way, it does not need to copied.
 */
class tPortBufferManager : public common::tReferenceCountingBufferManager
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Creates instance of tPortBufferManager containing a buffer
   * of the specified type
   *
   * \param Type of buffer
   * \return Created instance (can be deleted like any other objects using delete operator)
   */
  static tPortBufferManager* CreateInstance(const rrlib::rtti::tType& type);

  /*!
   * \return Managed Buffer as generic object
   */
  inline rrlib::rtti::tGenericObject& GetObject()
  {
    return reinterpret_cast<rrlib::rtti::tGenericObject&>(*(this + 1));
  }

  /*!
   * \return Is this (still) a unused buffer?
   */
  inline bool IsUnused() const
  {
    return unused;
  }

  /*!
   * \param unused Whether to mark this buffer as still unused
   */
  inline void SetUnused(bool unused)
  {
    this->unused = unused;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Value relevant for publishing thread only - is this still a unused buffer? */
  bool unused;

  /*! PortDataManager that this manager is derived from - null if not derived */
  tPortBufferManager* derived_from;

  /*! Helper variable - e.g. for blackboards; TODO: remove as soon as we have more complex responses from RPC calls */
  int lock_id;


  tPortBufferManager();

  virtual rrlib::rtti::tGenericObject& GetObjectImplementation();
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

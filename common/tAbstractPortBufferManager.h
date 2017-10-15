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
/*!\file    plugins/data_ports/common/tAbstractPortBufferManager.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-30
 *
 * \brief   Contains tAbstractPortBufferManager
 *
 * \b tAbstractPortBufferManager
 *
 * Base class for all port data manager classes used in data ports.
 * Contains things like time stamp that are common to these classes.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__common__tAbstractPortBufferManager_h__
#define __plugins__data_ports__common__tAbstractPortBufferManager_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/buffer_pools/tBufferManagementInfo.h"
#include "rrlib/concurrent_containers/tQueueable.h"
#include "rrlib/rtti/rtti.h"
#include "core/definitions.h"

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
namespace common
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Base class for different kinds of port data manager classes
/*!
 * Base class for all port data manager classes used in data ports.
 * Contains things like time stamp that are common to these classes.
 */
template <rrlib::concurrent_containers::tQueueability QUEUEABILITY>
class  __attribute__((aligned(8))) tAbstractPortBufferManager : public rrlib::concurrent_containers::tQueueable<QUEUEABILITY>, public rrlib::buffer_pools::tBufferManagementInfo
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tAbstractPortBufferManager() : timestamp(rrlib::time::cNO_TIME) {}

  virtual ~tAbstractPortBufferManager() {}

  /*!
   * \return String containing content type and data pointer
   */
  inline tString GetContentString()
  {
    std::ostringstream os;
    os << GetObjectImplementation().GetType().GetName() << " (" << GetObjectImplementation().GetRawDataPointer() << ")";
    return os.str();
  }

  /*!
   * \return Timestamp for currently managed data
   */
  inline const rrlib::time::tTimestamp& GetTimestamp() const
  {
    return timestamp;
  }

  /*!
   * \param timestamp New timestamp for currently managed data
   */
  inline void SetTimestamp(const rrlib::time::tTimestamp& timestamp)
  {
    this->timestamp = timestamp;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Timestamp for currently managed data */
  rrlib::time::tTimestamp timestamp;

  /*!
   * \return Managed Buffer as generic object
   */
  virtual rrlib::rtti::tGenericObject& GetObjectImplementation() = 0;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

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
/*!\file    plugins/data_ports/optimized/tCheaplyCopiedBufferManager.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-04
 *
 * \brief   Contains tCheaplyCopiedBufferManager
 *
 * \b tCheaplyCopiedBufferManager
 *
 * This class manages a single port buffer.
 * It handles information on locks, data type, timestamp etc.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__optimized__tCheaplyCopiedBufferManager_h__
#define __plugins__data_ports__optimized__tCheaplyCopiedBufferManager_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/thread/tThread.h"

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
namespace optimized
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Manager of 'cheaply copied' buffer.
/*!
 * This class manages a single, 'cheaply copied' port data buffer.
 * It handles information on locks, data type, timestamp etc.
 */
class tCheaplyCopiedBufferManager : public common::tReferenceCountingBufferManager
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  typedef typename rrlib::thread::tThread::tThreadId tThreadId;

  /*!
   * Creates instance of tCheaplyCopiedBufferManager containing a buffer
   * of the specified type
   *
   * \param Type of buffer
   * \return Created instance (can be deleted like any other objects using delete operator)
   */
  static tCheaplyCopiedBufferManager* CreateInstance(const rrlib::rtti::tType& type);

  /*!
   * \return Managed Buffer as generic object
   */
  inline rrlib::rtti::tGenericObject& GetObject()
  {
    return reinterpret_cast<rrlib::rtti::tGenericObject&>(*(this + 1));
  }

  /*!
   * \return True, if the current thread is the owner of this buffer
   */
  bool IsOwnerThread() const
  {
    return owner_thread_id == rrlib::thread::tThread::CurrentThreadId();
  }

  /*!
   * \return Id of owner thread - zero if there's no owner thread - otherwise this
   * indicates that this is actually an object of the subclass tThreadLocalBufferManager
   */
  tThreadId GetOwnerThreadId() const
  {
    return owner_thread_id;
  }

//----------------------------------------------------------------------
// Protected constructor for subclass
//----------------------------------------------------------------------
protected:

  tCheaplyCopiedBufferManager(tThreadId owner_thread_id = 0);

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  const tThreadId owner_thread_id;

  virtual rrlib::rtti::tGenericObject& GetObjectImplementation();
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

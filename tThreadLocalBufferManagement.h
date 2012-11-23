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
/*!\file    plugins/data_ports/tThreadLocalBufferManagement.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-04
 *
 * \brief   Contains tThreadLocalBufferManagement
 *
 * \b tThreadLocalBufferManagement
 *
 * Installs optimized buffer management for the current thread when dealing
 * with 'cheaply copied' types.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tThreadLocalBufferManagement_h__
#define __plugins__data_ports__tThreadLocalBufferManagement_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

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

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Thread local buffer pool instantiation and installation
/*!
 * Installs optimized buffer management for the current thread when dealing
 * with 'cheaply copied' types.
 *
 * Can exist once per thread and will greatly increase publishing performance -
 * and also memory usage:
 * Thread-local buffer pools are instantiated.
 * This produces overhead for buffer pool initialization and destruction.
 * Should only be used by long-lived, frequently publishing threads.
 *
 * Should be allocated on the thread's stack so that it is always deleted when
 * the thread exits.
 * (Alternatively, it can be attached to thread object to ensure deletion - using
 *  tThread::LockObject(...)  )
 */
class tThreadLocalBufferManagement : boost::noncopyable
{

  tThreadLocalBufferManagement() :
    pools(new optimized::tThreadLocalBufferPools())
  {}

  ~tThreadLocalBufferManagement()
  {
    pools.SafeDelete();
  }

private:

  /*! Pointer to allocated pools */
  optimized::tThreadLocalBufferPools* pools;
};


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

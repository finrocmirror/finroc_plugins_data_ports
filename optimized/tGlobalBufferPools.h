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
/*!\file    plugins/data_ports/optimized/tGlobalBufferPools.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-04
 *
 * \brief   Contains tGlobalBufferPools
 *
 * \b tGlobalBufferPools
 *
 * Global set of buffer pools for 'cheaply copied' types.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__optimized__tGlobalBufferPools_h__
#define __plugins__data_ports__optimized__tGlobalBufferPools_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/optimized/tThreadSpecificBufferPools.h"

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
//! Global buffer pools
/*!
 * Global set of buffer pools for 'cheaply copied' types.
 */
class tGlobalBufferPools : public tThreadSpecificBufferPools<true>
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \return Singleton instance
   */
  static tGlobalBufferPools& Instance()
  {
    return instance;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  tGlobalBufferPools();

  ~tGlobalBufferPools();

  /*! singleton instance */
  static tGlobalBufferPools instance;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

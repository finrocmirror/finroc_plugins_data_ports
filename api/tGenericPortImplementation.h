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
/*!\file    plugins/data_ports/api/tGenericPortImplementation.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 * \brief   Contains tGenericPortImplementation
 *
 * \b tGenericPortImplementation
 *
 * Implementations for tGenericPort.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__api__tGenericPortImplementation_h__
#define __plugins__data_ports__api__tGenericPortImplementation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tAbstractDataPortCreationInfo.h"
#include "plugins/data_ports/common/tAbstractDataPort.h"

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
// Class declaration
//----------------------------------------------------------------------
//! Generic port implementation
/*!
 * Implementations for tGenericPort.
 */
class tGenericPortImplementation
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  friend class tGenericPort;

  /*!
   * Creates port backend for specified port creation info
   *
   * \param pci Information for port creation
   */
  static tGenericPortImplementation* CreatePortImplementation(const common::tAbstractDataPortCreationInfo& pci);

  /*!
   * Gets Port's current value
   *
   * (Note that numbers and "cheap copy" types also have a method: T GetValue();  (defined in tPortParent<T>))
   *
   * \param result Buffer to (deep) copy port's current value to
   * (Using this get()-variant is more efficient when using CC types, but can be extremely costly with large data types)
   */
  virtual void Get(rrlib::rtti::tGenericObject& result, rrlib::time::tTimestamp& timestamp) = 0;

  /*!
   * \return Wrapped port.
   */
  virtual common::tAbstractDataPort* GetWrapped() = 0;

  /*!
   * Publish Data Buffer. This data will be forwarded to any connected ports.
   * Should only be called on output ports.
   *
   * \param data Data to publish. It will be deep-copied.
   * This publish()-variant is efficient when using CC types, but can be extremely costly with large data types)
   */
  virtual void Publish(const rrlib::rtti::tGenericObject& data, const rrlib::time::tTimestamp& timestamp) = 0;

  /*!
   * Set new bounds
   * (This is not thread-safe and must only be done in "pause mode")
   *
   * \param b New Bounds
   */
  virtual void SetBounds(const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max) = 0;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

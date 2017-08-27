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
/*!\file    plugins/data_ports/optimized/cheaply_copied_types.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-03
 *
 * \brief
 *
 * Utility functions with respect to cheaply copied types.
 * They keep track of cheaply copied types used in ports
 * and assign unique index to each of them.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__optimized__cheaply_copied_types_h__
#define __plugins__data_ports__optimized__cheaply_copied_types_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/rtti/rtti.h"

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
namespace optimized
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Function declarations
//----------------------------------------------------------------------

/*!
 * \param type Data type
 * \return Buffer pool index for this type
 */
uint32_t GetCheaplyCopiedBufferPoolIndex(const rrlib::rtti::tType& type);

/*!
 * \param cheaply_copied_type_buffer_pool_index Buffer pool index for this type
 * \return Number of ports that use this buffer pool
 */
size_t GetPortCount(uint32_t cheaply_copied_type_buffer_pool_index);

/*!
 * Register port for use of specified 'cheaply copied' type
 *
 * \param type Type that port uses
 * \return Buffer pool index for this type
 */
uint32_t RegisterPort(const rrlib::rtti::tType& type);

/*!
 * Unregister port for use of specified 'cheaply copied' type
 *
 * \param type Type that port uses
 */
void UnregisterPort(const rrlib::rtti::tType& type);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

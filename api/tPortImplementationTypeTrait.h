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
/*!\file    plugins/data_ports/api/tPortImplementationTypeTrait.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-17
 *
 * \brief   Contains tPortImplementationTypeTrait
 *
 * \b tPortImplementationTypeTrait
 *
 * Determines port implementation type for a type T.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__api__tPortImplementationTypeTrait_h__
#define __plugins__data_ports__api__tPortImplementationTypeTrait_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/type_traits.h"

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

/*!
 * Available port implementations
 */
enum class tPortImplementationType
{
  STANDARD,
  CHEAP_COPY,
  NUMERIC
};

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Determines port implementation type for a type T.
/*!
 * Determines port implementation type for a type T.
 */
template <typename T>
struct tPortImplementationTypeTrait
{
  /*! Port implementation to use */
  static const tPortImplementationType type = IsNumeric<T>::value ? tPortImplementationType::NUMERIC :
      (tIsCheaplyCopiedType<T>::value ? tPortImplementationType::CHEAP_COPY : tPortImplementationType::STANDARD);
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

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
/*!\file    plugins/data_ports/tPortPack.h
 *
 * \author  Tobias Föhst
 *
 * \date    2012-10-18
 *
 * \brief   Contains tPortPack
 *
 * \b tPortPack
 *
 * A group of several ports with different types.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tPortPack_h__
#define __plugins__data_ports__tPortPack_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/util/tTypeList.h"
#include "core/tFrameworkElement.h"

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
//! A group of several ports with different types.
/*!
 * This class creates a list of instances of the given port template
 * inserting several types from a list.  It therefore creates a nested
 * structure of inherited classes and provides a method to access the
 * port on a specific layer at runtime.
 *
 * \param TPort       A port class template to use for every packed port
 * \param TTypeList   A list of the data types used in the ports. e.g. rrlib::util::tTypeList
 * \param Tsize       The pack creates ports using TTypeList[0] to TTypeList[Tsize - 1]. This parameter must not be greater than TTypeList::cSIZE - 1 and is typically inferred and not set by the user.
 */
template <template <typename> class TPort, typename TTypeList, size_t Tsize = rrlib::util::type_list::tSizeOf<TTypeList>::cVALUE>
class tPortPack : private tPortPack < TPort, TTypeList, Tsize - 1 >
{

  template <bool value, typename>
  using CheckIterators = std::integral_constant<bool, value>;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  inline tPortPack(core::tFrameworkElement *parent, const std::string &name_prefix) :
    tPortPack < TPort, TTypeList, Tsize - 1 > (parent, name_prefix),
    port(name_prefix + std::to_string(Tsize), parent)
  {
    this->port.Init();
  }

  template <typename TIterator, typename std::enable_if<CheckIterators<(Tsize> 1), TIterator>::value, int>::type = 0 >
      inline tPortPack(core::tFrameworkElement *parent, TIterator names_begin, TIterator names_end) :
        tPortPack < TPort, TTypeList, Tsize - 1 > (parent, names_begin, names_end - 1),
        port(*(names_end - 1), parent)
  {
    this->port.Init();
  }

  template <typename TIterator, typename std::enable_if<CheckIterators<(Tsize == 1), TIterator>::value, int>::type = 0>
  inline tPortPack(core::tFrameworkElement *parent, TIterator names_begin, TIterator names_end) :
    tPortPack < TPort, TTypeList, Tsize - 1 > (parent, *names_begin),
    port(*names_begin, parent)
  {
    this->port.Init();
  }

  inline size_t NumberOfPorts() const
  {
    return Tsize;
  }

  inline core::tPortWrapperBase &GetPort(size_t index)
  {
    assert(index < this->NumberOfPorts());
    if (index == Tsize - 1)
    {
      return this->port;
    }
    return tPortPack < TPort, TTypeList, Tsize - 1 >::GetPort(index);
  }

  inline bool HasChanged(size_t index)
  {
    assert(index < this->NumberOfPorts());
    if (index == Tsize - 1)
    {
      return this->port.HasChanged();
    }
    return tPortPack < TPort, TTypeList, Tsize - 1 >::HasChanged(index);
  }

  inline void ManagedDelete()
  {
    this->port.GetWrapped()->ManagedDelete();
    tPortPack < TPort, TTypeList, Tsize - 1 >::ManagedDelete();
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------

private:

  TPort < typename TTypeList::template tAt < Tsize - 1 >::tResult > port;

};

//! The partial specialization of tPortPack to terminate recursion
template <template <typename> class TPort, typename TTypeList>
struct tPortPack <TPort, TTypeList, 0>
{
  inline tPortPack(core::tFrameworkElement *parent, const std::string &name_prefix)
  {}

  template <typename TIterator>
  inline tPortPack(core::tFrameworkElement *parent, TIterator names_begin, TIterator names_end)
  {}

  inline core::tPortWrapperBase &GetPort(size_t index)
  {
    return *reinterpret_cast<core::tPortWrapperBase *>(0);
  };

  inline bool HasChanged(size_t index)
  {
    return false;
  };

  inline void ManagedDelete()
  {}
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

#endif

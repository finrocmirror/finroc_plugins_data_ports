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
#include "core/tFrameworkElement.h"
#include "core/log_messages.h"
#include "rrlib/util/tIteratorRange.h"
#include "rrlib/util/tIntegerSequence.h"

#include "rrlib/util/tTypeList.h"

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
 * This class creates a tuple of instances of the given port template
 * inserting the given types.  It also provides methods to access the
 * included ports and their change flags at runtime.
 *
 * \param TPort    A port class template to use for every packed port
 * \param TTypes   A variadic list of the data types used in the ports
 */
template <template <typename> class TPort, typename ... TTypes>
class tPortPack
{
  using tPorts = std::tuple<std::unique_ptr<TPort<TTypes>>...>;
  using tIndex = typename rrlib::util::tIntegerSequenceGenerator<sizeof...(TTypes)>::type;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Ctor with common port name prefix
   *
   * The port names will follow the scheme "<prefix> [0..]"
   *
   * \param parent        The parent framework element
   * \param name_prefix   The common prefix used for all port names
   */
  inline tPortPack(core::tFrameworkElement *parent, const std::string &name_prefix)
  {
    this->DispatchInitializer(parent, tIndex(), [&name_prefix](size_t i)
    {
      return name_prefix + std::to_string(i);
    });
  }

  /*! Ctor with custom port names
   *
   * The port names are completely taken from two iterators
   *
   * \param parent        The parent framework element
   * \param names_begin   Begin of the list of port names
   * \param names_end     End of the list of port names
   */
  template <typename TIterator>
  inline tPortPack(core::tFrameworkElement *parent, TIterator names_begin, TIterator names_end)
  {
    auto number_of_names = std::distance(names_begin, names_end);
    if (number_of_names != sizeof...(TTypes))
    {
      FINROC_LOG_THROW(rrlib::util::tTraceableException<std::logic_error>("Number of ports names (" + std::to_string(number_of_names) + ") does not fit given number of ports (" + std::to_string(sizeof...(TTypes)) + ")"));
    }
    this->DispatchInitializer(parent, tIndex(), [names_begin](size_t i) mutable { return *names_begin++; });
  }

  /*! Ctor with custom port names (range)
   *
   * The port names are completely specified
   *
   * \param parent   The parent framework element
   * \param names    A range of port names to be used
   */
  template <typename TIterator>
  inline tPortPack(core::tFrameworkElement *parent, const rrlib::util::tIteratorRange<TIterator> &names) :
    tPortPack(parent, names.begin(), names.end())
  {}

  /*! The number of ports in this port pack
   *
   * \returns Number of ports in \a this pack
   */
  inline static constexpr size_t NumberOfPorts()
  {
    return sizeof...(TTypes);
  }

  /*! Access a specific port (strong type version, compile time)
   *
   * \param Tindex   The index of the port in this pack
   *
   * \return The specified port
   */
  template <size_t Tindex>
  inline typename std::tuple_element<Tindex, std::tuple<TPort<TTypes>...>>::type &GetPort()
  {
    static_assert(Tindex < this->NumberOfPorts(), "Port index not in range");
    return *std::get<Tindex>(this->ports);
  }

  /*! Access a specific port (runtime version)
   *
   * \param index   The index of the port in this pack
   *
   * \return The specified port
   */
  inline core::tPortWrapperBase &GetPort(size_t index)
  {
    assert(index < this->NumberOfPorts());
    return *this->port_wrappers[index];
  }

  /*! Access a specific port's changed flag
   *
   * \param index   The index of the port in this pack
   *
   * \return Whether the port has changed or not
   */
  inline bool HasChanged(size_t index)
  {
    std::vector<bool> changed_flags = this->GetChangedFlags(tIndex()); // TODO: std::vector<bool> may be replaced by auto in c++14 and gcc < 4.9 to avoid heap allocation
    return changed_flags[index];
  }

  /*! Check if any port in the pack has changed
   *
   * \return Whether any port in the pack has changed
   */
  inline bool Changed()
  {
    std::vector<bool> changed_flags = this->GetChangedFlags(tIndex()); // TODO: std::vector<bool> may be replaced by auto in c++14 and gcc < 4.9 to avoid heap allocation
    return std::any_of(changed_flags.begin(), changed_flags.end(), [](bool x)
    {
      return x;
    });
  }

  /*! Publish a tuple of value through this port pack (only for output ports)
   *
   * \param values      A tuple of values that can be published via this portspacks ports
   * \param timestamp   An optional timestamp to use (default: cNO_TIME)
   */
  template <typename ... TValues>
  inline void Publish(const std::tuple<TValues...> &values, const rrlib::time::tTimestamp timestamp = rrlib::time::cNO_TIME)
  {
    this->DispatchPublisher(tIndex(), values, timestamp);
  }

  /*! Deletion of the port pack's ports
   */
  inline void ManagedDelete()
  {
    this->ManagedDelete(tIndex());
  }

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*! Ctor with common port name prefix and port number offset to support legacy code
   *
   * The port names will follow the scheme "<prefix> [<offset>..]"
   *
   * \param parent        The parent framework element
   * \param name_prefix   The common prefix used for all port names
   * \param offset        Offset for numbering the created ports
   */
  inline tPortPack(core::tFrameworkElement *parent, const std::string &name_prefix, size_t offset)
  {
    this->DispatchInitializer(parent, tIndex(), [&name_prefix, offset](size_t i)
    {
      return name_prefix + std::to_string(i + offset);
    });
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  tPorts ports;
  core::tPortWrapperBase *port_wrappers[sizeof...(TTypes)];

  template <size_t Tindex>
  int InitializePort(core::tFrameworkElement *parent, const std::string &name)
  {
    std::get<Tindex>(this->ports).reset(new typename std::tuple_element<Tindex, std::tuple<TPort<TTypes>...>>::type(name, parent));
    std::get<Tindex>(this->ports)->Init();
    return 0;
  }

  template <int ... Tindex, typename TNameGenerator>
  void DispatchInitializer(core::tFrameworkElement *parent, rrlib::util::tIntegerSequence<Tindex...>, TNameGenerator name_generator)
  {
    __attribute__((unused)) int foo[] = { InitializePort<Tindex>(parent, name_generator(Tindex))... };
    auto port_wrappers = std::initializer_list<core::tPortWrapperBase *> {std::get<Tindex>(this->ports).get()...};
    std::copy(port_wrappers.begin(), port_wrappers.end(), this->port_wrappers);
  }

  template <typename T, typename TValue>
  int PublishPort(T &port, const TValue &value, const rrlib::time::tTimestamp &timestamp)
  {
    port.Publish(value, timestamp);
    return 0;
  }
  template <int ... Tindex, typename ... TValues>
  void DispatchPublisher(rrlib::util::tIntegerSequence<Tindex...>, const std::tuple<TValues...> &values, const rrlib::time::tTimestamp timestamp)
  {
    __attribute__((unused)) int foo[] = { PublishPort(*std::get<Tindex>(this->ports), std::get<Tindex>(values), timestamp)... };
  }

  template <int ... Tindex>
  inline std::initializer_list<bool> GetChangedFlags(rrlib::util::tIntegerSequence<Tindex...>)
  {
    return { std::get<Tindex>(this->ports)->HasChanged()..., false };
  }

  template <int ... Tindex>
  inline void ManagedDelete(rrlib::util::tIntegerSequence<Tindex...>)
  {
    for (auto p : this->port_wrappers)
    {
      p->GetWrapped()->ManagedDelete();
    }
  }

};

/*!
 * This class creates a list of instances of the given port template
 * inserting several types from a list.  It therefore creates a nested
 * structure of inherited classes and provides a method to access the
 * port on a specific layer at runtime.
 *
 * \note This class exists to support legacy code that used the initial
 *       version with tTypeList and 1-based port numbers.
 *
 * \param TPort       A port class template to use for every packed port
 * \param TTypeList   A list of the data types used in the ports. e.g. rrlib::util::tTypeList
 */
template <template <typename> class TPort, typename ... TTypes>
class tPortPack<TPort, rrlib::util::tTypeList<TTypes...>> : public tPortPack<TPort, TTypes...>
{
public:
  using tPortPack<TPort, TTypes...>::tPortPack;
  inline tPortPack(core::tFrameworkElement *parent, const std::string &name_prefix) :
    tPortPack<TPort, TTypes...>(parent, name_prefix, 1)
  {}
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

#endif

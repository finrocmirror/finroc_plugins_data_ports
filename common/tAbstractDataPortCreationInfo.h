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
/*!\file    plugins/data_ports/common/tAbstractDataPortCreationInfo.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-29
 *
 * \brief   Contains tAbstractDataPortCreationInfo
 *
 * \b tAbstractDataPortCreationInfo
 *
 * This class bundles various parameters for the creation of data ports.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__common__tAbstractDataPortCreationInfo_h__
#define __plugins__data_ports__common__tAbstractDataPortCreationInfo_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/port/tAbstractPortCreationInfo.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tUnit.h"
#include "plugins/data_ports/tBounds.h"

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
/*! Can be used to wrap lock order for tAbstractPortCreationInfo variadic template constructor */
struct tLockOrder
{
  int wrapped;

  tLockOrder(int i) : wrapped(i) {}
};

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Bundle of port creation parameters
/*!
 * This class bundles various parameters for the creation of data ports.
 *
 * Instead of providing suitable constructors for all types of sensible
 * combinations of the numerous (often optional) construction parameters,
 * there is only one constructor taking a single argument of this class.
 */
class tAbstractDataPortCreationInfo : public core::tAbstractPortCreationInfo
{

//----------------------------------------------------------------------
// Public fields and methods
//----------------------------------------------------------------------
public:

  /*! SI Unit of port. NULL for no unit = provides raw numbers */
  tUnit unit;

  /*! Input Queue size; value <= 0 means flexible size */
  int max_queue_size;

  /*! Minimum Network update interval; value < 0 => default values */
  int16_t min_net_update_interval;

  /*! config entry in config file */
  tString config_entry;

  /*!
   * Creates port creation info with default values
   * (Typically, at least flags and name should be set to something sensible)
   */
  tAbstractDataPortCreationInfo();

  /*!
   * Constructor takes variadic argument list... just any properties you want to assign to port.
   *
   * The first string is interpreted as port name, the second possibly as config entry (relevant for parameters only).
   * A framework element pointer is interpreted as parent.
   * tFrameworkElementFlag arguments are interpreted as flags.
   * int argument is interpreted as queue length.
   * tBounds<T> are port's bounds.
   * tUnit argument is port's unit.
   * int16/short argument is interpreted as minimum network update interval.
   * tAbstractPortCreationInfo argument is copied. This is only allowed as first argument.
   */
  template <typename ARG1, typename ... TArgs>
  explicit tAbstractDataPortCreationInfo(const ARG1& arg1, const TArgs&... rest) :
    unit(),
    max_queue_size(16),
    min_net_update_interval(-1),
    config_entry(),
    default_value(),
    bounds(),
    name_set(false)
  {
    ProcessFirstArg<ARG1>(arg1);
    ProcessArgs(rest...);
  }

  /*!
   * \return Have bounds for port been set?
   */
  bool BoundsSet() const
  {
    return bounds.GetSize() > 0;
  }

  /*!
   * \return Has a default value been set?
   */
  bool DefaultValueSet() const
  {
    return default_value.GetSize() > 0;
  }

  /*!
   * Derive method: Copy port creation info and change specified parameters
   * (Basically, the same arguments as in the constructor are possible)
   */
  template <typename ... TArgs>
  tAbstractDataPortCreationInfo Derive(const TArgs&... rest) const
  {
    ProcessArgs(rest...);
    return *this;
  }

  /*!
   * \return Bounds (when their exact type is not known at compile time)
   */
  const rrlib::serialization::tConstSource& GetBoundsGeneric() const
  {
    return bounds;
  }

  /*!
   * \return Bounds (when their exact type is not known at compile time)
   */
  const rrlib::serialization::tConstSource& GetDefaultGeneric() const
  {
    return default_value;
  }

  /*! Various set methods for different port properties */
  void Set(core::tFrameworkElement* parent)
  {
    this->parent = parent;
  }

  void Set(const char* c)
  {
    SetString(c);
  }

  void Set(const tString& s)
  {
    SetString(s);
  }

  void Set(int queue_size)
  {
    max_queue_size = queue_size;
    flags |= core::tFrameworkElement::tFlag::HAS_QUEUE | core::tFrameworkElement::tFlag::USES_QUEUE;
  }

  void Set(core::tFrameworkElement::tFlags flags)
  {
    this->flags |= flags;
  }

  void Set(short min_net_update_interval)
  {
    this->min_net_update_interval = min_net_update_interval;
  }

  void Set(const tUnit& unit)
  {
    this->unit = unit;
  }

  void Set(const tLockOrder& lo)
  {
    this->lock_order = lo.wrapped;
  }

  void Set(const rrlib::rtti::tType& dt)
  {
    this->data_type = dt;
  }

  /*!
   * Set bounds when type is not known at compile time
   *
   * \param min Minimum value
   * \param max Maximum value
   * \param out_of_bounds_action How to proceed if an incoming value is out of bounds
   */
  void SetBoundsGeneric(const rrlib::rtti::tGenericObject& min, const rrlib::rtti::tGenericObject& max,
                        tOutOfBoundsAction out_of_bounds_action = tOutOfBoundsAction::ADJUST_TO_RANGE)
  {
    rrlib::serialization::tOutputStream os(&bounds);
    // critical: needs to be the same serialization as used in tBounds
    os << min << max << out_of_bounds_action;
  }

  void SetDefaultGeneric(const rrlib::rtti::tGenericObject& default_val)
  {
    rrlib::serialization::tOutputStream os(&default_value);
    os << default_val;
  }

  /*!
   * Set Port flag
   *
   * \param flag Flag to set
   * \param value Value to set flag to
   */
  void SetFlag(uint flag, bool value);

  /*!
   * Removes default value from port creation info
   */
  void UnsetDefaultValue()
  {
    default_value.Clear();
  }

//----------------------------------------------------------------------
// Protected classes and fields (used by derived class tPortCreationInfo also)
//----------------------------------------------------------------------
protected:

  /*!
   * Class to store default values and bounds of arbitrary types (in serialized form)
   */
  template <size_t INITIAL_SIZE>
  class tStorage : public rrlib::serialization::tStackMemoryBuffer<INITIAL_SIZE>
  {
  public:
    tStorage() :
      rrlib::serialization::tStackMemoryBuffer<INITIAL_SIZE>(5, true)
    {}

    tStorage(const tStorage& o)
    {
      this->CopyFrom(o);
    }

    tStorage& operator=(const tStorage& o)
    {
      this->CopyFrom(o);
      return *this;
    }
  };

  /*! Storage for default value */
  tStorage<150> default_value;

  /*! Storage for bounds */
  tStorage<300> bounds;

  /*! Has name been set? (we do not check name for zero length, because ports without names may be created) */
  bool name_set;


  /*!
   * Processes next string argument
   */
  void SetString(const tString& s);

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*!
   * Processes first argument (only here tPortCreationInfoBase argument is allowed)
   */
  template <typename A>
  void ProcessFirstArg(const typename std::enable_if<std::is_base_of<tAbstractPortCreationInfo, A>::value, A>::type& a)
  {
    (*this) = a;
  }

  template <typename A>
  void ProcessFirstArg(const typename std::enable_if < !std::is_base_of<tAbstractPortCreationInfo, A>::value, A >::type& a)
  {
    ProcessArg<A>(a);
  }

  /*! Process constructor arguments */
  void ProcessArgs() {}

  template <typename A, typename ... ARest>
  void ProcessArgs(const A& arg, const ARest&... args)
  {
    ProcessArg<A>(arg);
    ProcessArgs(args...);
  }

  /*! Process single constructor argument */
  template <typename A>
  void ProcessArg(const A& arg)
  {
    // standard case
    Set(arg);
  }

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

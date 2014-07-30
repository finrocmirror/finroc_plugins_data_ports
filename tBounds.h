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
/*!\file    plugins/data_ports/tBounds.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-01
 *
 * \brief   Contains tBounds
 *
 * \b tBounds
 *
 * Information about bounds.
 * This is class is used, for instance, to restrict values in ports to a certain range.
 *
 * This class is suitable for any Type T that has an empty constructor a
 * copy constructor and a smaller-than operator.
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tBounds_h__
#define __plugins__data_ports__tBounds_h__

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

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

/*! How to proceed if an incoming value is out of bounds */
enum class tOutOfBoundsAction
{
  DISCARD,          //!< Discard incoming values that are out of bounds
  ADJUST_TO_RANGE,  //!< Adjust incoming values to bounds before assigning them
  APPLY_DEFAULT     //!< Apply default on incoming value out of bounds
};

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Bounds
/*!
 * Information about bounds.
 * This is class is used, for instance, to restrict values in ports to a certain range.
 *
 * This class is suitable for any Type T that has an empty constructor a
 * copy constructor and a smaller-than operator.
 */
template <typename T>
class tBounds
{
  static_assert(IsBoundable<T>::value, "Type T is not boundable");

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param min Minimum bound
   * \param max Maximum bound
   * \param out_of_bounds_action How to proceed if an incoming value is out of bounds
   */
  template <typename TMin, typename TMax>
  tBounds(TMin min, TMax max, tOutOfBoundsAction out_of_bounds_action = tOutOfBoundsAction::ADJUST_TO_RANGE) :
    min(min), max(max),
    action(out_of_bounds_action),
    out_of_bounds_default()
  {
    out_of_bounds_default = ToBounds(out_of_bounds_default);
  }

  /*!
   * \param min Minimum bound
   * \param max Maximum bound
   * \param out_of_bounds_default Default value to apply when value is out of bounds
   */
  template <typename TMin, typename TMax>
  tBounds(TMin min, TMax max, T out_of_bounds_default) :
    min(min), max(max),
    action(tOutOfBoundsAction::APPLY_DEFAULT),
    out_of_bounds_default(out_of_bounds_default)
  {
  }

  /*!
   * \return Maximum value
   */
  inline T GetMax() const
  {
    return max;
  }

  /*!
   * \return Minimum value
   */
  inline T GetMin() const
  {
    return min;
  }

  /*!
   * \return Default value when value is out of bounds
   */
  inline T GetOutOfBoundsDefault() const
  {
    return out_of_bounds_default;
  }

  /*!
   * \return Action to perform when value is out of range
   */
  inline tOutOfBoundsAction GetOutOfBoundsAction() const
  {
    return action;
  }

  /*!
   * Does value lie within bounds ?
   *
   * \param val Value
   * \return Answer
   */
  inline bool InBounds(const T& val) const
  {
    return (!(val < min)) && (!(max < val));
  }

  /*!
   * \param value Value to adjust to range
   * \return Adjusted value
   */
  inline T ToBounds(const T& value) const
  {
    if (value < min)
    {
      return min;
    }
    else if (max < value)
    {
      return max;
    }
    return value;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Minimum and maximum bounds */
  T min, max;

  /*! Action to perform when value is out of range */
  tOutOfBoundsAction action;

  /*! Default value when value is out of bounds */
  T out_of_bounds_default;
};

template <typename T>
inline rrlib::serialization::tOutputStream& operator<<(rrlib::serialization::tOutputStream& stream, const tBounds<T>& bounds)
{
  stream << bounds.GetMin() << bounds.GetMax() << bounds.GetOutOfBoundsAction();
  if (bounds.GetOutOfBoundsAction() == tOutOfBoundsAction::APPLY_DEFAULT)
  {
    stream << bounds.GetOutOfBoundsDefault();
  }
  return stream;
}

template <typename T>
inline rrlib::serialization::tInputStream& operator>>(rrlib::serialization::tInputStream& stream, tBounds<T>& bounds)
{
  T min, max, ood;
  tOutOfBoundsAction action;
  stream >> min >> max >> action;
  if (action == tOutOfBoundsAction::APPLY_DEFAULT)
  {
    stream >> ood;
    bounds = tBounds<T>(min, max, ood);
  }
  else
  {
    bounds = tBounds<T>(min, max, action);
  }
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

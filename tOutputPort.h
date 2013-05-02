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
/*!\file    plugins/data_ports/tOutputPort.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-25
 *
 * \brief   Contains tOutputPort
 *
 * \b tOutputPort
 *
 * This port class is used in applications.
 * It provides a convenient API for the type-less port implementation classes.
 * Derived from tPort, this class provides some additional functions that
 * are only relevant for output ports.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tOutputPort_h__
#define __plugins__data_ports__tOutputPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tPort.h"
#include "plugins/data_ports/tPullRequestHandler.h"

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
//! Output port
/*!
 * This port class is used in applications.
 * It provides a convenient API for the type-less port implementation classes.
 * Derived from tPort, this class provides some additional functions that
 * are only relevant for output ports.
 *
 * \tparam T T is the data type of the port (see tPort)
 */
template <typename T>
class tOutputPort : public tPort<T>
{
  typedef typename tPort<T>::tImplementation tImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Creates no wrapped port */
  tOutputPort() {}

  /*!
   * Constructor takes variadic argument list... just any properties you want to assign to port.
   *
   * The first string is interpreted as port name, the second possibly as config entry (relevant for parameters only).
   * A framework element pointer is interpreted as parent.
   * tFrameworkElement::tFlags arguments are interpreted as flags.
   * int argument is interpreted as queue length.
   * tBounds<T> are port's bounds.
   * tUnit argument is port's unit.
   * int16/short argument is interpreted as minimum network update interval.
   * const T& is interpreted as port's default value.
   * tPortCreationInfo<T> argument is copied. This is only allowed as first argument.
   *
   * This becomes a little tricky when port has numeric or string type.
   * There we have these rules:
   *
   * string type: The second string argument is interpreted as default_value. The third as config entry.
   * numeric type: The first numeric argument is interpreted as default_value.
   */
  template <typename ... ARGS>
  tOutputPort(const ARGS& ... args) :
    tPort<T>(args..., core::tFrameworkElement::tFlag::EMITS_DATA | core::tFrameworkElement::tFlag::OUTPUT_PORT)
  {}

  /*!
   * \return Unused buffer of type T.
   * Buffers to be published using this port should be acquired using this function.
   * The buffer might contain old data, so it should be cleared prior to using.
   *
   * Using this method with 'cheaply copied' types is not required and slightly less
   * efficient than publishing values directly.
   */
  inline tPortDataPointer<T> GetUnusedBuffer()
  {
    return tImplementation::GetUnusedBuffer(*this->GetWrapped());
  }

  /*!
   * Publish Data Buffer. This data will be forwarded to any connected ports.
   *
   * (This pass-by-value Publish()-variant is efficient when using 'cheaply copied' types,
   *  but can be computationally expensive with large data types)
   *
   * \param data Data to publish. It will be deep-copied.
   * \param timestamp Timestamp for attached data (optional)
   */
  inline void Publish(const T& data, const rrlib::time::tTimestamp& timestamp = rrlib::time::cNO_TIME)
  {
    tImplementation::CopyAndPublish(*this->GetWrapped(), data, timestamp);
  }

  /*!
   * Publish Data Buffer. This data will be forwarded to any connected ports.
   * It should not be modified thereafter (tPortDataPointer will be reset).
   *
   * \param data Data buffer acquired from a port using getUnusedBuffer (or locked data received from another port)
   */
  inline void Publish(tPortDataPointer<T> && data)
  {
    tImplementation::Publish(*this->GetWrapped(), std::forward<tPortDataPointer<T>>(data));
  }
  inline void Publish(tPortDataPointer<T> & data)
  {
    tImplementation::Publish(*this->GetWrapped(), data);
  }
  inline void Publish(tPortDataPointer<const T> && data)
  {
    tImplementation::PublishConstBuffer(*this->GetWrapped(), std::forward<tPortDataPointer<const T>>(data));
  }
  inline void Publish(tPortDataPointer<const T> & data)
  {
    tImplementation::PublishConstBuffer(*this->GetWrapped(), data);
  }

  /*!
   * \return Is data from this port pushed or pulled?
   */
  inline bool PushStrategy() const
  {
    return this->GetWrapped()->PushStrategy();
  }

  /*!
   * \return Is data to this port pushed in reverse direction?
   */
  inline bool ReversePushStrategy() const
  {
    return this->GetWrapped()->ReversePushStrategy();
  }

  /*!
   * \param pull_request_handler Object that handles any incoming pull requests - null if there is none (typical case)
   */
  void SetPullRequestHandler(tPullRequestHandler<T>* pull_request_handler)
  {
    this->GetWrapped()->SetPullRequestHandler(pull_request_handler);
  }
  void SetPullRequestHandler(tPullRequestHandler<rrlib::rtti::tGenericObject>& pull_request_handler)
  {
    this->GetWrapped()->SetPullRequestHandler(&pull_request_handler);
  }

  /*!
   * Set whether data should be pushed or pulled in reverse direction
   *
   * \param push Push data?
   */
  inline void SetReversePushStrategy(bool push)
  {
    this->GetWrapped()->SetReversePushStrategy(push);
  }

  /*!
   * Wraps raw port
   * Throws std::runtime_error if port to wrap has invalid flags.
   *
   * \param wrap Type-less port to wrap as tPort<T>
   */
  static tOutputPort Wrap(core::tAbstractPort& wrap)
  {
    tPort<T>::Wrap(wrap); // checks types
    if (!(wrap.GetFlag(core::tFrameworkElement::tFlag::EMITS_DATA) && wrap.GetFlag(core::tFrameworkElement::tFlag::OUTPUT_PORT)))
    {
      std::runtime_error("tOutputPort can only wrap output ports. Attempted to wrap port that has invalid flags.");
    }
    tOutputPort port;
    port.SetWrapped(&wrap);
    return port;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif

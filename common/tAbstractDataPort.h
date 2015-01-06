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
/*!\file    plugins/data_ports/common/tAbstractDataPort.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-28
 *
 * \brief   Contains tAbstractDataPort
 *
 * \b tAbstractDataPort
 *
 * Abstract base class for all data port implementations.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__common__tAbstractDataPort_h__
#define __plugins__data_ports__common__tAbstractDataPort_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/port/tAbstractPort.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/definitions.h"
#include "plugins/data_ports/common/tAbstractDataPortCreationInfo.h"
#include "plugins/data_ports/common/tPortListenerRaw.h"

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

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Data port implementation base class
/*!
 * Abstract base class for all data port implementations.
 */
class tAbstractDataPort : public core::tAbstractPort
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tAbstractDataPort(const tAbstractDataPortCreationInfo& create_info);

  /*!
   * Set current value to default value
   */
  virtual void ApplyDefaultValue() = 0;

  /*!
   * Forwards current data to specified port (publishes the data via this port)
   *
   * \param other Port to which data is forwarded
   */
  virtual void ForwardData(tAbstractDataPort& other) = 0;

  /*!
   * \return Changed "flag" (has two different values for ordinary and initial data)
   */
  inline tChangeStatus GetChangedFlag() const
  {
    return static_cast<tChangeStatus>(changed.load());
  }

  /*!
   * \return Has port changed since last reset? (Flag for use by custom API - not used/accessed by core port classes.)
   */
  tChangeStatus GetCustomChangedFlag() const
  {
    return custom_changed_flag;
  }

  /*!
   * \return Maximum queue length
   */
  int GetMaxQueueLength() const
  {
    return GetMaxQueueLengthImplementation();
  }

  /*!
   * \return Minimum Network Update Interval (only-port specific one; -1 if there's no specific setting for port)
   */
  inline rrlib::time::tDuration GetMinNetUpdateInterval() const
  {
    return std::chrono::milliseconds(min_net_update_time);
  }

  /*!
   * \return Minimum Network Update Interval (only-port specific one; -1 if there's no specific setting for port)
   */
  inline int16_t GetMinNetUpdateIntervalRaw() const
  {
    return min_net_update_time;
  }

  /*!
   * (Helper function for network functions)
   * Look for minimal port-specific minimal network update interval
   * at all connected ports.
   *
   * \return result - -1 if no port has specific setting
   */
  int16_t GetMinNetworkUpdateIntervalForSubscription() const;

  /*!
   * \param listener Listener to remove
   */
  inline common::tPortListenerRaw* GetPortListener()
  {
    return port_listener;
  }

  /*!
   * \return Strategy to use, when this port is target
   */
  inline int16_t GetStrategy() const
  {
    assert((strategy >= -1));
    return strategy;
  }

  /*!
   * (relevant for input ports only)
   *
   * \return Has port changed since last reset?
   */
  inline bool HasChanged() const
  {
    return changed != static_cast<int8_t>(tChangeStatus::NO_CHANGE);
  }

  /*!
   * \return Is data to this port pushed or pulled?
   */
  inline bool PushStrategy() const
  {
    return GetStrategy() > 0;
  }

  /*!
   * (relevant for input ports only)
   *
   * Reset changed flag.
   */
  inline void ResetChanged()
  {
    changed = static_cast<int8_t>(tChangeStatus::NO_CHANGE);
  }

  /*!
   * \return Is data to this port pushed or pulled (in reverse direction)?
   */
  inline bool ReversePushStrategy() const
  {
    return GetFlag(tFlag::PUSH_STRATEGY_REVERSE);
  }

  /*!
   * \param new_value New value for custom changed flag (for use by custom API - not used/accessed by core port classes.)
   */
  void SetCustomChangedFlag(tChangeStatus new_value)
  {
    custom_changed_flag = new_value;
  }

  /*!
   * \param interval Minimum Network Update Interval
   */
  void SetMinNetUpdateInterval(rrlib::time::tDuration& interval);
  void SetMinNetUpdateIntervalRaw(int16_t interval);

  /*!
   * \param listener New ports Listener
   *
   * (warning: this will not delete the old listener)
   */
  inline void SetPortListener(common::tPortListenerRaw* listener)
  {
    port_listener = listener;
  }

  /*!
   * Set whether data should be pushed or pulled
   *
   * \param push Push data?
   */
  void SetPushStrategy(bool push);

  /*!
   * Set whether data should be pushed or pulled in reverse direction
   *
   * \param push Push data?
   */
  void SetReversePushStrategy(bool push);

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  virtual ~tAbstractDataPort();

  /*!
   * Propagates max target queue length to sources
   * (call on target with new connections)
   *
   * \param push_wanter Port that "wants" an initial push and from whom this call originates - null if there's no port that wants as push
   * \param new_connection_partner When a new connection is created - The new port that is connected to this (target) port
   * \return Did Strategy for this port change?
   */
  virtual bool PropagateStrategy(tAbstractDataPort* push_wanter, tAbstractDataPort* new_connection_partner);

  /*!
   * (relevant for input ports only)
   *
   * \value Value to set change flag to
   */
  inline void SetChanged(tChangeStatus value)
  {
    changed = static_cast<int8_t>(value);
  }

  /*!
   * Update edge statistics
   *
   * \param source Source port
   * \param target Target port
   * \param data Data that was sent
   */
  void UpdateEdgeStatistics(tAbstractPort& source, tAbstractPort& target, rrlib::rtti::tGenericObject& data);

  /*!
   * Does this port "want" to receive a value via push strategy?
   *
   * \param cReverse direction? (typically we push forward)
   * \param change_constant If this is about an initial push, this should be CHANGED_INITIAL - otherwise CHANGED
   * \return Answer
   *
   * Typically it does, unless it has multiple sources or no push strategy itself.
   * (Standard implementation for this)
   */
  template <bool cREVERSE, tChangeStatus cCHANGE_CONSTANT>
  inline bool WantsPush() const
  {
    // The compiler should optimize branches away
    if (cREVERSE)
    {
      if (cCHANGE_CONSTANT == tChangeStatus::CHANGED_INITIAL)
      {
        return GetFlag(tFlag::PUSH_STRATEGY_REVERSE) && CountOutgoingConnections() <= 1;
      }
      else
      {
        return GetFlag(tFlag::PUSH_STRATEGY_REVERSE);
      }
    }
    else if (cCHANGE_CONSTANT == tChangeStatus::CHANGED_INITIAL)
    {
      // We don't want initial pushes to ports with multiple inputs
      return strategy > 0 && CountIncomingConnections() <= 1;
    }
    else
    {
      return strategy > 0;
    }
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Has port changed since last reset? (see constants above) */
  std::atomic<int8_t> changed;

  /*!
   * Has port changed since last reset? Flag for use by custom API - not used/accessed by core port classes.
   * Defined here, because it shouldn't require any more memory due to alignment.
   * Alternative would be letting the API allocate an extra memory block per port, just to store this.
   */
  tChangeStatus custom_changed_flag;

  /*!
   * Strategy to use, when this port is target
   * -1:     not connected at all
   * 0:      pull strategy
   * n >= 1: push strategy for queue with n elements (queue length makes no difference locally, but network ports need to buffer this amount of elements)
   */
  int16_t strategy;

  /*! Minimum network update interval. Value < 0 means default for this type */
  int16_t min_net_update_time;

  /*! Listener(s) of port value changes */
  common::tPortListenerRaw* port_listener;


  /*!
   * Make some auto-adjustments to port creation info in constructor
   *
   * \param create_info Orifinal port creation info
   * \return Modified port creation info for base class
   */
  static core::tAbstractPortCreationInfo AdjustPortCreationInfo(const tAbstractDataPortCreationInfo& create_info);

  /*!
   * Should be called in situations where there might need to be an initial push
   * (e.g. connecting or strategy change)
   *
   * \param target Potential Target port
   */
  void ConsiderInitialReversePush(tAbstractDataPort& target);

  /*!
   * Forward current strategy to source ports (helper for above - and possibly variations of above)
   *
   * \param strategy New Strategy of this port
   * \param push_wanter Port that "wants" an initial push and from whom this call originates - null if there's no port that wants as push
   */
  void ForwardStrategy(int16_t strategy, tAbstractDataPort* push_wanter);

  /*!
   * \return Maximum queue length
   */
  virtual int GetMaxQueueLengthImplementation() const = 0;

  /*!
   * \return Returns minimum strategy requirement (for this port in isolation) - typically 0 for non-input-ports
   * (Called in runtime-registry synchronized context only)
   */
  virtual int16_t GetStrategyRequirement() const;

  /*!
   * Push initial value to the specified port
   * (checks etc. have been done by AbstractDataPort class)
   *
   * \param target Port to push data to
   * \param reverse Is this a reverse push?
   */
  virtual void InitialPushTo(tAbstractPort& target, bool reverse) = 0;

  virtual void OnConnect(tAbstractPort& partner, bool partner_is_destination) override;

  virtual void OnDisconnect(tAbstractPort& partner, bool partner_is_destination) override;

  virtual void OnNetworkConnectionLoss() override;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#endif

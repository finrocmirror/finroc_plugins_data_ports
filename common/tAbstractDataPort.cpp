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
/*!\file    plugins/data_ports/common/tAbstractDataPort.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-28
 *
 */
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tAbstractDataPort.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/port/tEdgeAggregator.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/type_traits.h"

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
//----------------------------------------------------------------------

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
// Const values
//----------------------------------------------------------------------
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
constexpr rrlib::time::tDuration tAbstractDataPort::cPULL_TIMEOUT;
#else
rrlib::time::tDuration tAbstractDataPort::cPULL_TIMEOUT = std::chrono::seconds(1);
#endif

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tAbstractDataPort::tAbstractDataPort(const tAbstractDataPortCreationInfo& create_info) :
  core::tAbstractPort(AdjustPortCreationInfo(create_info)),
  changed(static_cast<int8_t>(tChangeStatus::NO_CHANGE)),
  custom_changed_flag(tChangeStatus::NO_CHANGE),
  strategy(-1),
  min_net_update_time(create_info.min_net_update_interval)
{
  printf("Data Port %p\n", this);
}

tAbstractDataPort::~tAbstractDataPort()
{}

core::tAbstractPortCreationInfo tAbstractDataPort::AdjustPortCreationInfo(const tAbstractDataPortCreationInfo& create_info)
{
  core::tAbstractPortCreationInfo result = create_info;
  assert(result.data_type != NULL);
  if (IsCheaplyCopiedType(result.data_type))
  {
    // no priority flag set...if "cheaply copyable type" set to EXPRESS_PORT, because it does not hurt
    result.flags |= tFlag::EXPRESS_PORT;
  }
  return result;
}

void tAbstractDataPort::ConnectionAdded(tAbstractPort& partner, bool partner_is_destination)
{
  if (!dynamic_cast<tAbstractDataPort*>(&partner))
  {
    FINROC_LOG_PRINT(ERROR, "Invalid port was connected");
    abort();
  }
  if (partner_is_destination)
  {
    (static_cast<tAbstractDataPort&>(partner)).PropagateStrategy(NULL, this);

    // check whether we need an initial reverse push
    this->ConsiderInitialReversePush(static_cast<tAbstractDataPort&>(partner));
  }
}

void tAbstractDataPort::ConnectionRemoved(tAbstractPort& partner, bool partner_is_destination)
{
  if (partner_is_destination)
  {
    if (!this->IsConnected())
    {
      this->strategy = -1;
    }
    if (!partner.IsConnected())
    {
      static_cast<tAbstractDataPort&>(partner).strategy = -1;
    }

    static_cast<tAbstractDataPort&>(partner).PropagateStrategy(NULL, NULL);
    this->PropagateStrategy(NULL, NULL);
  }
}

void tAbstractDataPort::ConsiderInitialReversePush(tAbstractDataPort& target)
{
  if (IsReady() && target.IsReady())
  {
    if (ReversePushStrategy() && CountOutgoingConnections() == 1)
    {
      FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "Performing initial reverse push from ", target.GetQualifiedName(), " to ", GetQualifiedName());
      target.InitialPushTo(*this, true);
    }
  }
}

void tAbstractDataPort::ForwardStrategy(int16_t strategy2, tAbstractDataPort* push_wanter)
{
  for (auto it = IncomingConnectionsBegin(); it != IncomingConnectionsEnd(); ++it)
  {
    tAbstractDataPort& port = static_cast<tAbstractDataPort&>(*it);
    if (push_wanter || port.GetStrategy() != strategy2)
    {
      port.PropagateStrategy(push_wanter, NULL);
    }
  }
}

int16_t tAbstractDataPort::GetMinNetworkUpdateIntervalForSubscription() const
{
  int16_t result = std::numeric_limits<short>::max();
  int16_t t = 0;

  for (auto it = OutgoingConnectionsBegin(); it != OutgoingConnectionsEnd(); ++it)
  {
    tAbstractDataPort& port = static_cast<tAbstractDataPort&>(*it);
    if (port.GetStrategy() > 0)
    {
      if ((t = port.min_net_update_time) >= 0 && t < result)
      {
        result = t;
      }
    }
  }
  for (auto it = IncomingConnectionsBegin(); it != IncomingConnectionsEnd(); ++it)
  {
    tAbstractDataPort& port = static_cast<tAbstractDataPort&>(*it);
    if (port.GetFlag(tFlag::PUSH_STRATEGY_REVERSE))
    {
      if ((t = port.min_net_update_time) >= 0 && t < result)
      {
        result = t;
      }
    }
  }

  return result == std::numeric_limits<short>::max() ? -1 : result;
}

int16_t tAbstractDataPort::GetStrategyRequirement() const
{
  if (IsInputPort())
  {
    if (GetFlag(tFlag::PUSH_STRATEGY))
    {
      if (GetFlag(tFlag::USES_QUEUE))
      {
        int qlen = GetMaxQueueLength();
        return static_cast<int16_t>((qlen > 0 ? std::min<int>(qlen, std::numeric_limits<int16_t>::max()) : std::numeric_limits<int16_t>::max()));
      }
      else
      {
        return 1;
      }
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return static_cast<int16_t>((IsConnected() ? 0 : -1));
  }
}

bool tAbstractDataPort::PropagateStrategy(tAbstractDataPort* push_wanter, tAbstractDataPort* new_connection_partner)
{
  tLock lock(GetStructureMutex());

  // step1: determine max queue length (strategy) for this port
  int16_t max = static_cast<int16_t>(std::min(GetStrategyRequirement(), std::numeric_limits<short>::max()));
  for (auto it = OutgoingConnectionsBegin(); it != OutgoingConnectionsEnd(); ++it)
  {
    tAbstractDataPort& port = static_cast<tAbstractDataPort&>(*it);
    max = static_cast<int16_t>(std::max(max, port.GetStrategy()));
  }

  // has max length (strategy) for this port changed? => propagate to sources
  bool change = (max != strategy);

  // if origin wants a push - and we are a "source" port - provide this push (otherwise - "push wish" should be propagated further)
  if (push_wanter)
  {
    bool source_port = (strategy >= 1 && max >= 1) || (!HasIncomingConnections());
    if (!source_port)
    {
      bool all_sources_reverse_pushers = true;
      for (auto it = IncomingConnectionsBegin(); it != IncomingConnectionsEnd(); ++it)
      {
        tAbstractDataPort& port = static_cast<tAbstractDataPort&>(*it);
        if (port.IsReady() && (!port.ReversePushStrategy()))
        {
          all_sources_reverse_pushers = false;
          break;
        }
      }
      source_port = all_sources_reverse_pushers;
    }
    if (source_port)
    {
      if (IsReady() && push_wanter->IsReady() && (!GetFlag(tFlag::NO_INITIAL_PUSHING)) && (!push_wanter->GetFlag(tFlag::NO_INITIAL_PUSHING)))
      {
        FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "Performing initial push from ", GetQualifiedName(), " to ", push_wanter->GetQualifiedName());
        InitialPushTo(*push_wanter, false);
      }
      push_wanter = NULL;
    }
  }

  // okay... do we wish to receive a push?
  // yes if...
  //  1) we are target of a new connection, have a push strategy, no other sources, and partner is no reverse push source
  //  2) our strategy changed to push, and exactly one source
  int other_sources = 0;
  for (auto it = IncomingConnectionsBegin(); it != IncomingConnectionsEnd(); ++it)
  {
    if (it->IsReady() && &(*it) != new_connection_partner)
    {
      other_sources++;
    }
  }
  bool request_push = (new_connection_partner && (max >= 1) && (other_sources == 0) && (!new_connection_partner->ReversePushStrategy())) || ((max >= 1 && strategy < 1) && (other_sources == 1));

  // register strategy change
  if (change)
  {
    strategy = max;
  }

  ForwardStrategy(strategy, request_push ? this : NULL);  // forward strategy... do it anyway, since new ports may have been connected

  if (change)    // do this last to ensure that all relevant strategies have been set, before any network updates occur
  {
    PublishUpdatedInfo(core::tRuntimeListener::tEvent::CHANGE);
  }

  return change;
}

void tAbstractDataPort::SetMinNetUpdateInterval(rrlib::time::tDuration& new_interval)
{
  tLock lock(GetStructureMutex());
  int64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(new_interval).count();
  int16_t interval = std::min<int64_t>(ms < 0 ? 0 : ms, std::numeric_limits<int16_t>::max()); // adjust value to valid range
  if (min_net_update_time != interval)
  {
    min_net_update_time = interval;
    PublishUpdatedInfo(core::tRuntimeListener::tEvent::CHANGE);
  }
}

void tAbstractDataPort::SetPushStrategy(bool push)
{
  tLock lock(GetStructureMutex());
  if (push == GetFlag(tFlag::PUSH_STRATEGY))
  {
    return;
  }
  SetFlag(tFlag::PUSH_STRATEGY, push);
  PropagateStrategy(NULL, NULL);
}

void tAbstractDataPort::SetReversePushStrategy(bool push)
{
  if (push == GetFlag(tFlag::PUSH_STRATEGY_REVERSE))
  {
    return;
  }

  tLock lock(GetStructureMutex());
  SetFlag(tFlag::PUSH_STRATEGY_REVERSE, push);
  if (push && IsReady())    // strategy change
  {
    for (auto it = OutgoingConnectionsBegin(); it != OutgoingConnectionsEnd(); ++it)
    {
      tAbstractDataPort& port = static_cast<tAbstractDataPort&>(*it);
      if (port.IsReady())
      {
        FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "Performing initial reverse push from ", port.GetQualifiedName(), " to ", GetQualifiedName());
        port.InitialPushTo(*this, true);
        break;
      }
    }
  }
  this->PublishUpdatedInfo(core::tRuntimeListener::tEvent::CHANGE);
}

void tAbstractDataPort::UpdateEdgeStatistics(tAbstractPort& source, tAbstractPort& target, rrlib::rtti::tGenericObject& data)
{
  core::tEdgeAggregator::UpdateEdgeStatistics(source, target, data.GetType().GetSize() /* TODO: This is no accurate size estimation for types that allocate memory internally */);
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

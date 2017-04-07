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
#include "plugins/data_ports/common/tConversionConnector.h"

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
typedef rrlib::thread::tLock tLock;

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tAbstractDataPort::tAbstractDataPort(const tAbstractDataPortCreationInfo& create_info) :
  core::tAbstractPort(AdjustPortCreationInfo(create_info)),
  changed(static_cast<int8_t>(tChangeStatus::CHANGED_INITIAL)),
  custom_changed_flag(tChangeStatus::CHANGED_INITIAL),
  strategy(-1),
  port_listener(nullptr)
{
}

tAbstractDataPort::~tAbstractDataPort()
{
  if (port_listener)
  {
    port_listener->PortDeleted();
  }
}

core::tAbstractPortCreationInfo tAbstractDataPort::AdjustPortCreationInfo(const tAbstractDataPortCreationInfo& create_info)
{
  core::tAbstractPortCreationInfo result = create_info;
  assert(result.data_type);
  if (IsCheaplyCopiedType(result.data_type))
  {
    // no priority flag set...if "cheaply copyable type" set to EXPRESS_PORT, because it does not hurt
    result.flags |= tFlag::EXPRESS_PORT;
  }
  return result;
}

void tAbstractDataPort::ConsiderInitialReversePush(tAbstractDataPort& target)
{
  if (IsReady() && target.IsReady())
  {
    if (ReversePushStrategy() && CountOutgoingConnections() == 1)
    {
      FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "Performing initial reverse push from ", target, " to ", (*this));
      target.InitialPushTo(*this, true);
    }
  }
}

core::tConnector* tAbstractDataPort::CreateConnector(tAbstractPort& destination, const core::tConnectOptions& connect_options)
{
  if (connect_options.conversion_operations.Size() == 0 && this->GetDataType() == destination.GetDataType())
  {
    return tAbstractPort::CreateConnector(destination, connect_options);
  }
  return new tConversionConnector(*this, destination, connect_options);
}

void tAbstractDataPort::ForwardStrategy(int16_t strategy2, tAbstractDataPort* push_wanter)
{
  for (auto it = IncomingConnectionsBegin(); it != IncomingConnectionsEnd(); ++it)
  {
    tAbstractDataPort& port = static_cast<tAbstractDataPort&>(it->Source());
    if (push_wanter || port.GetStrategy() != strategy2)
    {
      port.PropagateStrategy(push_wanter, nullptr);
    }
  }
}

int16_t tAbstractDataPort::GetStrategyRequirement() const
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
    return static_cast<int16_t>((IsInputPort() || IsConnected() ? 0 : -1));
  }
}

void tAbstractDataPort::OnConnect(tAbstractPort& partner, bool partner_is_destination)
{
  if (!dynamic_cast<tAbstractDataPort*>(&partner))
  {
    FINROC_LOG_PRINT(ERROR, "Invalid port was connected");
    abort();
  }
  if (partner_is_destination)
  {
    (static_cast<tAbstractDataPort&>(partner)).PropagateStrategy(nullptr, this);

    // check whether we need an initial reverse push
    this->ConsiderInitialReversePush(static_cast<tAbstractDataPort&>(partner));
  }
}

void tAbstractDataPort::OnDisconnect(tAbstractPort& partner, bool partner_is_destination)
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

    static_cast<tAbstractDataPort&>(partner).PropagateStrategy(nullptr, nullptr);
    this->PropagateStrategy(nullptr, nullptr);
  }
  else
  {
    this->OnNetworkConnectionLoss();
  }
}

void tAbstractDataPort::OnNetworkConnectionLoss()
{
  if (GetFlag(tFlag::DEFAULT_ON_DISCONNECT))
  {
    ApplyDefaultValue();
  }
}

bool tAbstractDataPort::PropagateStrategy(tAbstractDataPort* push_wanter, tAbstractDataPort* new_connection_partner)
{
  tLock lock(GetStructureMutex());

  // step1: determine max queue length (strategy) for this port
  int16_t max = static_cast<int16_t>(std::min(GetStrategyRequirement(), std::numeric_limits<short>::max()));
  for (auto it = OutgoingConnectionsBegin(); it != OutgoingConnectionsEnd(); ++it)
  {
    tAbstractDataPort& port = static_cast<tAbstractDataPort&>(it->Destination());
    max = static_cast<int16_t>(std::max(max, port.GetStrategy()));
  }
  if (GetFlag(tFlag::HIJACKED_PORT))
  {
    max = -1;
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
        tAbstractDataPort& port = static_cast<tAbstractDataPort&>(it->Source());
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
        FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "Performing initial push from ", (*this), " to ", (*push_wanter));
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
    if (it->Source().IsReady() && &(it->Source()) != new_connection_partner)
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

void tAbstractDataPort::SetHijacked(bool hijacked)
{
  tLock lock(GetStructureMutex());
  if (hijacked == GetFlag(tFlag::HIJACKED_PORT))
  {
    return;
  }
  SetFlag(tFlag::HIJACKED_PORT, hijacked);
  PropagateStrategy(nullptr, nullptr);
}

void tAbstractDataPort::SetPushStrategy(bool push)
{
  tLock lock(GetStructureMutex());
  if (push == GetFlag(tFlag::PUSH_STRATEGY))
  {
    return;
  }
  SetFlag(tFlag::PUSH_STRATEGY, push);
  PropagateStrategy(nullptr, nullptr);
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
      tAbstractDataPort& port = static_cast<tAbstractDataPort&>(it->Destination());
      if (port.IsReady())
      {
        FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "Performing initial reverse push from ", port, " to ", (*this));
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

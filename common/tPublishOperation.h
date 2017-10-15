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
/*!\file    plugins/data_ports/common/tPublishOperation.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-04
 *
 * \brief   Contains tPublishOperation
 *
 * \b tPublishOperation
 *
 * Implements data buffer publishing for all data port implementations
 * in a generic and efficient way.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__common__tPublishOperation_h__
#define __plugins__data_ports__common__tPublishOperation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tConversionConnector.h"

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

// Some raw flag combinations (defined as constants to get maximum performance - also in debug mode)
enum { cRAW_FLAGS_READY_AND_HIJACKED = (core::tFrameworkElementFlag::READY | core::tFrameworkElementFlag::HIJACKED_PORT).Raw() };
enum { cRAW_FLAG_READY = core::tFrameworkElementFlags(core::tFrameworkElementFlag::READY).Raw() };

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Data buffer publishing operation
/*!
 * Implements data buffer publishing for all data port implementations
 * in a generic and efficient way.
 *
 * \tparam TPort Port implementation
 * \tparam TPublishingData Class, provided by port implementation class with relevant publishing information
 */
template <typename TPort, typename TPublishingData>
class tPublishOperation : public TPublishingData
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  template <typename ... TArgs>
  tPublishOperation(TArgs && ... args) : TPublishingData(std::forward<TArgs>(args)...) {}

  /*!
   * Performs publishing operation
   *
   * \param port (Output) port to perform publishing operation on
   * \tparam CHANGE_CONSTANT changedConstant to use
   * \tparam BROWSER_PUBLISH Inform this port's listeners on change? (only set from BrowserPublish())
   */
  template <tChangeStatus CHANGE_CONSTANT, bool BROWSER_PUBLISH, bool NOTIFY_LISTENER_ON_THIS_PORT>
  inline void Execute(TPort& port)
  {
    uint flag_query = port.GetAllFlags().Raw() & cRAW_FLAGS_READY_AND_HIJACKED;
    if (flag_query != cRAW_FLAG_READY && (!BROWSER_PUBLISH))
    {
      if (!port.IsReady())
      {
        PrintWarning(port, "is not ready. Ignoring publishing request.");
      }
      this->CheckRecycle();
      return;
    }

    if (!port.template Assign<CHANGE_CONSTANT>(*this))
    {
      this->CheckRecycle();
      return;
    }

    // inform listeners?
    if (NOTIFY_LISTENER_ON_THIS_PORT)
    {
      port.SetChanged(CHANGE_CONSTANT);
#ifndef _LIB_FINROC_PLUGINS_DATA_RECORDING_PRESENT_
      port.template NotifyListeners<CHANGE_CONSTANT>(*this);
    }
#else
    }
    port.template NotifyListeners<CHANGE_CONSTANT>(*this);
#endif

    for (auto it = port.OutgoingConnectionsBegin(); it != port.OutgoingConnectionsEnd(); ++it)
    {
      if (static_cast<const common::tAbstractDataPort&>(it->Destination()).template WantsPush<CHANGE_CONSTANT>())
      {
        if (it->Flags().Get(core::tConnectionFlag::CONVERSION))
        {
          static_cast<const tConversionConnector&>(*it).Publish(this->GetObject(), this->GetTimestamp(), CHANGE_CONSTANT);
        }
        else
        {
          TPort& destination_port = static_cast<TPort&>(it->Destination());
          Receive<CHANGE_CONSTANT>(*this, destination_port, port);
        }
      }
    }
  }

  /*!
   * \param publishing_data Custom data on publishing operation
   * \param port Port that receives data
   * \param origin Port that value was received from
   * \tparam CHANGE_CONSTANT changedConstant to use
   */
  template <tChangeStatus CHANGE_CONSTANT>
  static inline void Receive(typename std::conditional<TPublishingData::cCOPY_ON_RECEIVE, TPublishingData, TPublishingData&>::type publishing_data, TPort& port, TPort& origin)
  {
    if (!port.template Assign<CHANGE_CONSTANT>(publishing_data))
    {
      return;
    }
    port.SetChanged(CHANGE_CONSTANT);
    port.template NotifyListeners<CHANGE_CONSTANT>(publishing_data);
    port.UpdateStatistics(publishing_data, origin, port);

    // forward
    for (auto it = port.OutgoingConnectionsBegin(); it != port.OutgoingConnectionsEnd(); ++it)
    {
      if (static_cast<const common::tAbstractDataPort&>(it->Destination()).template WantsPush<CHANGE_CONSTANT>())
      {
        if (it->Flags().Get(core::tConnectionFlag::CONVERSION))
        {
          static_cast<const tConversionConnector&>(*it).Publish(publishing_data.GetObject(), publishing_data.GetTimestamp(), CHANGE_CONSTANT);
        }
        else
        {
          TPort& destination_port = static_cast<TPort&>(it->Destination());
          Receive<CHANGE_CONSTANT>(publishing_data, destination_port, port);
        }
      }
    }
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*!
   * Put to separate method as it expands to quite a lot of code that does not need to be inlined
   *
   * \param port Port to print message for
   * \param warning Message to print
   */
  __attribute__((noinline))
  static void PrintWarning(TPort& port, const char* warning)
  {
    FINROC_LOG_PRINT_STATIC(WARNING, "Port '", port, "' ", warning);
  }

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

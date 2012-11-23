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
/*!\file    plugins/data_ports/common/tPullOperation.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-11
 *
 * \brief   Contains tPullOperation
 *
 * \b tPullOperation
 *
 * Implements data buffer pulling for all data port implementations
 * in a generic and efficient way.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__common__tPullOperation_h__
#define __plugins__data_ports__common__tPullOperation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

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
namespace common
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Data buffer publishing operation
/*!
 * Implements data buffer pulling for all data port implementations
 * in a generic and efficient way.
 *
 * \tparam TPort Port implementation
 * \tparam TPublishingData Class, provided by port implementation class with relevant publishing information
 * \tparam TManager Port data manager type
 */
template <typename TPort, typename TPublishingData, typename TManager>
class tPullOperation : public TPublishingData
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  template <typename ... TArgs>
  tPullOperation(TArgs && ... args) : TPublishingData(std::forward<TArgs>(args)...) {}

  /*!
   * Performs pull operation
   *
   * \param port (Output) port to perform pull operation on
   * \param intermediate_assign Assign pulled value to ports in between?
   */
  inline void Execute(TPort& port, bool intermediate_assign)
  {
    ExecuteImplementation(port, intermediate_assign, true);
    this->AddLock(); // lock for return
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*!
   * Performs pull operation
   *
   * \param port (Output) port to perform pull operation on
   * \param intermediate_assign Assign pulled value to ports in between?
   * \param first Is this the call on the first (originating) port?
   */
  void ExecuteImplementation(TPort& port, bool intermediate_assign, bool first)
  {
    if ((!first) && port.pull_request_handler)
    {
      port.CallPullRequestHandler(*this, intermediate_assign);
      if (this->published_buffer)
      {
        typename TPort::tTaggedBufferPointer::tStorage tagged_pointer_raw = this->published_buffer_tagged_pointer;
        if (tagged_pointer_raw != port.current_value.load())
        {
          if (!port.Assign(*this))
          {
            port.LockCurrentValueForPublishing(*this);
          }
        }
        return;
      }
    }

    // continue with next-best connected source port
    for (auto it = port.IncomingConnectionsBegin(); it != port.IncomingConnectionsEnd(); ++it)
    {
      ExecuteImplementation(static_cast<TPort&>(*it), intermediate_assign, false);
      typename TPort::tTaggedBufferPointer::tStorage tagged_pointer_raw = this->published_buffer_tagged_pointer;
      if ((first || intermediate_assign) && (tagged_pointer_raw != port.current_value.load()))
      {
        if (!port.Assign(*this))
        {
          port.LockCurrentValueForPublishing(*this);
        }
      }
      return;
    }

    // no connected source port... pull/return current value
    port.LockCurrentValueForPublishing(*this);
  }

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

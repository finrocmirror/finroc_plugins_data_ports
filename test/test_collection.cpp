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
/*!\file    plugins/data_ports/test/test_collection.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2013-01-03
 *
 * Collections of tests.
 * This is the place to add simple tests for data_ports.
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/tRuntimeEnvironment.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tInputPort.h"
#include "plugins/data_ports/tOutputPort.h"
#include "plugins/data_ports/tProxyPort.h"

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
//----------------------------------------------------------------------
using namespace finroc::core;
using namespace finroc::data_ports;

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

void TestPortChains()
{
  FINROC_LOG_PRINT(USER, "Testing forwarding data among port chains");
  tFrameworkElement* parent = new tFrameworkElement(&finroc::core::tRuntimeEnvironment::GetInstance(), "TestPortChains");

  // Create ports
  tOutputPort<std::string> output_port1("Output Port 1", parent);
  tOutputPort<std::string> output_port2("Output Port 2", parent);
  tOutputPort<std::string> output_port3("Output Port 3", parent);
  tProxyPort<std::string, true> proxy_port1("Proxy Port 1", parent);
  tProxyPort<std::string, true> proxy_port2("Proxy Port 2", parent);
  tProxyPort<std::string, true> proxy_port3("Proxy Port 3", parent);
  tInputPort<std::string> input_port1("Input Port 1", parent);
  tInputPort<std::string> input_port2("Input Port 2", parent);
  tInputPort<std::string> input_port3("Input Port 3", parent);

  // Connect ports
  output_port1.ConnectTo(proxy_port1);
  output_port2.ConnectTo(proxy_port2);
  output_port3.ConnectTo(proxy_port3);
  proxy_port1.ConnectTo(input_port1);
  proxy_port2.ConnectTo(input_port2);
  proxy_port3.ConnectTo(input_port3);
  parent->Init();

  std::string test_string = "12345";
  for (int i = 0; i < 20; i++)
  {
    // Publish data
    tPortDataPointer<std::string> unused_buffer = output_port1.GetUnusedBuffer();
    *unused_buffer = "Test";
    output_port1.Publish(unused_buffer);

    // Forward data to second and third chain
    output_port2.Publish(input_port1.GetPointer());
    output_port3.Publish(input_port2.GetPointer());

    if (i > 10)
    {
      output_port2.Publish(test_string);
      output_port3.Publish(input_port2.GetPointer());
    }
  }

  parent->ManagedDelete();
}

int main(int, char**)
{
  TestPortChains();

  return 0;
}

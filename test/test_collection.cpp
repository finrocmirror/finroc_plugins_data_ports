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
#include "plugins/data_ports/tThreadLocalBufferManagement.h"

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
  FINROC_LOG_PRINT(USER, "\nTesting forwarding data among port chains");
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

template <typename T>
void TestPortQueues(const T& value1, const T& value2, const T& value3)
{
  FINROC_LOG_PRINT(USER, "\nTesting port queue basic operation for type ", (rrlib::rtti::tDataType<T>()).GetName());
  tFrameworkElement* parent = new tFrameworkElement(&finroc::core::tRuntimeEnvironment::GetInstance(), "TestPortQueue");

  tOutputPort<T> output_port("Output Port", parent);
  tInputPort<T> input_port_fifo("Input Port FIFO", parent, finroc::core::tFrameworkElement::tFlag::HAS_QUEUE | finroc::core::tFrameworkElement::tFlag::USES_QUEUE);
  tInputPort<T> input_port_all("Input Port ALL", parent, finroc::core::tFrameworkElement::tFlag::HAS_QUEUE | finroc::core::tFrameworkElement::tFlag::USES_QUEUE | finroc::core::tFrameworkElement::tFlag::HAS_DEQUEUE_ALL_QUEUE);
  output_port.ConnectTo(input_port_fifo);
  output_port.ConnectTo(input_port_all);
  parent->Init();

  FINROC_LOG_PRINT(USER, " Enqueueing three values");
  output_port.Publish(value1);
  output_port.Publish(value2);
  output_port.Publish(value3);

  FINROC_LOG_PRINT(USER, " Dequeueing five values FIFO");
  for (size_t i = 0; i < 5; ++i)
  {
    tPortDataPointer<const T> result = input_port_fifo.Dequeue();
    if (result)
    {
      FINROC_LOG_PRINT(USER, "  Dequeued ", *result);
    }
    else
    {
      FINROC_LOG_PRINT(USER, "  Dequeued nothing");
    }
  }

  FINROC_LOG_PRINT(USER, " Dequeueing all values at once");
  tPortBuffers<tPortDataPointer<const T>> dequeued = input_port_all.DequeueAllBuffers();
  while (!dequeued.Empty())
  {
    FINROC_LOG_PRINT(USER, "  Dequeued ", *dequeued.PopFront());
  }

  parent->ManagedDelete();
};



template <typename T>
void TestPortListeners(const T& publish_value)
{
  class tListener
  {
  public:
    void PortChanged(const T& value, tChangeContext& change_context)
    {
      FINROC_LOG_PRINT(USER, "  Port Changed: ", value);
    }
    void PortChanged(tPortDataPointer<const T>& value, tChangeContext& change_context)
    {
      FINROC_LOG_PRINT(USER, "  Port Changed (tPortDataPointer): ", *value);
    }
    void PortChanged(const rrlib::rtti::tGenericObject& value, tChangeContext& change_context)
    {
      FINROC_LOG_PRINT(USER, "  Port Changed Generic: ", value);
    }
    void PortChanged(tPortDataPointer<const rrlib::rtti::tGenericObject>& value, tChangeContext& change_context)
    {
      FINROC_LOG_PRINT(USER, "  Port Changed Generic (tPortDataPointer): ", *value);
    }
    void PortChanged(tChangeContext& change_context)
    {
      FINROC_LOG_PRINT(USER, "  Port Changed Simple");
    }
  };

  FINROC_LOG_PRINT(USER, "\nTesting port listeners for type ", (rrlib::rtti::tDataType<T>()).GetName());
  tListener listener;
  tFrameworkElement* parent = new tFrameworkElement(&finroc::core::tRuntimeEnvironment::GetInstance(), "TestPortListeners");

  tOutputPort<T> output_port("Output Port", parent);
  tInputPort<T> input_port("Input Port", parent);
  output_port.ConnectTo(input_port);
  input_port.AddPortListener(listener);
  input_port.AddPortListenerForPointer(listener);
  input_port.AddPortListenerSimple(listener);
  tGenericPort generic_input_port = tGenericPort::Wrap(*input_port.GetWrapped());
  generic_input_port.AddPortListener(listener);
  generic_input_port.AddPortListenerForPointer(listener);
  generic_input_port.AddPortListenerSimple(listener);
  parent->Init();

  output_port.Publish(publish_value);

  parent->ManagedDelete();
}

int main(int, char**)
{
  TestPortChains();
  TestPortQueues<int>(1, 2, 3);
  TestPortQueues<std::string>("1", "2", "3");
  TestPortListeners<int>(1);
  TestPortListeners<std::string>("test");

  tThreadLocalBufferManagement local_buffers;
  TestPortChains();
  TestPortQueues<int>(1, 2, 3);
  TestPortListeners<int>(1);

  return 0;
}

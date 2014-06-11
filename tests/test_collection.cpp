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
/*!\file    plugins/data_ports/tests/test_collection.cpp
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
#include "rrlib/util/tUnitTestSuite.h"

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
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

void TestPortChains()
{
  FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "\nTesting forwarding data among port chains");
  core::tFrameworkElement* parent = new core::tFrameworkElement(&core::tRuntimeEnvironment::GetInstance(), "TestPortChains");

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
    std::string test_string2 = "Test" + std::to_string(i);
    *unused_buffer = test_string2;
    output_port1.Publish(unused_buffer);

    // Forward data to second and third chain
    output_port2.Publish(input_port1.GetPointer());
    output_port3.Publish(input_port2.GetPointer());
    RRLIB_UNIT_TESTS_ASSERT(test_string2 == *input_port3.GetPointer());

    if (i > 10)
    {
      output_port2.Publish(test_string);
      output_port3.Publish(input_port2.GetPointer());
      RRLIB_UNIT_TESTS_ASSERT(test_string == *input_port3.GetPointer());
      RRLIB_UNIT_TESTS_ASSERT(test_string == *input_port2.GetPointer());
      RRLIB_UNIT_TESTS_ASSERT(test_string2 == *input_port1.GetPointer());
    }
  }

  parent->ManagedDelete();
}

template <typename T>
void TestPortQueues(const T& value1, const T& value2, const T& value3)
{
  FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "\nTesting port queue basic operation for type ", (rrlib::rtti::tDataType<T>()).GetName());
  core::tFrameworkElement* parent = new core::tFrameworkElement(&core::tRuntimeEnvironment::GetInstance(), "TestPortQueue");

  tOutputPort<T> output_port("Output Port", parent);
  tInputPort<T> input_port_fifo("Input Port FIFO", parent, tQueueSettings(false));
  tInputPort<T> input_port_all("Input Port ALL", parent, tQueueSettings(true));
  output_port.ConnectTo(input_port_fifo);
  output_port.ConnectTo(input_port_all);
  parent->Init();

  FINROC_LOG_PRINT(DEBUG_VERBOSE_1, " Enqueueing three values");
  output_port.Publish(value1);
  output_port.Publish(value2);
  output_port.Publish(value3);

  FINROC_LOG_PRINT(DEBUG_VERBOSE_1, " Dequeueing five values FIFO");
  for (size_t i = 0; i < 5; ++i)
  {
    tPortDataPointer<const T> result = input_port_fifo.Dequeue();
    if (result)
    {
      FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "  Dequeued ", *result);
    }
    else
    {
      FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "  Dequeued nothing");
    }
    RRLIB_UNIT_TESTS_ASSERT((i == 0 && value1 == *result) || (i == 1 && value2 == *result) || (i == 2 && value3 == *result) || (i > 2 && (!result)));
  }

  FINROC_LOG_PRINT(DEBUG_VERBOSE_1, " Dequeueing all values at once");
  tPortBuffers<tPortDataPointer<const T>> dequeued = input_port_all.DequeueAllBuffers();
  size_t i = 0;
  while (!dequeued.Empty())
  {
    T result = *dequeued.PopFront();
    FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "  Dequeued ", result);
    RRLIB_UNIT_TESTS_ASSERT((i == 0 && value1 == result) || (i == 1 && value2 == result) || (i == 2 && value3 == result));
    i++;
  }

  parent->ManagedDelete();
};



template <typename T>
void TestPortListeners(const T& publish_value)
{
  class tListener
  {
  public:
    void OnPortChange(const T& value, tChangeContext& change_context)
    {
      FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "  Port Changed: ", value);
      this->value1 = value;
      this->calls++;
    }
    void OnPortChange(tPortDataPointer<const T>& value, tChangeContext& change_context)
    {
      FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "  Port Changed (tPortDataPointer): ", *value);
      this->value2 = *value;
      this->calls++;
    }
    void OnPortChange(const rrlib::rtti::tGenericObject& value, tChangeContext& change_context)
    {
      FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "  Port Changed Generic: ", value);
      this->calls++;
    }
    void OnPortChange(tPortDataPointer<const rrlib::rtti::tGenericObject>& value, tChangeContext& change_context)
    {
      FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "  Port Changed Generic (tPortDataPointer): ", *value);
      this->calls++;
    }
    void OnPortChange(tChangeContext& change_context)
    {
      FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "  Port Changed Simple");
      this->calls++;
    }

    T value1, value2;
    size_t calls;

    tListener() : value1(), value2(), calls(0) {}
  };

  FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "\nTesting port listeners for type ", (rrlib::rtti::tDataType<T>()).GetName());
  tListener listener;
  core::tFrameworkElement* parent = new core::tFrameworkElement(&core::tRuntimeEnvironment::GetInstance(), "TestPortListeners");

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

  RRLIB_UNIT_TESTS_ASSERT(listener.value1 == publish_value && listener.value2 == publish_value && listener.calls == 6);

  parent->ManagedDelete();
}

class DataPortsTestCollection : public rrlib::util::tUnitTestSuite
{
  RRLIB_UNIT_TESTS_BEGIN_SUITE(DataPortsTestCollection);
  RRLIB_UNIT_TESTS_ADD_TEST(Test);
  RRLIB_UNIT_TESTS_END_SUITE;

  void Test()
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
  }
};

RRLIB_UNIT_TESTS_REGISTER_SUITE(DataPortsTestCollection);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

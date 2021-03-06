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
/*!\file    plugins/data_ports/tests/initial_pushing.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-24
 *
 * Tests whether initial pushing functionality in ports works correctly.
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/tRuntimeEnvironment.h"
#include "rrlib/util/demangle.h"
#include "rrlib/util/tUnitTestSuite.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tInputPort.h"
#include "plugins/data_ports/tOutputPort.h"

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

template <typename T>
void CheckPortValue(tPort<T>& port, const T& expected)
{
  T buffer = T();
  port.Get(buffer);
  if (buffer == expected)
  {
    FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "Port '", port.GetName(), "' has value '", buffer, "' - as expected");
  }
  else
  {
    FINROC_LOG_PRINT(ERROR, "Port '", port.GetName(), "' has value '", buffer, "' - expected '", expected, "'");
    RRLIB_UNIT_TESTS_ASSERT_MESSAGE("Failed", false);
  }
}

template <typename T>
void TestInitialPushing(const std::array<T, 9>& test_values)
{
  core::tFrameworkElement* parent = new core::tFrameworkElement(&core::tRuntimeEnvironment::GetInstance(), "Test");
  FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "\nChecking initial pushing for type ", rrlib::util::Demangle(typeid(T).name()));

  // Create initial set of ports
  tOutputPort<T> output_port("Output Port", parent);
  tInputPort<T> input_port("Input Port", parent);
  tOutputPort<T> output_port_reverse("Output Port with reverse pushing", core::tFrameworkElement::tFlag::PUSH_STRATEGY_REVERSE, parent);
  core::tFrameworkElement::InitAll();

  // Fill output port with something
  output_port.Publish(test_values[0]);

  // Connect to other ports and check their values
  output_port.ConnectTo(input_port);
  output_port_reverse.ConnectTo(input_port);
  CheckPortValue(input_port, test_values[0]);
  CheckPortValue(output_port_reverse, test_values[0]);

  // Change strategy and see if everything behaves as expected
  input_port.SetPushStrategy(false);
  output_port.Publish(test_values[1]);
  input_port.SetPushStrategy(true);
  CheckPortValue(input_port, test_values[0]); // expects old value because we have two sources => no push
  CheckPortValue(output_port_reverse, test_values[0]);
  output_port_reverse.SetReversePushStrategy(false);
  output_port.Publish(test_values[2]);
  CheckPortValue(output_port_reverse, test_values[0]);
  output_port_reverse.SetReversePushStrategy(true);
  CheckPortValue(output_port_reverse, test_values[2]);

  // now for a complex net
  FINROC_LOG_PRINT(DEBUG_VERBOSE_1, "\nNow for a complex net...");

  // o1->o2
  tOutputPort<T> o1("o1", core::tFrameworkElement::tFlag::ACCEPTS_DATA, parent); // flag makes this also a proxy port
  core::tFrameworkElement::InitAll();
  o1.Publish(test_values[3]);
  tInputPort<T> o2("o2", core::tFrameworkElement::tFlag::EMITS_DATA, parent); // flag makes this also a proxy port
  core::tFrameworkElement::InitAll();
  o1.ConnectTo(o2);
  CheckPortValue(o2, test_values[3]);

  // o1->o2->o3
  tInputPort<T> o3("o3", parent);
  o2.ConnectTo(o3);
  core::tFrameworkElement::InitAll();
  o2.SetPushStrategy(false);
  o3.SetPushStrategy(false);
  o1.Publish(test_values[4]);
  //print(o3, 24); ok pulled
  o3.SetPushStrategy(true);
  CheckPortValue(o3, test_values[4]);

  // o0->o1->o2->o3
  tOutputPort<T> o0("o0", core::tFrameworkElement::tFlag::ACCEPTS_DATA, parent); // flag makes this also a proxy port
  core::tFrameworkElement::InitAll();
  o0.Publish(test_values[5]);
  o0.ConnectTo(o1, core::tAbstractPort::tConnectDirection::TO_TARGET);
  CheckPortValue(o3, test_values[5]);

  // o6->o0->o1->o2->o3
  //              \            .
  //               o4->o5
  tInputPort<T> o4("o4", core::tFrameworkElement::tFlag::EMITS_DATA, parent); // flag makes this also a proxy port
  tInputPort<T> o5("o5", parent);
  core::tFrameworkElement::InitAll();
  o4.ConnectTo(o5);
  o2.ConnectTo(o4, core::tAbstractPort::tConnectDirection::TO_TARGET);
  CheckPortValue(o5, test_values[5]);
  tOutputPort<T> o6("o6", parent);
  core::tFrameworkElement::InitAll();
  o6.Publish(test_values[6]);
  o6.ConnectTo(o0);
  CheckPortValue(o3, test_values[6]);
  CheckPortValue(o5, test_values[6]);

  // o6->o0->o1->o2->o3
  //        /     \            .
  //      o7->o8   o4->o5
  tOutputPort<T> o7("o7", core::tFrameworkElement::tFlag::ACCEPTS_DATA, parent); // flag makes this also a proxy port
  core::tFrameworkElement::InitAll();
  o7.Publish(test_values[7]);
  tInputPort<T> o8("o8", parent, tQueueSettings(true, 5));
  core::tFrameworkElement::InitAll();
  o7.ConnectTo(o8);
  CheckPortValue(o8, test_values[7]);
  o7.ConnectTo(o1, core::tAbstractPort::tConnectDirection::TO_TARGET);
  CheckPortValue(o1, test_values[6]);

  tPortBuffers<tPortDataPointer<const T>> queue_fragment = o8.DequeueAllBuffers();
  RRLIB_UNIT_TESTS_ASSERT_MESSAGE("o8 queue is not empty as expected.", queue_fragment.Empty());

  // o6->o0->o1->o2->o3
  //        /     \            .
  //  o9->o7->o8   o4->o5
  tOutputPort<T> o9("o9", parent);
  core::tFrameworkElement::InitAll();
  o9.Publish(test_values[8]);
  o9.ConnectTo(o7);
  CheckPortValue(o8, test_values[8]);
  CheckPortValue(o1, test_values[6]);
  CheckPortValue(o3, test_values[6]);

  parent->ManagedDelete();
}

class InitialPushTest : public rrlib::util::tUnitTestSuite
{
  RRLIB_UNIT_TESTS_BEGIN_SUITE(InitialPushTest);
  RRLIB_UNIT_TESTS_ADD_TEST(Test);
  RRLIB_UNIT_TESTS_END_SUITE;

  void Test()
  {
    TestInitialPushing<int>({{11, 22, 33, 44, 55, 66, 77, 88, 99}});
    TestInitialPushing<finroc::data_ports::numeric::tNumber>({{11, 22, 33, 44, 55, 66, 77, 88, 99}});
    TestInitialPushing<std::string>({{"11", "22", "33", "44", "55", "66", "77", "88", "99"}});
  }
};

RRLIB_UNIT_TESTS_REGISTER_SUITE(InitialPushTest);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

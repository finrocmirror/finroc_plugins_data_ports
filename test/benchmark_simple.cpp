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
/*!\file    plugins/data_ports/test/benchmark_simple.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-27
 *
 * Benchmarks for various port constellations and usage.
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
// Namespace declaration
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------
int cycles = 10000000; // 10 million cycles by default

/*!
 * Test type for standard ports
 */
struct tTestType
{
  int value;

  ~tTestType()
  {
    FINROC_LOG_PRINT_STATIC(DEBUG_VERBOSE_1, "Deleting"); // non-trivial destructor
  }
};

inline rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tTestType& t)
{
  return stream;
}
inline rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tTestType& t)
{
  return stream;
}


void BenchmarkCheapCopyPort()
{
  tFrameworkElement* parent = new tFrameworkElement(&tRuntimeEnvironment::GetInstance(), "Test");
  tOutputPort<int> output_port("Output Port", parent);
  tInputPort<int> input_port("Input Port", parent);
  output_port.ConnectTo(input_port);
  parent->Init();

  rrlib::time::tTimestamp start = rrlib::time::Now();

  __attribute__((unused))
  int result = 0;
  for (int i = 0; i < cycles; i++)
  {
    output_port.Publish(i);
    result = input_port.Get();
    assert(result == i);
  }
  rrlib::time::tDuration time = rrlib::time::Now() - start;
  double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(time).count() / 1000.0; // cycle time in seconds
  FINROC_LOG_PRINT(USER, "  Completed ", cycles, " Publish() and Get() operations in ", rrlib::time::ToString(time), " => ", static_cast<int64_t>(cycles / seconds), " Publish()+Get() operations per second");

  parent->ManagedDelete();
}

void BenchmarkStandardPort()
{
  tFrameworkElement* parent = new tFrameworkElement(&tRuntimeEnvironment::GetInstance(), "Test");
  tOutputPort<tTestType> output_port("Output Port", parent);
  tInputPort<tTestType> input_port("Input Port", parent);
  output_port.ConnectTo(input_port);
  parent->Init();

  rrlib::time::tTimestamp start = rrlib::time::Now();

  __attribute__((unused))
  int result = 0;
  for (int i = 0; i < cycles; i++)
  {
    tPortDataPointer<tTestType> buffer = output_port.GetUnusedBuffer();
    buffer->value = i;
    output_port.Publish(buffer);

    result = input_port.GetPointer()->value;
    assert(result == i);
  }
  rrlib::time::tDuration time = rrlib::time::Now() - start;
  double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(time).count() / 1000.0; // cycle time in seconds
  FINROC_LOG_PRINT(USER, "  Completed ", cycles, " Publish() and Get() operations in ", rrlib::time::ToString(time), " => ", static_cast<int64_t>(cycles / seconds), " Publish()+Get() operations per second");

  parent->ManagedDelete();
}

int main(int argc, char** argv)
{
  if (argc >= 2)
  {
    int temp = atoi(argv[1]);
    if (temp > 0)
    {
      cycles = temp;
      FINROC_LOG_PRINT(USER, "Doing benchmarks with ", cycles, " cycles.");
    }
  }

  FINROC_LOG_PRINT(USER, "\nBenchmarking cheap copy port with global buffers...");
  BenchmarkCheapCopyPort();
  tThreadLocalBufferManagement thread_local_buffers;
  FINROC_LOG_PRINT(USER, "Benchmarking cheap copy port with thread local buffers...");
  BenchmarkCheapCopyPort();

  FINROC_LOG_PRINT(USER, "Benchmarking standard port...");
  BenchmarkStandardPort();

  return 0;
}


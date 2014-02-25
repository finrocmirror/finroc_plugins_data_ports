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
/*!\file    plugins/data_ports/test/basic_realtime.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-28
 *
 * Tests basic real-time functionality (maximum and average latency of loop threads)
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/tOutputPort.h"

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
//----------------------------------------------------------------------
using namespace finroc::data_ports;

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------
rrlib::time::tDuration cINTERVAL = std::chrono::microseconds(500);

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

class tTestThread : public rrlib::thread::tLoopThread
{
public:

  tOutputPort<int64_t> port;

  rrlib::time::tAtomicDuration max_latency;

  rrlib::time::tAtomicDuration total_latency;

  std::atomic<int64_t> cycles;

  tTestThread(const std::string& name) :
    tLoopThread(cINTERVAL),
    port(name + "-port"),
    max_latency(rrlib::time::tDuration(0)),
    total_latency(rrlib::time::tDuration(0)),
    cycles(0)
  {
    port.Init();
    SetName(name);
  }

  virtual void MainLoopCallback() override
  {
    rrlib::time::tDuration diff = rrlib::time::Now() - tLoopThread::GetCurrentCycleStartTime();
    if (max_latency.Load() < diff)
    {
      max_latency.Store(diff);
    }
    total_latency.Store(total_latency.Load() + diff);
    int c = cycles.fetch_add(1) + 1;
    port.Publish(c);
  }

  void PrintStats(std::ostream& stream) const
  {
    stream << "Cycles: " << cycles.load() << "; Max Latency: " << rrlib::time::ToString(max_latency.Load()) << "; Average Latency: " << rrlib::time::ToString(std::chrono::nanoseconds(total_latency.Load()) / std::max<int64_t>(1, cycles.load()));
  }
};

int main(int argc__, char **argv__)
{
  tTestThread* rt_thread = new tTestThread("RT-Thread");
  rt_thread->SetAutoDelete();
  tTestThread* thread = new tTestThread("Normal Thread");
  thread->SetAutoDelete();
  rt_thread->SetRealtime();
  rt_thread->Start();
  thread->Start();

  while (true)
  {
    rt_thread->PrintStats(std::cout);
    std::cout << "    ";
    thread->PrintStats(std::cout);
    std::cout << std::endl;
    rrlib::thread::tThread::Sleep(std::chrono::seconds(1), false);
  }
}

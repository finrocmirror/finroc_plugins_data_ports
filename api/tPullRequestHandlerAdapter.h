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
/*!\file    plugins/data_ports/api/tPullRequestHandlerAdapter.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-24
 *
 * \brief   Contains tPullRequestHandlerAdapter
 *
 * \b tPullRequestHandlerAdapter
 *
 * Adapts tPortPullRequestHandlerRaw of different port implementations to tPortPullRequestHandler
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__api__tPullRequestHandlerAdapter_h__
#define __plugins__data_ports__api__tPullRequestHandlerAdapter_h__

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
namespace api
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Adapts tPortPullRequestHandlerRaw
/*!
 * Adapts tPortPullRequestHandlerRaw of different port implementations to tPortPullRequestHandler
 */
template <typename T, tPortImplementationType TPortImplementationType>
class tPullRequestHandlerAdapter : public optimized::tPullRequestHandlerRaw
{
  typedef tPortImplementation<T, TPortImplementationType> tImplementation;

  virtual bool PullRequest(const tPort<T>& origin, T& result) = 0;

  virtual bool PullRequest(optimized::tCheapCopyPort& origin, optimized::tCheaplyCopiedBufferManager& result_buffer, bool intermediate_assign)
  {
    T t = T();
    rrlib::time::tTimestamp timestamp(rrlib::time::cNO_TIME);
    bool result = PullRequeust(tPort<T>(origin), t, timestamp);
    if (result)
    {
      tImplementation::Assign(result_buffer.GetObject().GetData<typename tImplementation::tPortBuffer>(), t, origin);
    }
    return result;
  }
};

template <typename T>
class tPullRequestHandlerAdapter<T, tPortImplementationType::STANDARD> : public standard::tPullRequestHandlerRaw
{
  virtual bool PullRequest(const tPort<T>& origin, T& result) = 0;

  virtual standard::tPortBufferManager* PullRequest(standard::tStandardPort& origin, int add_locks, bool intermediate_assign)
  {
    T t = T();
    rrlib::time::tTimestamp timestamp(rrlib::time::cNO_TIME);
    bool result = PullRequeust(tPort<T>(origin), t, timestamp);
    if (result)
    {
      typename standard::tStandardPort::tUnusedManagerPointer buffer(origin.GetUnusedBufferRaw());
      rrlib::rtti::sStaticTypeInfo<T>::DeepCopy(t, buffer.GetObject.GetData<T>());
      buffer.InitReferenceCounter(add_locks);
      return buffer.release();
    }
    return NULL;
  }
};


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

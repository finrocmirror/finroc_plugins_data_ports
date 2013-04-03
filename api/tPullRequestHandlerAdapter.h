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
#include "plugins/data_ports/optimized/tPullRequestHandlerRaw.h"
#include "plugins/data_ports/standard/tPullRequestHandlerRaw.h"

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
template <typename T, bool CHEAPLY_COPIED_TYPE>
class tPullRequestHandlerAdapter : public optimized::tPullRequestHandlerRaw
{
  typedef tPortImplementation<T, api::tPortImplementationTypeTrait<T>::type> tImplementation;

  virtual tPortDataPointer<const T> PullRequest(common::tAbstractDataPort& origin) = 0;

  virtual bool RawPullRequest(optimized::tCheapCopyPort& origin, optimized::tCheaplyCopiedBufferManager& result_buffer)
  {
    tPortDataPointer<const T> pulled_buffer = PullRequest(origin);
    if (pulled_buffer)
    {
      tImplementation::Assign(result_buffer.GetObject().GetData<typename tImplementation::tPortBuffer>(), *pulled_buffer, origin);
      result_buffer.SetTimestamp(pulled_buffer->GetTimestamp());
    }
    return pulled_buffer;
  }
};

template <typename T>
class tPullRequestHandlerAdapter<T, false> : public standard::tPullRequestHandlerRaw
{
  virtual tPortDataPointer<const T> PullRequest(common::tAbstractDataPort& origin) = 0;

  virtual typename standard::tStandardPort::tUniversalManagerPointer RawPullRequest(standard::tStandardPort& origin)
  {
    tPortDataPointer<const T> pulled_buffer = PullRequest(origin);
    typename standard::tStandardPort::tUniversalManagerPointer buffer(pulled_buffer.implementation.Release());
    return buffer;
    /*typename standard::tStandardPort::tUnusedManagerPointer buffer(origin.GetUnusedBufferRaw());
    rrlib::time::tTimestamp timestamp(rrlib::time::cNO_TIME);
    bool result = PullRequeust(tPort<T>(origin), buffer->GetObject().GetData<T>(), timestamp);
    if (result)
    {
      buffer->SetTimestamp(timestamp);
      buffer.InitReferenceCounter(add_locks);
      return buffer.release();
    }
    return NULL;*/
  }
};

template <>
class tPullRequestHandlerAdapter<rrlib::rtti::tGenericObject, false> : public standard::tPullRequestHandlerRaw, public optimized::tPullRequestHandlerRaw
{
  virtual tPortDataPointer<const rrlib::rtti::tGenericObject> PullRequest(common::tAbstractDataPort& origin) = 0;

  virtual typename standard::tStandardPort::tUniversalManagerPointer RawPullRequest(standard::tStandardPort& origin)
  {
    tPortDataPointer<const rrlib::rtti::tGenericObject> pulled_buffer = PullRequest(origin);
    typename standard::tStandardPort::tUniversalManagerPointer buffer(static_cast<standard::tPortBufferManager*>(pulled_buffer.implementation.Release()));
    return buffer;
  }

  virtual bool RawPullRequest(optimized::tCheapCopyPort& origin, optimized::tCheaplyCopiedBufferManager& result_buffer)
  {
    tPortDataPointer<const rrlib::rtti::tGenericObject> pulled_buffer = PullRequest(origin);
    if (pulled_buffer)
    {
      result_buffer.GetObject().DeepCopyFrom(*pulled_buffer, NULL);
      result_buffer.SetTimestamp(pulled_buffer.GetTimestamp());
    }
    return pulled_buffer;
  }
};


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

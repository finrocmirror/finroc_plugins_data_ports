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
/*!\file    plugins/data_ports/tInputPort.hpp
 *
 * \author  Max Reichardt
 *
 * \date    2013-01-17
 *
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

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
namespace api
{
template <typename LISTENER, bool FIRST_LISTENER>
class tPortListenerAdapterSimple;
}

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

template <typename T> template <typename TListener>
void tInputPort<T>::AddPortListener(TListener& listener)
{
  if (this->GetWrapped()->GetPortListener())
  {
    typedef api::tPortListenerAdapter<TListener, T, api::tPortImplementationTypeTrait<T>::type, false> tAdapter;
    this->GetWrapped()->SetPortListener(new tAdapter(listener, *this->GetWrapped()->GetPortListener()));
  }
  else
  {
    typedef api::tPortListenerAdapter<TListener, T, api::tPortImplementationTypeTrait<T>::type, true> tAdapter;
    this->GetWrapped()->SetPortListener(new tAdapter(listener));
  }
}

template <typename T> template <typename TListener>
void tInputPort<T>::AddPortListenerForPointer(TListener& listener)
{
  if (this->GetWrapped()->GetPortListener())
  {
    typedef api::tPortListenerAdapterForPointer<TListener, T, false> tAdapter;
    this->GetWrapped()->SetPortListener(new tAdapter(listener, *this->GetWrapped()->GetPortListener()));
  }
  else
  {
    typedef api::tPortListenerAdapterForPointer<TListener, T, true> tAdapter;
    this->GetWrapped()->SetPortListener(new tAdapter(listener));
  }
}

template <typename T> template <typename TListener>
void tInputPort<T>::AddPortListenerSimple(TListener& listener)
{
  if (this->GetWrapped()->GetPortListener())
  {
    typedef api::tPortListenerAdapterSimple<TListener, false> tAdapter;
    this->GetWrapped()->SetPortListener(new tAdapter(listener, *this->GetWrapped()->GetPortListener()));
  }
  else
  {
    typedef api::tPortListenerAdapterSimple<TListener, true> tAdapter;
    this->GetWrapped()->SetPortListener(new tAdapter(listener));
  }
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

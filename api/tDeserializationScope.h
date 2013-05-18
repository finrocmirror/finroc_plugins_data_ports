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
/*!\file    plugins/data_ports/api/tDeserializationScope.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-12-19
 *
 * \brief   Contains tDeserializationScope
 *
 * \b tDeserializationScope
 *
 * When deserializing data from (e.g. network) streams,
 * contains information where to get the empty/unused buffers from.
 * This buffer source will be used until the object goes out of scope.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__api__tDeserializationScope_h__
#define __plugins__data_ports__api__tDeserializationScope_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/standard/tMultiTypePortBufferPool.h"

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

template <typename, bool>
class tPortDataPointerImplementation;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Buffer source scope for deserialization
/*!
 * When deserializing data from (e.g. network) streams,
 * contains information where to get the empty/unused buffers from.
 * This buffer source will be used until the object goes out of scope.
 */
class tDeserializationScope : private rrlib::util::tNoncopyable
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Standard constructor:
   * A buffer pool is provided on construction.
   */
  tDeserializationScope(standard::tMultiTypePortBufferPool& buffer_source) :
    buffer_source(&buffer_source),
    outer_scope(current_scope)
  {
    current_scope = this;
  }

  virtual ~tDeserializationScope()
  {
    current_scope = outer_scope; // restores old scope
  }

  /*!
   * \return Buffer source in current scope
   */
  static standard::tMultiTypePortBufferPool& GetBufferSource()
  {
    if (!current_scope)
    {
      throw std::runtime_error("No scope created");
    }
    if (!current_scope->buffer_source)
    {
      current_scope->buffer_source = &current_scope->ObtainBufferPool();
    }
    return *(current_scope->buffer_source);
  }

  /*!
   * \param type Data type
   * \return Unused buffer of this type
   */
  static tPortDataPointerImplementation<rrlib::rtti::tGenericObject, false> GetUnusedBuffer(const rrlib::rtti::tType& type);


//----------------------------------------------------------------------
// Protected constructor
//----------------------------------------------------------------------
protected:

  /*!
   * Constructor for subclasses only:
   * A buffer pool is provided on callback
   * (CreateBufferPool must be overridden by subclass)
   */
  tDeserializationScope() :
    buffer_source(NULL),
    outer_scope(current_scope)
  {
    current_scope = this;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Buffer source to use */
  standard::tMultiTypePortBufferPool* buffer_source;

  /*! Active scope before this scope was created */
  tDeserializationScope* outer_scope;

  /*! Active scope */
  static __thread tDeserializationScope* current_scope;


  /*!
   * Obtains buffer pool, if not provided on construction.
   */
  virtual standard::tMultiTypePortBufferPool& ObtainBufferPool();
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

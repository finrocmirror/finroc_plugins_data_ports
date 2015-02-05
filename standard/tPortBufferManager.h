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
/*!\file    plugins/data_ports/standard/tPortBufferManager.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-10-30
 *
 * \brief   Contains tPortBufferManager
 *
 * \b tPortBufferManager
 *
 * This class manages a single port buffer.
 * It handles information on locks, data type, timestamp etc.
 *
 * If it possible to derive a port buffer manager from another port buffer manager.
 * They will share the same reference counter.
 * This makes sense, when an object contained in the original port buffer
 * shall be used in another port.
 * This way, it does not need to copied.
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__standard__tPortBufferManager_h__
#define __plugins__data_ports__standard__tPortBufferManager_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/data_ports/common/tReferenceCountingBufferManager.h"

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
namespace data_compression
{
class tPlugin;
}

namespace data_ports
{
namespace standard
{
//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Manages a port buffer
/*!
 * This class manages a single port buffer.
 * It handles information on locks, data type, timestamp etc.
 *
 * TODO: If it possible to derive a port buffer manager from another port buffer manager.
 * They will share the same reference counter.
 * This makes sense, when an object contained in the original port buffer
 * shall be used in another port.
 * This way, it does not need to copied.
 */
class tPortBufferManager : public common::tReferenceCountingBufferManager
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  ~tPortBufferManager();

  /*!
   * Attach compressed data to buffer.
   * Components that have their data also available in compressed form, can attach this to their published data.
   * E.g. a frame grabber might receive images in MJPG from a camera driver.
   * Later, compression might be required for network connections or recording.
   * Instead of recompressing the data, the original can be used.
   * Must be called, before buffer is pusblished.
   *
   * Note: if Finroc data_compression plugin is not available, the data is discarded.
   *
   * (only available for data types that are not cheaply copied (they are small anyway) and not for buffers that contain const data)
   *
   * \param compression_format Format in which data was compressed (e.g. "jpg" - should be the same strings as used with rrlib_data_compression to allow decompression)
   *                           Note that string is not copied and must remain valid as long as data
   * \param data Pointer to compressed data (data is copied)
   * \param size Size of compressed data
   * \param key_frame Is this a key frame? (meaning that data be uncompressed without knowledge of any former data (frames))
   */
  void AttachCompressedData(const char* compression_format, void* data, size_t size, bool key_frame)
  {
    if (!unused)
    {
      FINROC_LOG_PRINT(WARNING, "Buffer has already been published. No data is attached.");
      return;
    }

    if (!compressed_data)
    {
      compressed_data.reset(new std::tuple<rrlib::serialization::tMemoryBuffer, const char*, bool>());
    }
    rrlib::serialization::tOutputStream stream(std::get<0>(*compressed_data));
    stream.Write(data, size);
    std::get<1>(*compressed_data) = compression_format;
    std::get<2>(*compressed_data) = key_frame;
    this->compression_status = 3;  // Enum value for "data available" (see finroc::data_compression::tPlugin)
  }

  /*!
   * Creates instance of tPortBufferManager containing a buffer
   * of the specified type
   *
   * \param Type of buffer
   * \return Created instance (can be deleted like any other objects using delete operator)
   */
  static tPortBufferManager* CreateInstance(const rrlib::rtti::tType& type);

  /*!
   * \return Managed Buffer as generic object
   */
  inline rrlib::rtti::tGenericObject& GetObject()
  {
    return reinterpret_cast<rrlib::rtti::tGenericObject&>(*(this + 1));
  }

  /*!
   * \return Is this (still) a unused buffer?
   */
  inline bool IsUnused() const
  {
    return unused;
  }

  /*!
   * \param unused Whether to mark this buffer as still unused
   */
  inline void SetUnused(bool unused)
  {
    this->unused = unused;
    this->compression_status = 0;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Value relevant for publishing thread only - is this still a unused buffer? */
  bool unused;

  /*! PortDataManager that this manager is derived from - null if not derived */
  tPortBufferManager* derived_from;

  friend class data_compression::tPlugin;

  /*! Compression status */
  std::atomic<uint8_t> compression_status;

  /*! Info on compressed data (compressed data, compression format, key frame?) */
  std::unique_ptr<std::tuple<rrlib::serialization::tMemoryBuffer, const char*, bool>> compressed_data;

  tPortBufferManager();

  virtual rrlib::rtti::tGenericObject& GetObjectImplementation() override;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif

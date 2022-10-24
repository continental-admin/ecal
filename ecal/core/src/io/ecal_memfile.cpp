/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

/**
 * @brief  base memory file interface
**/

#include "ecal_def.h"
#include "ecal_memfile.h"
#include "ecal_memfile_info.h"
#include "ecal_memfile_db.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <random>

#define SIZEOF_PARTIAL_STRUCT(_STRUCT_NAME_, _FIELD_NAME_) (reinterpret_cast<std::size_t>(&(reinterpret_cast<_STRUCT_NAME_*>(0)->_FIELD_NAME_)) + sizeof(_STRUCT_NAME_::_FIELD_NAME_)) //NOLINT

namespace eCAL
{
  /////////////////////////////////////////////////////////////////////////////////
  // Memory file handling class
  /////////////////////////////////////////////////////////////////////////////////

  CMemoryFile::CMemoryFile() :
    m_created(false),
    m_access_state(access_state::closed),
    m_name("")
  {
  }

  CMemoryFile::~CMemoryFile()
  {
    Destroy(false);
  }

  bool CMemoryFile::Create(const char* name_, const bool create_, const size_t len_, bool auto_sanitizing_)
  {
    assert((create_ && len_ > 0) || (!create_ && len_ == 0));
    assert((auto_sanitizing_ && create_) || !auto_sanitizing_);

    m_auto_sanitizing = auto_sanitizing_;

    // do we have to recreate the file ?
    if ((m_name != name_)
      || (
        create_
        && (len_ > 0)
        && (m_header.max_data_size != (unsigned long)len_)
        )
      )
    {
      // destroy existing connection
      Destroy(create_);

      // reset states
      m_created      = false;
      m_access_state = access_state::closed;
      m_name.clear();

      // reset header and info
      m_header       = SInternalHeader();
      m_memfile_info = SMemFileInfo();

      // create memory file
      if (!memfile::db::AddFile(name_, create_, create_ ? len_ + m_header.int_hdr_size : SIZEOF_PARTIAL_STRUCT(SInternalHeader, int_hdr_size), m_memfile_info))
      {
#ifndef NDEBUG
        printf("Could not create memory file: %s.\n\n", name_);
#endif
        return(false);
      }
    }

    // create mutex
    if (!CreateMtx(name_, m_memfile_info.mutex, m_auto_sanitizing))
    {
#ifndef NDEBUG
      printf("Could not create memory file mutex: %s.\n\n", name_);
#endif
      return(false);
    }

    if (create_)
    {
      // create header
      m_header.max_data_size = (unsigned long)len_;

      // lock mutex
      // for performance reasons only apply consistency check if it is explicitly set
      bool lock_is_consistent{ true };
      if (LockMtx(&m_memfile_info.mutex, PUB_MEMFILE_CREATE_TO, m_auto_sanitizing ? &lock_is_consistent : nullptr))
      {
        if (m_memfile_info.mem_address)
        {
          SInternalHeader* header = reinterpret_cast<SInternalHeader*>(m_memfile_info.mem_address);

          // reset header if memfile does not exist or rather is not initialized as well as if lock state is inconsistent
          if (!m_memfile_info.exists || header->int_hdr_size == 0 || (m_auto_sanitizing && !lock_is_consistent))
            *header = m_header;
          else
          {
            // read compatible header part if magic number already exists
            memcpy(&m_header, header, std::min(sizeof(SInternalHeader), static_cast<std::size_t>(header->int_hdr_size)));

            // set version number manually if version field is not available
            if (SIZEOF_PARTIAL_STRUCT(SInternalHeader, max_data_size) == m_header.int_hdr_size)
              m_header.version = 0;
          }
        }

        // unlock mutex
        UnlockMtx(&m_memfile_info.mutex);

      }
    }
    else
    {
      // lock mutex
      // consistency check cannot be performed on read-only memfiles
      if (LockMtx(&m_memfile_info.mutex, PUB_MEMFILE_CREATE_TO))
      {
        // read internal header size of memory file
        const auto header_size = static_cast<SInternalHeader*>(m_memfile_info.mem_address)->int_hdr_size;
        memfile::db::CheckFileSize(name_, header_size, m_memfile_info);

        // copy compatible header part into m_header
        memcpy(&m_header, m_memfile_info.mem_address, std::min(sizeof(SInternalHeader), static_cast<std::size_t>(header_size)));

        // set version number manually if version field is not available
        if (SIZEOF_PARTIAL_STRUCT(SInternalHeader, max_data_size) == header_size)
          m_header.version = 0;

        // unlock mutex
        UnlockMtx(&m_memfile_info.mutex);
      }
    }

    // set states
    m_created = true;
    m_name    = name_;

    return(m_created);
  }

  bool CMemoryFile::Destroy(const bool remove_)
  {
    if (!m_created) return(false);

    // return state
    bool ret_state = true;

    // destroy memory file
    ret_state &= memfile::db::RemoveFile(m_name, remove_);

    // unlock mutex
    ret_state &= DestroyMtx(&m_memfile_info.mutex);

    // cleanup mutex
    ret_state &= CleanupMtx(m_name);

    // reset states
    m_created      = false;
    m_access_state = access_state::closed;
    m_name.clear();

    // reset header and info
    m_header       = SInternalHeader();
    m_memfile_info = SMemFileInfo();

    return(ret_state);
  }

  bool CMemoryFile::GetReadAccess(int timeout_)
  {
    // currently we do not differ between read and write access
    if (GetAccess(timeout_))
    {
      // mark as opened for read access
      m_access_state = access_state::read_access;

      return(true);
    }

    return(false);
  }

  bool CMemoryFile::ReleaseReadAccess()
  {
    if (!m_created)                                  return(false);
    if (m_access_state != access_state::read_access) return(false);

    // reset states
    m_access_state = access_state::closed;

    // release read mutex
    UnlockMtx(&m_memfile_info.mutex);

    return(true);
  }

  size_t CMemoryFile::GetReadAddress(const void*& buf_, const size_t len_)
  {
    if (!m_created)                                          return(0);
    if (m_access_state != access_state::read_access)         return(0);
    if (len_ == 0)                                           return(0);
    if (len_ > static_cast<size_t>(m_header.cur_data_size))  return(0);
    if (!m_memfile_info.mem_address)                         return(0);

    // return read address
    buf_ = static_cast<char*>(m_memfile_info.mem_address) + m_header.int_hdr_size;

    return(len_);
  }

  size_t CMemoryFile::Read(void* buf_, const size_t len_, const size_t offset_)
  {
    if (!buf_) return(0);

    const void* rbuf(nullptr);
    if (GetReadAddress(rbuf, len_ + offset_))
    {
      // copy from read buffer with offset
      memcpy(buf_, static_cast<const char*>(rbuf) + offset_, len_);

      // return number of read bytes
      return(len_);
    }
    else
    {
      return(0);
    }
  }

  bool CMemoryFile::GetWriteAccess(int timeout_)
  {
    // currently we do not differ between read and write access
    if (GetAccess(timeout_))
    {
      // mark as opened for write access
      m_access_state = access_state::write_access;

      return(true);
    }

    return(false);
  }

  bool CMemoryFile::ReleaseWriteAccess()
  {
    if (!m_created)                                   return(false);
    if (m_access_state != access_state::write_access) return(false);

    // reset access state
    m_access_state = access_state::closed;

    // unlock mutex
    UnlockMtx(&m_memfile_info.mutex);

    return(true);
  }

  size_t CMemoryFile::GetWriteAddress(void*& buf_, const size_t len_)
  {
    if (!m_created)                                          return(0);
    if (m_access_state != access_state::write_access)        return(0);
    if (len_ == 0)                                           return(0);
    if (len_ > static_cast<size_t>(m_header.max_data_size))  return(0);
    if (!m_memfile_info.mem_address)                         return(0);

    // update m_header and write into memory file header
    m_header.cur_data_size = (unsigned long)(len_);
    SInternalHeader* pHeader = static_cast<SInternalHeader*>(m_memfile_info.mem_address);
    pHeader->cur_data_size = m_header.cur_data_size;

    // return write address
    buf_ = static_cast<char*>(m_memfile_info.mem_address) + m_header.int_hdr_size;

    return(len_);
  }

  size_t CMemoryFile::Write(const void* buf_, const size_t len_, const size_t offset_)
  {
    if (!m_created) return(0);
    if (!buf_)      return(0);

    void* wbuf(nullptr);
    if (GetWriteAddress(wbuf, len_ + offset_))
    {
      // copy to write buffer
      memcpy(static_cast<char*>(wbuf) + offset_, buf_, len_);

      // return number of written bytes
      return(len_);
    }
    else
    {
      return(0);
    }
  }

  bool CMemoryFile::GetAccess(int timeout_)
  {
    if (!m_created)                  return(false);
    if (!m_memfile_info.mem_address) return(false);

    // lock mutex
    bool lock_is_consistent {true};
    if (!LockMtx(&m_memfile_info.mutex, timeout_, m_auto_sanitizing ? &lock_is_consistent : nullptr))
    {
#ifndef NDEBUG
      printf("Could not lock memory file mutex: %s.\n\n", m_name.c_str());
#endif
      return(false);
    }

    // reset current data size field of memfile header if lock is inconsistent 
    if ((m_auto_sanitizing && !lock_is_consistent) && m_header.version > 0)
    {
      m_header.cur_data_size = 0;
      *reinterpret_cast<SInternalHeader*>(m_memfile_info.mem_address) = m_header;
    }

    // update compatible header part of m_header
    memcpy(&m_header, m_memfile_info.mem_address, std::min(sizeof(SInternalHeader), static_cast<std::size_t>(m_header.int_hdr_size)));

    // check size again
    size_t len = static_cast<size_t>(m_header.int_hdr_size) + static_cast<size_t>(m_header.max_data_size);
    if (len > m_memfile_info.size)
    {
      // check file size and update memory file map
      memfile::db::CheckFileSize(m_name, len, m_memfile_info);

      // check size again and give up if it is still to small
      if (len > m_memfile_info.size)
      {
        // unlock mutex
        UnlockMtx(&m_memfile_info.mutex);
        return(false);
      }
    }

    return(true);
  }
}

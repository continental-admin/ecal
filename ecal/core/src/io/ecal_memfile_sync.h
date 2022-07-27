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
 * @brief  synchronized memory file interface
**/

#pragma once

#include "readwrite/ecal_writer_base.h"
#include "io/ecal_memfile.h"

#include <ecal/ecal_eventhandle.h>

#include <mutex>
#include <string>
#include <unordered_map>

namespace eCAL
{
  class CSyncMemoryFile
  {
  public:
    CSyncMemoryFile(const std::string& base_name_, size_t size_, int timeout_open_ms, int timeout_ack_ms);
    ~CSyncMemoryFile();

    bool Connect(const std::string& process_id_);
    bool Disconnect(const std::string& process_id_);

    bool CheckSize(size_t size_);
    bool Write(const CDataWriterBase::SWriterData& data_);

    std::string GetName();

  protected:
    bool Create(const std::string& base_name_, size_t size_);
    bool Destroy();
    bool Recreate(size_t size_);

    void BuildMemFileName();

    void SendSyncEvents();
    void DisconnectAll();

    std::string      m_base_name;
    std::string      m_memfile_name;
    CMemoryFile      m_memfile;
    int              m_timeout_open;
    int              m_timeout_ack;
    bool             m_created;

    struct SEventHandlePair
    {
      EventHandleT event_snd;
      EventHandleT event_ack;
    };
    typedef std::unordered_map<std::string, SEventHandlePair> EventHandleMapT;
    std::mutex       m_event_handle_map_sync;
    EventHandleMapT  m_event_handle_map;
  };
}

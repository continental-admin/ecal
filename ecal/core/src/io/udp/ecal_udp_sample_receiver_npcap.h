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
 * @brief  UDP sample receiver to receive messages of type eCAL::Sample
**/

#pragma once

#include "io/udp/ecal_udp_sample_receiver_base.h"

#include <ecaludp/socket_npcap.h>
#include <memory>
#include <thread>

namespace eCAL
{
  namespace UDP
  {
    class CSampleReceiverNpcap : public CSampleReceiverBase
    {
    public:
      CSampleReceiverNpcap(const SReceiverAttr& attr_, const HasSampleCallbackT& has_sample_callback_, const ApplySampleCallbackT& apply_sample_callback_);
      virtual ~CSampleReceiverNpcap();

      bool AddMultiCastGroup(const char* ipaddr_);
      bool RemMultiCastGroup(const char* ipaddr_);

      // prevent copying and moving
      CSampleReceiverNpcap(const CSampleReceiverNpcap&) = delete;
      CSampleReceiverNpcap& operator=(const CSampleReceiverNpcap&) = delete;
      CSampleReceiverNpcap(CSampleReceiverNpcap&&) = delete;
      CSampleReceiverNpcap& operator=(CSampleReceiverNpcap&&) = delete;

    private:
      void InitializeSocket(const SReceiverAttr& attr_);
      void Receive();

      std::unique_ptr<asio::io_context>       m_io_context;
      std::unique_ptr<asio::io_context::work> m_work;
      std::unique_ptr<ecaludp::SocketNpcap>   m_socket;

      asio::ip::udp::endpoint                 m_sender_endpoint;
      std::thread                             m_io_thread;
    };
  }
}

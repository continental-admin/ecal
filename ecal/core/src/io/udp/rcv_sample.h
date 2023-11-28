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
 * @brief  UDP sample receiver to receive messages of type eCAL::pb::Sample
**/

#pragma once

#include "ecal_def.h"
#include "udp_receiver.h"
#include "rcv_sample_slot.h"
#include "util/ecal_thread.h"

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/ecal.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class CUDPSampleReceiver
{
  class CSampleReceiveSlot : public CReceiveSlot
  {
  public:
    explicit CSampleReceiveSlot(CUDPSampleReceiver* sample_receiver_);
    ~CSampleReceiveSlot() override;

    int OnMessageCompleted(std::vector<char> &&msg_buffer_) override;

  protected:
    CUDPSampleReceiver* m_sample_receiver;
    eCAL::pb::Sample    m_ecal_sample;
  };

public:
  using HasSampleCallbackT   = std::function<bool(const std::string& sample_name_)>;
  using ApplySampleCallbackT = std::function<void(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType layer_)>;

  CUDPSampleReceiver(const eCAL::SReceiverAttr& attr_, HasSampleCallbackT has_sample_callback_, ApplySampleCallbackT apply_sample_callback_);
  virtual ~CUDPSampleReceiver();

  bool AddMultiCastGroup(const char* ipaddr_);
  bool RemMultiCastGroup(const char* ipaddr_);

protected:
  void ReceiveThread();
  void Process(const char* sample_buffer_, size_t sample_buffer_len_);

  HasSampleCallbackT                     m_has_sample_callback;
  ApplySampleCallbackT                   m_apply_sample_callback;

  eCAL::CUDPReceiver                     m_udp_receiver;
  std::shared_ptr<eCAL::CallbackThread>  m_udp_receiver_thread;

  using ReceiveSlotMapT = std::unordered_map<int32_t, std::shared_ptr<CSampleReceiveSlot>>;
  ReceiveSlotMapT                        m_receive_slot_map;
  std::vector<char>                      m_msg_buffer;
  eCAL::pb::Sample                       m_ecal_sample;

  std::chrono::steady_clock::time_point  m_cleanup_start;
};

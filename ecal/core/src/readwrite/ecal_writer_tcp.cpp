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
 * @brief  tcp writer
**/

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/layer.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include <ecal/ecal_log.h>
#include "readwrite/ecal_writer_tcp.h"

namespace eCAL
{
  std::shared_ptr<tcpub::Executor> CDataWriterTCP::m_executor;

  CDataWriterTCP::CDataWriterTCP() : m_port(0)
  {
  }

  CDataWriterTCP::~CDataWriterTCP()
  {
    Destroy();
  }

  void CDataWriterTCP::GetInfo(SWriterInfo info_)
  {
    info_.name                 = "tcp";
    info_.description          = "tcp data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = true;

    info_.has_qos_history_kind = false;
    info_.has_qos_reliability  = false;

    info_.send_size_max        = -1;
  }

  bool CDataWriterTCP::Create(const std::string& host_name_, const std::string& topic_name_, const std::string& topic_id_)
  {
    if (!m_executor) m_executor = std::make_shared<tcpub::Executor>(4);

    // create publisher
    m_publisher = std::make_shared<tcpub::Publisher>(m_executor);
    m_port      = m_publisher->getPort();

    // writer parameter
    m_host_name  = host_name_;
    m_topic_name = topic_name_;
    m_topic_id   = topic_id_;

    return true;
  }

  bool CDataWriterTCP::Destroy()
  {
    if(!m_publisher) return false;

    // destroy publisher
    m_publisher = nullptr;
    m_port      = 0;

    return true;
  }

  size_t CDataWriterTCP::Write(const SWriterData& data_)
  {
    if (!m_publisher) return 0;

    // create new sample
    m_ecal_sample.Clear();
    m_ecal_sample.set_cmd_type(eCAL::pb::bct_set_sample);
    auto ecal_sample_mutable_topic = m_ecal_sample.mutable_topic();
    ecal_sample_mutable_topic->set_hname(m_host_name);
    ecal_sample_mutable_topic->set_tname(m_topic_name);
    ecal_sample_mutable_topic->set_tid(m_topic_id);

    // set layer
    auto layer = ecal_sample_mutable_topic->add_tlayer();
    layer->set_type(eCAL::pb::eTLayerType::tl_ecal_tcp);
    layer->set_confirmed(true);

    // append content
    auto ecal_sample_mutable_content = m_ecal_sample.mutable_content();
    ecal_sample_mutable_content->set_id(data_.id);
    ecal_sample_mutable_content->set_clock(data_.clock);
    ecal_sample_mutable_content->set_time(data_.time);
    ecal_sample_mutable_content->set_hash(data_.hash);
    ecal_sample_mutable_content->set_size((google::protobuf::int32)data_.len);
    ecal_sample_mutable_content->set_payload(data_.buf, data_.len);

    size_t size(0);
#if GOOGLE_PROTOBUF_VERSION >= 3001000
    size = (size_t)m_ecal_sample.ByteSizeLong();
#else
    size = (size_t)m_ecal_sample.ByteSize();
#endif

    m_sample_buffer.resize(size);
    m_ecal_sample.SerializeToArray((void*)m_sample_buffer.data(), (int)size);

    if (m_publisher->send(m_sample_buffer.data(), m_sample_buffer.size()))
    {
      return m_sample_buffer.size();
    }
    else
    {
      return 0;
    }
  }

  std::string CDataWriterTCP::GetConnectionParameter()
  {
    // set tcp port
    eCAL::pb::ConnnectionPar connection_par;
    connection_par.mutable_layer_par_tcp()->set_port(m_port);
    return connection_par.SerializeAsString();
  }
}

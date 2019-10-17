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
 * @brief  Global monitoring class (implementation)
**/

#include <ecal/ecal.h>

#include <ecal/ecal_core.h>

#include "ecal_config_hlp.h"
#include "ecal_monitoring_impl.h"

#include "ecal_def.h"

#include <regex>
#include <sstream>

namespace eCAL
{
  ////////////////////////////////////////////////////////
  // local helper
  ////////////////////////////////////////////////////////
  static void GetSampleHost(const eCAL::pb::Sample& ecal_sample_, std::string& host_name_)
  {
    if (ecal_sample_.has_host())
    {
      host_name_ = ecal_sample_.host().hname();
    }
    if (ecal_sample_.has_process())
    {
      host_name_ = ecal_sample_.process().hname();
    }
    if (ecal_sample_.has_service())
    {
      host_name_ = ecal_sample_.service().hname();
    }
    if (ecal_sample_.has_topic())
    {
      host_name_ = ecal_sample_.topic().hname();
    }
  }

  static bool IsLocalHost(const eCAL::pb::Sample& ecal_sample_)
  {
    std::string host_name;
    GetSampleHost(ecal_sample_, host_name);
    if (host_name.empty())                   return(false);
    if (host_name == Process::GetHostName()) return(true);
    return(false);
  }


  ////////////////////////////////////////
  // Monitoring Implementation
  ////////////////////////////////////////
  CMonitoringImpl::CMonitoringImpl() :
    m_init(false),
    m_network       (eCALPAR(NET, ENABLED)),
    m_publisher_map (std::chrono::milliseconds(eCALPAR(MON, TIMEOUT))),
    m_subscriber_map(std::chrono::milliseconds(eCALPAR(MON, TIMEOUT))),
    m_process_map   (std::chrono::milliseconds(eCALPAR(MON, TIMEOUT))),
    m_service_map   (std::chrono::milliseconds(eCALPAR(MON, TIMEOUT)))
  {
  }

  CMonitoringImpl::~CMonitoringImpl()
  {
  }

  void CMonitoringImpl::Create()
  {
    if (m_init) return;

    // network mode
    m_network = eCALPAR(NET, ENABLED);

    // get name of this host
    m_host_name = Process::GetHostName();

    // start registration receive thread
    CRegistrationReceiveThread::RegMessageCallbackT regmsg_cb = std::bind(&CSampleReceiver::Receive, this, std::placeholders::_1);
    m_reg_rcv_threadcaller = std::make_shared<CRegistrationReceiveThread>(regmsg_cb);

    // start logging receive thread
    CLoggingReceiveThread::LogMessageCallbackT logmsg_cb = std::bind(&CMonitoringImpl::RegisterLogMessage, this, std::placeholders::_1);
    m_log_rcv_threadcaller = std::make_shared<CLoggingReceiveThread>(logmsg_cb);
    m_log_rcv_threadcaller->SetNetworkMode(eCALPAR(NET, ENABLED));

    // start monitoring and logging publishing thread
    CMonLogPublishingThread::MonitoringCallbackT mon_cb = std::bind(&CMonitoringImpl::GetMonitoringMsg, this, std::placeholders::_1);
    CMonLogPublishingThread::LoggingCallbackT    log_cb = std::bind(&CMonitoringImpl::GetLoggingMsg, this, std::placeholders::_1);
    m_pub_threadcaller = std::make_shared<CMonLogPublishingThread>(mon_cb, log_cb);

    // setup blacklist and whitelist filter strings#
    m_topic_filter_excl_s = eCALPAR(MON, FILTER_EXCL);
    m_topic_filter_incl_s = eCALPAR(MON, FILTER_INCL);

    // setup filtering on by default
    SetFilterState(true);

    m_init = true;
  }

  void CMonitoringImpl::Destroy()
  {
    m_init = false;
  }

  void CMonitoringImpl::SetExclFilter(const std::string& filter_)
  {
    m_topic_filter_excl_s = filter_;
  }

  void CMonitoringImpl::SetInclFilter(const std::string& filter_)
  {
    m_topic_filter_incl_s = filter_;
  }

  void CMonitoringImpl::SetFilterState(bool state_)
  {
    if (state_)
    {
      // create excluding filter list
      {
        std::lock_guard<std::mutex> lock(m_topic_filter_excl_mtx);
        Tokenize(m_topic_filter_excl_s, m_topic_filter_excl, ",;", true);
      }

      // create including filter list
      {
        std::lock_guard<std::mutex> lock(m_topic_filter_incl_mtx);
        Tokenize(m_topic_filter_incl_s, m_topic_filter_incl, ",;", true);
      }
    }
    else
    {
      {
        std::lock_guard<std::mutex> lock(m_topic_filter_excl_mtx);
        m_topic_filter_excl.clear();
      }
      {
        std::lock_guard<std::mutex> lock(m_topic_filter_incl_mtx);
        m_topic_filter_incl.clear();
      }
    }
  }

  size_t CMonitoringImpl::ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType /*layer_*/)
  {
    // if sample is from outside and we are in local network mode
    // do not process sample
    if (!IsLocalHost(ecal_sample_) && !m_network) return 0;

    switch (ecal_sample_.cmd_type())
    {
    case eCAL::pb::bct_none:
    case eCAL::pb::bct_set_sample:
      break;
    case eCAL::pb::bct_reg_process:
    {
      // register process
      RegisterProcess(ecal_sample_);
    }
    break;
    case eCAL::pb::bct_reg_service:
    {
      // register service
      RegisterService(ecal_sample_);
    }
    break;
    case eCAL::pb::bct_reg_publisher:
    {
      // register publisher
      RegisterTopic(ecal_sample_, CMonitoringImpl::publisher);
    }
    break;
    case eCAL::pb::bct_reg_subscriber:
    {
      // register subscriber
      RegisterTopic(ecal_sample_, CMonitoringImpl::subscriber);
    }
    break;
    default:
    {
      eCAL::Logging::Log(log_level_warning, "CMonitoringImpl::ApplySample : unknown sample type");
    }
    break;
    }
    return 0;
  }

  bool CMonitoringImpl::RegisterTopic(const eCAL::pb::Sample& sample_, enum ePubSub pubsub_type_)
  {
    auto sample_topic = sample_.topic();
    int          process_id      = sample_topic.pid();
    std::string  topic_name      = sample_topic.tname();
    size_t       topic_size      = static_cast<size_t>(sample_topic.tsize());
    bool         topic_tlayer_ecal_udp_mc(false);
    bool         topic_tlayer_ecal_udp_uc(false);
    bool         topic_tlayer_ecal_udp_metal(false);
    bool         topic_tlayer_ecal_shm(false);
    bool         topic_tlayer_lcm(false);
    bool         topic_tlayer_rtps(false);
    bool         topic_tlayer_inproc(false);
    for (auto layer : sample_topic.tlayer())
    {
      topic_tlayer_ecal_udp_mc    |= (layer.type() == eCAL::pb::tl_ecal_udp_mc)    && layer.confirmed();
      topic_tlayer_ecal_udp_uc    |= (layer.type() == eCAL::pb::tl_ecal_udp_uc)    && layer.confirmed();
      topic_tlayer_ecal_udp_metal |= (layer.type() == eCAL::pb::tl_ecal_udp_metal) && layer.confirmed();
      topic_tlayer_ecal_shm       |= (layer.type() == eCAL::pb::tl_ecal_shm)       && layer.confirmed();
      topic_tlayer_lcm            |= (layer.type() == eCAL::pb::tl_lcm)            && layer.confirmed();
      topic_tlayer_rtps           |= (layer.type() == eCAL::pb::tl_rtps)           && layer.confirmed();
      topic_tlayer_inproc         |= (layer.type() == eCAL::pb::tl_inproc)         && layer.confirmed();
    }
    size_t       connections_loc = static_cast<size_t>(sample_topic.connections_loc());
    size_t       connections_ext = static_cast<size_t>(sample_topic.connections_ext());
    long long    did             = sample_topic.did();
    long long    dclock          = sample_topic.dclock();
    long long    ddropped        = sample_topic.message_drops();
    long         dfreq           = sample_topic.dfreq();
    long         dfreq_min       = sample_topic.dfreq_min();
    long         dfreq_max       = sample_topic.dfreq_max();
    long         dfreq_min_err   = sample_topic.dfreq_min_err();
    long         dfreq_max_err   = sample_topic.dfreq_max_err();

    // check blacklist topic filter
    {
      std::lock_guard<std::mutex> lock(m_topic_filter_excl_mtx);
      for (const auto& it : m_topic_filter_excl)
      {
        if (std::regex_match(topic_name, std::regex(it, std::regex::icase)))
          return(false);
      }
    }

    // check whitelist topic filter
    bool is_topic_in_filter(false);
    {
      std::lock_guard<std::mutex> lock(m_topic_filter_incl_mtx);
      is_topic_in_filter = m_topic_filter_incl.empty();
      for (const auto& it : m_topic_filter_incl)
      {
        if (std::regex_match(topic_name, std::regex(it, std::regex::icase)))
        {
          is_topic_in_filter = true;
          break;
        }
      }
    }

    if (is_topic_in_filter == false) return (false);

    /////////////////////////////////
    // register in topic map
    /////////////////////////////////
    STopicMonMap* pTopicMap = GetMap(pubsub_type_);
    if (pTopicMap)
    {
      // acquire access
      std::lock_guard<std::mutex> lock(pTopicMap->sync);

      // common infos
      std::string host_name    = sample_topic.hname();
      std::string process_name = sample_topic.pname();
      std::string unit_name    = sample_topic.uname();
      std::string topic_id     = sample_topic.tid();
      std::string topic_type   = sample_topic.ttype();
      std::string topic_desc   = sample_topic.tdesc();

      // try to get topic info
      std::string topic_name_id = topic_name + topic_id;
      STopicMon& TopicInfo = (*pTopicMap->map)[topic_name_id];

      // set static content
      TopicInfo.hname  = std::move(host_name);
      TopicInfo.pid    = process_id;
      TopicInfo.pname  = std::move(process_name);
      TopicInfo.uname  = std::move(unit_name);
      TopicInfo.tname  = std::move(topic_name);
      TopicInfo.tid    = std::move(topic_id);

      // update flexible content
      TopicInfo.rclock++;
      TopicInfo.ttype                 = std::move(topic_type);
      TopicInfo.tdesc                 = std::move(topic_desc);
      TopicInfo.tlayer_ecal_udp_mc    = topic_tlayer_ecal_udp_mc;
      TopicInfo.tlayer_ecal_udp_uc    = topic_tlayer_ecal_udp_uc;
      TopicInfo.tlayer_ecal_udp_metal = topic_tlayer_ecal_udp_metal;
      TopicInfo.tlayer_ecal_shm       = topic_tlayer_ecal_shm;
      TopicInfo.tlayer_lcm            = topic_tlayer_lcm;
      TopicInfo.tlayer_rtps           = topic_tlayer_rtps;
      TopicInfo.tlayer_inproc         = topic_tlayer_inproc;
      TopicInfo.tsize                 = static_cast<int>(topic_size);
      TopicInfo.connections_loc       = static_cast<int>(connections_loc);
      TopicInfo.connections_ext       = static_cast<int>(connections_ext);
      TopicInfo.did                   = did;
      TopicInfo.dclock                = dclock;
      TopicInfo.ddropped              = ddropped;
      TopicInfo.dfreq                 = dfreq;
      TopicInfo.dfreq_min             = dfreq_min;
      TopicInfo.dfreq_max             = dfreq_max;
      TopicInfo.dfreq_min_err         = dfreq_min_err;
      TopicInfo.dfreq_max_err         = dfreq_max_err;
    }

    return(true);
  }

  bool CMonitoringImpl::RegisterProcess(const eCAL::pb::Sample& sample_)
  {
    auto sample_process = sample_.process();
    std::string     host_name                    = sample_process.hname();
    std::string     process_name                 = sample_process.pname();
    int             process_id                   = sample_process.pid();
    std::string     process_param                = sample_process.pparam();
    std::string     unit_name                    = sample_process.uname();
    long long       process_memory               = sample_process.pmemory();
    float           process_cpu                  = sample_process.pcpu();
    float           process_usrptime             = sample_process.usrptime();
    long long       process_datawrite            = sample_process.datawrite();
    long long       process_dataread             = sample_process.dataread();
    auto            sample_process_state         = sample_process.state();
    int             process_state_severity       = sample_process_state.severity();
    int             process_state_severity_level = sample_process_state.severity_level();
    std::string     process_state_info           = sample_process_state.info();
    int             process_tsync_mode           = sample_process.tsync_mode();

    std::stringstream process_id_ss;
    process_id_ss << process_id;
    std::string process_name_id = process_name + process_id_ss.str();

    // acquire access
    std::lock_guard<std::mutex> lock(m_process_map.sync);

    // try to get process info
    SProcessMon& ProcessInfo = (*m_process_map.map)[process_name_id];

    // set static content
    ProcessInfo.hname  = std::move(host_name);
    ProcessInfo.pname  = std::move(process_name);
    ProcessInfo.uname  = std::move(unit_name);
    ProcessInfo.pid    = process_id;
    ProcessInfo.pparam = std::move(process_param);

    // update flexible content
    ProcessInfo.rclock++;
    ProcessInfo.pmemory              = process_memory;
    ProcessInfo.pcpu                 = process_cpu;
    ProcessInfo.usrptime             = process_usrptime;
    ProcessInfo.datawrite            = process_datawrite;
    ProcessInfo.dataread             = process_dataread;
    ProcessInfo.state_severity       = process_state_severity;
    ProcessInfo.state_severity_level = process_state_severity_level;
    ProcessInfo.state_info           = std::move(process_state_info);
    ProcessInfo.tsync_mode           = process_tsync_mode;

    return(true);
  }

  bool CMonitoringImpl::RegisterService(const eCAL::pb::Sample& sample_)
  {
    auto sample_service = sample_.service();
    std::string  host_name    = sample_service.hname();
    std::string  service_name = sample_service.sname();
    std::string  process_name = sample_service.pname();
    std::string  unit_name    = sample_service.uname();
    int          process_id   = sample_service.pid();
    int          tcp_port     = sample_service.tcp_port();

    std::stringstream process_id_ss;
    process_id_ss << process_id;
    std::string service_name_id = service_name + process_id_ss.str();

    // acquire access
    std::lock_guard<std::mutex> lock(m_service_map.sync);

    // try to get service info
    SServiceMon& ServiceInfo = (*m_service_map.map)[service_name_id];

    // set static content
    ServiceInfo.hname    = std::move(host_name);
    ServiceInfo.sname    = std::move(service_name);
    ServiceInfo.pname    = std::move(process_name);
    ServiceInfo.uname    = std::move(unit_name);
    ServiceInfo.pid      = process_id;
    ServiceInfo.tcp_port = tcp_port;

    // update flexible content
    ServiceInfo.rclock++;
    ServiceInfo.methods.clear();
    for (int i = 0; i < sample_.service().methods_size(); ++i)
    {
      struct SMethodMon method;
      auto sample_service_methods = sample_.service().methods(i);
      method.mname      = sample_service_methods.mname();
      method.req_type   = sample_service_methods.req_type();
      method.resp_type  = sample_service_methods.resp_type();
      method.call_count = sample_service_methods.call_count();
      ServiceInfo.methods.push_back(method);
    }

    return(true);
  }

  void CMonitoringImpl::RegisterLogMessage(const eCAL::pb::LogMessage& log_msg_)
  {
    std::lock_guard<std::mutex> lock(m_log_msglist_sync);
    m_log_msglist.emplace_back(log_msg_);
  }

  CMonitoringImpl::STopicMonMap* CMonitoringImpl::GetMap(enum ePubSub pubsub_type_)
  {
    STopicMonMap* pHostMap = nullptr;
    switch (pubsub_type_)
    {
    case publisher:
      pHostMap = &m_publisher_map;
      break;
    case subscriber:
      pHostMap = &m_subscriber_map;
      break;
    }
    return(pHostMap);
  };

  void CMonitoringImpl::GetMonitoringMsg(eCAL::pb::Monitoring& monitoring_)
  {
    // clear protobuf object
    monitoring_.Clear();

    // write all registrations to monitoring message object
    MonitorProcs(monitoring_);
    MonitorServices(monitoring_);
    MonitorTopics(m_publisher_map, monitoring_, "publisher");
    MonitorTopics(m_subscriber_map, monitoring_, "subscriber");
  }

  void CMonitoringImpl::GetLoggingMsg(eCAL::pb::Logging& logging_)
  {
    // clear protobuf object
    logging_.Clear();

    // acquire access
    std::lock_guard<std::mutex> lock(m_log_msglist_sync);

    LogMessageListT::const_iterator siter = m_log_msglist.begin();
    while (siter != m_log_msglist.end())
    {
      // add log message
      eCAL::pb::LogMessage* pMonLogMessage = logging_.add_logs();

      // copy content
      pMonLogMessage->CopyFrom(*siter);

      ++siter;
    }

    // empty message list
    m_log_msglist.clear();
  }

  int CMonitoringImpl::PubMonitoring(bool state_, std::string & name_)
  {
    // (de)activate monitor publisher
    m_pub_threadcaller->SetMonState(state_, name_);
    return 0;
  }

  int CMonitoringImpl::PubLogging(bool state_, std::string & name_)
  {
    // (de)activate logging publisher
    m_pub_threadcaller->SetLogState(state_, name_);
    return 0;
  }

  void CMonitoringImpl::MonitorProcs(eCAL::pb::Monitoring& monitoring_)
  {
    // acquire access
    std::lock_guard<std::mutex> lock(m_process_map.sync);

    // iterate map
    m_process_map.map->remove_deprecated();
    for (auto process : (*m_process_map.map))
    {
      // add host
      eCAL::pb::Process* pMonProcs = monitoring_.add_processes();

      // registration clock
      pMonProcs->set_rclock(process.second.rclock);

      // host name
      pMonProcs->set_hname(process.second.hname);

      // process name
      pMonProcs->set_pname(process.second.pname);

      // unit name
      pMonProcs->set_uname(process.second.uname);

      // process id
      pMonProcs->set_pid(process.second.pid);

      // process parameter
      pMonProcs->set_pparam(process.second.pparam);

      // process memory
      pMonProcs->set_pmemory(process.second.pmemory);

      // process cpu
      pMonProcs->set_pcpu(process.second.pcpu);

      // process user core time
      pMonProcs->set_usrptime(process.second.usrptime);

      // process data write bytes
      pMonProcs->set_datawrite(process.second.datawrite);

      // process data read bytes
      pMonProcs->set_dataread(process.second.dataread);

      // state
      auto state = pMonProcs->mutable_state();

      // severity state
      state->set_severity(eCAL::pb::eProcessSeverity(process.second.state_severity));

      // severity level
      state->set_severity_level(eCAL::pb::eProcessSeverityLevel(process.second.state_severity_level));

      // severity info
      state->set_info(process.second.state_info);

      // time synchronization mode
      pMonProcs->set_tsync_mode(eCAL::pb::eTSyncState(process.second.tsync_mode));
    }
  }

  void CMonitoringImpl::MonitorServices(eCAL::pb::Monitoring& monitoring_)
  {
    // acquire access
    std::lock_guard<std::mutex> lock(m_service_map.sync);

    // iterate map
    m_service_map.map->remove_deprecated();
    for (auto service : (*m_service_map.map))
    {
      // add host
      eCAL::pb::Service* pMonService = monitoring_.add_services();

      // registration clock
      pMonService->set_rclock(service.second.rclock);

      // host name
      pMonService->set_hname(service.second.hname);

      // process name
      pMonService->set_pname(service.second.pname);

      // unit name
      pMonService->set_uname(service.second.uname);

      // process id
      pMonService->set_pid(service.second.pid);

      // service name
      pMonService->set_sname(service.second.sname);

      // tcp port
      pMonService->set_tcp_port(service.second.tcp_port);

      // methods
      for (auto method : service.second.methods)
      {
        eCAL::pb::Method* pMonMethod = pMonService->add_methods();
        pMonMethod->set_mname(method.mname);
        pMonMethod->set_req_type(method.req_type);
        pMonMethod->set_resp_type(method.resp_type);
        pMonMethod->set_call_count(method.call_count);
      }
    }
  }

  void CMonitoringImpl::MonitorTopics(STopicMonMap& map_, eCAL::pb::Monitoring& monitoring_, const std::string& direction_)
  {
    // acquire access
    std::lock_guard<std::mutex> lock(map_.sync);

    // iterate map
    map_.map->remove_deprecated();
    for (auto topic : (*map_.map))
    {
      // add topic
      eCAL::pb::Topic* pMonTopic = monitoring_.add_topics();

      // registration clock
      pMonTopic->set_rclock(topic.second.rclock);

      // host name
      pMonTopic->set_hname(topic.second.hname);

      // process id
      pMonTopic->set_pid(topic.second.pid);

      // process name
      pMonTopic->set_pname(topic.second.pname);

      // unit name
      pMonTopic->set_uname(topic.second.uname);

      // topic id
      pMonTopic->set_tid(topic.second.tid);

      // topic name
      pMonTopic->set_tname(topic.second.tname);

      // direction
      pMonTopic->set_direction(direction_);

      // topic type
      pMonTopic->set_ttype(topic.second.ttype);

      // topic transport layers
      if (topic.second.tlayer_ecal_udp_mc)
      {
        auto tlayer = pMonTopic->add_tlayer();
        tlayer->set_type(eCAL::pb::tl_ecal_udp_mc);
        tlayer->set_confirmed(true);
      }
      if (topic.second.tlayer_ecal_udp_uc)
      {
        auto tlayer = pMonTopic->add_tlayer();
        tlayer->set_type(eCAL::pb::tl_ecal_udp_uc);
        tlayer->set_confirmed(true);
      }
      if (topic.second.tlayer_ecal_udp_metal)
      {
        auto tlayer = pMonTopic->add_tlayer();
        tlayer->set_type(eCAL::pb::tl_ecal_udp_metal);
        tlayer->set_confirmed(true);
      }
      if (topic.second.tlayer_ecal_shm)
      {
        auto tlayer = pMonTopic->add_tlayer();
        tlayer->set_type(eCAL::pb::tl_ecal_shm);
        tlayer->set_confirmed(true);
      }
      if (topic.second.tlayer_lcm)
      {
        auto tlayer = pMonTopic->add_tlayer();
        tlayer->set_type(eCAL::pb::tl_lcm);
        tlayer->set_confirmed(true);
      }
      if (topic.second.tlayer_rtps)
      {
        auto tlayer = pMonTopic->add_tlayer();
        tlayer->set_type(eCAL::pb::tl_rtps);
        tlayer->set_confirmed(true);
      }
      if (topic.second.tlayer_inproc)
      {
        auto tlayer = pMonTopic->add_tlayer();
        tlayer->set_type(eCAL::pb::tl_inproc);
        tlayer->set_confirmed(true);
      }

      // topic description
      pMonTopic->set_tdesc(topic.second.tdesc);

      // topic size
      pMonTopic->set_tsize(topic.second.tsize);

      // local connections
      pMonTopic->set_connections_loc(topic.second.connections_loc);

      // external connections
      pMonTopic->set_connections_ext(topic.second.connections_ext);

      // data id (publisher setid)
      pMonTopic->set_did(topic.second.did);

      // data clock
      pMonTopic->set_dclock(topic.second.dclock);

      // data dropped
      pMonTopic->set_message_drops(google::protobuf::int32(topic.second.ddropped));

      // data frequency
      pMonTopic->set_dfreq(topic.second.dfreq);

      // data frequency minimum
      pMonTopic->set_dfreq_min(topic.second.dfreq_min);

      // data frequency maximum
      pMonTopic->set_dfreq_max(topic.second.dfreq_max);

      // data frequency minimum violation error counter
      pMonTopic->set_dfreq_min_err(topic.second.dfreq_min_err);

      // data frequency maximum violation error counter
      pMonTopic->set_dfreq_max_err(topic.second.dfreq_max_err);
    }
  }

  void CMonitoringImpl::Tokenize(const std::string& str, StrICaseSetT& tokens, const std::string& delimiters, bool trimEmpty)
  {
    std::string::size_type pos, lastPos = 0;

    for (;;)
    {
      pos = str.find_first_of(delimiters, lastPos);
      if (pos == std::string::npos)
      {
        pos = str.length();
        if (pos != lastPos || !trimEmpty)
        {
          tokens.insert(std::string(str.data() + lastPos, pos - lastPos));
        }
        break;
      }
      else
      {
        if (pos != lastPos || !trimEmpty)
        {
          tokens.insert(std::string(str.data() + lastPos, pos - lastPos));
        }
      }
      lastPos = pos + 1;
    }
  }
}

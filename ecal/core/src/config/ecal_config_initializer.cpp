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
 * @brief  Functions for setting default config values
**/

#include "ecal/types/ecal_config_types.h"
#include "ecal_config_reader.h"

#include "ecal_global_accessors.h"
#include "ecal_def.h"
#include "config/ecal_config_reader.h"

constexpr char COMMON[]       = "common";
constexpr char MONITORING[]   = "monitoring";
constexpr char NETWORK[]      = "network";
constexpr char EXPERIMENTAL[] = "experimental";
constexpr char PUBLISHER[]    = "publisher";
constexpr char SYS[]          = "sys";
constexpr char TIME[]         = "time";
constexpr char SERVICE[]      = "service";
constexpr char PROCESS[]      = "process";

namespace {
  void tokenize(const std::string& str, std::vector<std::string>& tokens,
      const std::string& delimiters = " ", bool trimEmpty = false)
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
          tokens.emplace_back(std::string(str.data() + lastPos, pos - lastPos));
        }
        break;
      }
      else
      {
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace_back(std::string(str.data() + lastPos, pos - lastPos));
        }
      }
      lastPos = pos + 1;
    }
  }

  eCAL_Logging_Filter ParseLogLevel(const std::string& filter_)
  {
    // tokenize it
    std::vector<std::string> token_filter_;
    tokenize(filter_, token_filter_, " ,;");
    // create excluding filter list
    char filter_mask = log_level_none;
    for (auto& it : token_filter_)
    {
      if (it == "all")     filter_mask |= log_level_all;
      if (it == "info")    filter_mask |= log_level_info;
      if (it == "warning") filter_mask |= log_level_warning;
      if (it == "error")   filter_mask |= log_level_error;
      if (it == "fatal")   filter_mask |= log_level_fatal;
      if (it == "debug1")  filter_mask |= log_level_debug1;
      if (it == "debug2")  filter_mask |= log_level_debug2;
      if (it == "debug3")  filter_mask |= log_level_debug3;
      if (it == "debug4")  filter_mask |= log_level_debug4;
    }
    return(filter_mask);
  };
}

namespace eCAL 
{
  namespace Config
  {
    eCALConfig GetDefaultConfig(eCALConfig& config)
    {
      // transport layer options
      auto& transportLayerOptions = config.transport_layer_options;
      transportLayerOptions.network_enabled            = NET_ENABLED;
      transportLayerOptions.drop_out_of_order_messages = EXP_DROP_OUT_OF_ORDER_MESSAGES;
      
      auto& multicastOptions = transportLayerOptions.mc_options;
      multicastOptions.config_version      = NET_UDP_MULTICAST_CONFIG_VERSION;
      multicastOptions.group			         = std::string(NET_UDP_MULTICAST_GROUP);
      multicastOptions.mask                = std::string(NET_UDP_MULTICAST_MASK);
      multicastOptions.port			           = NET_UDP_MULTICAST_PORT;
      multicastOptions.ttl                 = NET_UDP_MULTICAST_TTL;
      multicastOptions.recbuf              = NET_UDP_MULTICAST_RCVBUF;
      multicastOptions.sndbuf              = NET_UDP_MULTICAST_SNDBUF;
      multicastOptions.join_all_interfaces = NET_UDP_MULTICAST_JOIN_ALL_IF_ENABLED;
      multicastOptions.bandwidth_max_udp   = NET_BANDWIDTH_MAX_UDP;
      multicastOptions.npcap_enabled       = NET_NPCAP_ENABLED;

      auto& tcpPubSubOptions = transportLayerOptions.tcp_options;
      tcpPubSubOptions.num_executor_reader = NET_TCP_PUBSUB_NUM_EXECUTOR_READER;
      tcpPubSubOptions.num_executor_writer = NET_TCP_PUBSUB_NUM_EXECUTOR_WRITER;
      tcpPubSubOptions.max_reconnections   = NET_TCP_PUBSUB_MAX_RECONNECTIONS;

      auto& shmOptions = transportLayerOptions.shm_options;
      shmOptions.host_group_name		        = NET_HOST_GROUP_NAME;
      shmOptions.memfile_minsize		        = PUB_MEMFILE_MINSIZE;
      shmOptions.memfile_reserve		        = PUB_MEMFILE_RESERVE;
      shmOptions.memfile_ack_timeout        = PUB_MEMFILE_ACK_TO;
      shmOptions.memfile_buffer_count       = PUB_MEMFILE_BUF_COUNT;
      shmOptions.drop_out_of_order_messages = EXP_DROP_OUT_OF_ORDER_MESSAGES;
      shmOptions.memfile_zero_copy		      = PUB_MEMFILE_ZERO_COPY;

      // registration options
      auto& registrationOptions = config.registration_options;
      registrationOptions             = RegistrationOptions(CMN_REGISTRATION_TO, CMN_REGISTRATION_REFRESH);
      registrationOptions.share_tdesc = PUB_SHARE_TDESC;
      registrationOptions.share_ttype = PUB_SHARE_TTYPE;

      // monitoring options
      auto& monitoringOptions = config.monitoring_options;
      monitoringOptions.monitoring_mode             = EXP_MONITORING_MODE;
      monitoringOptions.monitoring_timeout          = MON_TIMEOUT;
      monitoringOptions.network_monitoring_disabled = EXP_NETWORK_MONITORING_DISABLED;
      monitoringOptions.filter_excl                 = MON_FILTER_EXCL;
      monitoringOptions.filter_incl                 = MON_FILTER_INCL;
      monitoringOptions.filter_log_con              = MON_LOG_FILTER_CON;
      monitoringOptions.filter_log_file             = MON_LOG_FILTER_FILE;
      monitoringOptions.filter_log_udp              = MON_LOG_FILTER_UDP;

      // auto& udpMonitoringOptions = monitoringOptions.udp_options;
      // TODO: Nothing here yet

      auto& shmMonitoringOptions = monitoringOptions.shm_options;
      shmMonitoringOptions.shm_monitoring_domain     = std::string(EXP_SHM_MONITORING_DOMAIN);
      shmMonitoringOptions.shm_monitoring_queue_size = EXP_SHM_MONITORING_QUEUE_SIZE;

      // receiving options
      auto& receivingOptions = config.receiving_options;
      receivingOptions.shm_recv_enabled    = NET_SHM_REC_ENABLED;
      receivingOptions.tcp_recv_enabled    = NET_TCP_REC_ENABLED;
      receivingOptions.udp_mc_recv_enabled = NET_UDP_MC_REC_ENABLED;

      // publisher options
      auto& publisherOptions = config.publisher_options;
      publisherOptions.use_shm    = PUB_USE_SHM;
      publisherOptions.use_tcp    = PUB_USE_TCP;
      publisherOptions.use_udp_mc = PUB_USE_UDP_MC;
      
      // timesync options
      auto& timesyncOptions = config.timesync_options;
      timesyncOptions.timesync_module = TIME_SYNC_MODULE;

      // service options
      auto& serviceOptions = config.service_options;
      serviceOptions.protocol_v0 = SERVICE_PROTOCOL_V0;
      serviceOptions.protocol_v1 = SERVICE_PROTOCOL_V1;

      // sys options
      auto& sysOptions = config.application_options.sys_options;
      sysOptions.filter_excl = SYS_FILTER_EXCL;

      // process options
      auto& processOptions = config.application_options.process_options;
      processOptions.terminal_emulator = PROCESS_TERMINAL_EMULATOR;

      return config;
    };

    eCALConfig GetIniConfig(eCALConfig& config)
    {
      CConfig iniConfig;
      iniConfig.AddFile(g_default_ini_file);

      // transport layer options
      auto& transportLayerOptions = config.transport_layer_options;
      transportLayerOptions.network_enabled			       = iniConfig.get(NETWORK,      "network_enabled",            false);
      transportLayerOptions.drop_out_of_order_messages = iniConfig.get(EXPERIMENTAL, "drop_out_of_order_messages", false);

      auto& multicastOptions = transportLayerOptions.mc_options;
      
      const std::string udp_config_version_string = iniConfig.get(NETWORK, "multicast_config_version", "v1");
      if (udp_config_version_string == "v1")
        multicastOptions.config_version = UdpConfigVersion::V1;
      if (udp_config_version_string == "v2")
        multicastOptions.config_version = UdpConfigVersion::V2;
      
      multicastOptions.group               = iniConfig.get(NETWORK, "multicast_group",       "239.0.0.0");
      multicastOptions.mask                = iniConfig.get(NETWORK, "multicast_mask",        "0.0.0.15");
      multicastOptions.port                = iniConfig.get(NETWORK, "multicast_port",        14000);
      multicastOptions.ttl                 = iniConfig.get(NETWORK, "multicast_ttl",         3);
      multicastOptions.recbuf              = iniConfig.get(NETWORK, "multicast_rcvbuf",      (1024 * 1024 * 5));
      multicastOptions.sndbuf              = iniConfig.get(NETWORK, "multicast_sndbuf",      (1024 * 1024 * 5));
      multicastOptions.join_all_interfaces = iniConfig.get(NETWORK, "multicast_join_all_if", false);
      multicastOptions.bandwidth_max_udp   = iniConfig.get(NETWORK, "bandwidth_max_udp",     (- 1));
      multicastOptions.npcap_enabled       = iniConfig.get(NETWORK, "npcap_enabled",         false);

      auto& tcpPubSubOptions = transportLayerOptions.tcp_options;
      tcpPubSubOptions.num_executor_reader = iniConfig.get(NETWORK, "tcp_pubsup_num_executor_reader", 4);
      tcpPubSubOptions.num_executor_writer = iniConfig.get(NETWORK, "tcp_pubsup_num_executor_writer", 4);
      tcpPubSubOptions.max_reconnections   = iniConfig.get(NETWORK, "tcp_pubsup_max_reconnections",   5);

      auto& shmOptions = transportLayerOptions.shm_options;
      shmOptions.host_group_name            = iniConfig.get(NETWORK,      "host_group_name",            "");
      shmOptions.memfile_minsize            = iniConfig.get(PUBLISHER,    "memfile_minsize",            (4 * 1024));
      shmOptions.memfile_reserve            = iniConfig.get(PUBLISHER,    "memfile_reserve",            50);
      shmOptions.memfile_ack_timeout        = iniConfig.get(PUBLISHER,    "memfile_ack_timeout",        0);
      shmOptions.memfile_buffer_count       = iniConfig.get(PUBLISHER,    "memfile_buffer_count",       1);
      shmOptions.drop_out_of_order_messages = iniConfig.get(EXPERIMENTAL, "drop_out_of_order_messages", false);
      shmOptions.memfile_zero_copy          = (iniConfig.get(PUBLISHER,   "memfile_zero_copy",          0) == 1) ? true : false;

      // registration options
      auto registrationTimeout        = iniConfig.get(COMMON,     "registration_timeout", 5000);
      auto registrationRefresh        = iniConfig.get(COMMON,     "registration_refresh", 1000);
      config.registration_options     = RegistrationOptions(registrationTimeout, registrationRefresh);
      auto& registrationOptions       = config.registration_options;
      registrationOptions.share_tdesc = (iniConfig.get(PUBLISHER, "share_tdesc",          1) == 1) ? true : false;
      registrationOptions.share_ttype = (iniConfig.get(PUBLISHER, "share_ttype",          1) == 1) ? true : false;

      // monitoring options
      auto& monitoringOptions = config.monitoring_options;
      auto  monitoringMode                          = iniConfig.get(EXPERIMENTAL, "shm_monitoring_enabled",      false) == true ? MonitoringMode::shm_monitoring : MonitoringMode::none;
      monitoringOptions.monitoring_mode             = monitoringMode;
      monitoringOptions.monitoring_timeout          = iniConfig.get(MONITORING,   "timeout", 5000);;
      monitoringOptions.network_monitoring_disabled = iniConfig.get(EXPERIMENTAL, "network_monitoring_disabled", false);
      monitoringOptions.filter_excl                 = iniConfig.get(MONITORING,   "filter_excl",                 "__.*");
      monitoringOptions.filter_incl                 = iniConfig.get(MONITORING,   "filter_incl",                 "");
      monitoringOptions.filter_log_con              = ParseLogLevel(iniConfig.get(MONITORING, "filter_log_con",  "info,warning,error,fatal"));
      monitoringOptions.filter_log_file             = ParseLogLevel(iniConfig.get(MONITORING, "filter_log_file", ""));
      monitoringOptions.filter_log_udp              = ParseLogLevel(iniConfig.get(MONITORING, "filter_log_udp",  "info,warning,error,fatal"));

      auto& udpMonitoringOptions = monitoringOptions.udp_options;
      // TODO: Nothing here yet

      auto& shmMonitoringOptions = monitoringOptions.shm_options;
      shmMonitoringOptions.shm_monitoring_domain     = iniConfig.get(EXPERIMENTAL, "shm_monitoring_domain",     "ecal_monitoring");
      shmMonitoringOptions.shm_monitoring_queue_size = iniConfig.get(EXPERIMENTAL, "shm_monitoring_queue_size", 1024);

      // receiving options
      auto& receivingOptions = config.receiving_options;
      receivingOptions.shm_recv_enabled    = iniConfig.get(NETWORK, "shm_rec_enabled",    true);
      receivingOptions.tcp_recv_enabled    = iniConfig.get(NETWORK, "tcp_rec_enabled",    true);
      receivingOptions.udp_mc_recv_enabled = iniConfig.get(NETWORK, "udp_mc_rec_enabled", true);

      // publisher options
      auto& publisherOptions = config.publisher_options;
      publisherOptions.use_inproc = TLayer::eSendMode(iniConfig.get(PUBLISHER, "use_inproc", 0));
      publisherOptions.use_shm    = TLayer::eSendMode(iniConfig.get(PUBLISHER, "use_shm",    0));
      publisherOptions.use_tcp    = TLayer::eSendMode(iniConfig.get(PUBLISHER, "use_tcp",    0));
      publisherOptions.use_udp_mc = TLayer::eSendMode(iniConfig.get(PUBLISHER, "use_udp_mc", 0));

      // timesync options
      auto& timesyncOptions = config.timesync_options;
      timesyncOptions.timesync_module = iniConfig.get(TIME, "timesync_module_rt", "");

      // service options
      auto& serviceOptions = config.service_options;
      serviceOptions.protocol_v0 = iniConfig.get(SERVICE, "protocol_v0", 1) != 0;
      serviceOptions.protocol_v1 = iniConfig.get(SERVICE, "protocol_v1", 1) != 0;

      // sys options
      auto& sysOptions = config.application_options.sys_options;
      sysOptions.filter_excl = iniConfig.get(SYS, "filter_excl", "^eCALSysClient$|^eCALSysGUI$|^eCALSys$*"); // default different than ini file

      // process options
      auto& processOptions = config.application_options.process_options;
      processOptions.terminal_emulator = iniConfig.get(PROCESS, "terminal_emulator", "");

      return config;
    };

    eCALConfig::eCALConfig()
    {
      GetIniConfig(*this);
    }

    // after initialization
    eCALConfig* GetCurrentConfig()
    {
      return g_ecal_config();
    };
  }
}
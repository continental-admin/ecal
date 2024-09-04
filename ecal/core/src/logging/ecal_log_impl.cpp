/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  eCAL logging class
**/

#include <ecal/ecal.h>
#include <ecal/ecal_os.h>
#include <ecal/ecal_config.h>

#include "ecal_log_impl.h"
#include "serialization/ecal_serialize_logging.h"
#include "builder/udp_attribute_builder.h"
#include <mutex>
#include <cstdio>

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <iomanip>
#include <ctime>
#include <chrono>

#ifdef ECAL_OS_WINDOWS
#include "ecal_win_main.h"
#include <ecal_utils/filesystem.h>

namespace
{  
  bool isDirectory(const std::string& path_)
  {
    if (path_.empty()) return false;

    return EcalUtils::Filesystem::IsDir(path_, EcalUtils::Filesystem::Current);
  }

  std::string get_time_str()
  {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::stringstream tstream;
    tstream << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
    return(tstream.str());
  }
}
#endif

#ifdef ECAL_OS_LINUX
#include <sys/stat.h>
#include <sys/time.h>

static bool isDirectory(const std::string& path_)
{
  if (path_.empty()) return false;

  struct stat st;
  if (stat(path_.c_str(), &st) == 0)
    return S_ISDIR(st.st_mode);

  return false;
}

static std::string get_time_str()
{
  char            fmt[64];
  struct timeval  tv;
  struct tm       *tm;
  gettimeofday(&tv, NULL);
  if ((tm = localtime(&tv.tv_sec)) != NULL)
  {
    strftime(fmt, sizeof fmt, "%Y-%m-%d-%H-%M-%S", tm);
  }
  return(std::string(fmt));
}
#endif

namespace
{
  void logWarningToConsole(const std::string& msg_)
  {
    std::cout << "[eCAL][Logging][Warning] " << msg_ << "\n";
  }
 
  void triggerLogging(const std::unique_ptr<spdlog::logger>& logger_, const std::string& msg_, const eCAL_Logging_eLogLevel level_)
  {
    switch(level_)
    {
      case log_level_none:
        break;
      case log_level_all:
        logger_->trace(msg_);
        break;
      case log_level_info:
        logger_->info(msg_);
        break;
      case log_level_warning:
        logger_->warn(msg_);
        break;
      case log_level_error:
        logger_->error(msg_);
        break;
      case log_level_fatal:
        logger_->critical(msg_);
        break;
      case log_level_debug1:        
      case log_level_debug2:
      case log_level_debug3:
      case log_level_debug4:
        logger_->debug(msg_);
        break;
      default:
        break;
    }
  }
}

namespace eCAL
{
  CLog::CLog(const Logging::SAttributes& attr_) :
          m_created(false),
          m_attributes(attr_)
  {
  }

  CLog::~CLog()
  {
    Stop();
  }

  void CLog::Start()
  {
    // create log file if file logging is enabled
    if(m_attributes.file.enabled)
    {
      if (isDirectory(m_attributes.file.path))
      {
        const std::string tstring = get_time_str();
      
        m_logfile_name = m_attributes.file.path + tstring + "_" + m_attributes.unit_name + "_" + std::to_string(m_attributes.process_id) + ".log";
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(m_logfile_name);        
        m_file_logger = std::make_unique<spdlog::logger>("eCAL_file_logger", file_sink);
      }
      else
      {
        logWarningToConsole("Logging for file enabled, but specified path to log is not valid: " + m_attributes.file.path);
      }

      if (m_file_logger == nullptr)
      {
        logWarningToConsole("Logging for file enabled, but file logger could not be created.");
      }   
    }

    if(m_attributes.console.enabled)
    {
      auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      m_console_logger = std::make_unique<spdlog::logger>("eCAL_console_logger", console_sink);

      if (m_console_logger == nullptr)
      {
        logWarningToConsole("Logging for console enabled, but console logger could not be created.");
      }      
    }

    if(m_attributes.udp.enabled)
    {
      // set logging send network attributes
      const eCAL::UDP::SSenderAttr attr = Logging::UDP::ConvertToIOUDPSenderAttributes(m_attributes.udp_sender);

      // create udp logging sender
      m_udp_logging_sender = std::make_unique<UDP::CSampleSender>(attr);

      if(m_udp_logging_sender == nullptr)
      {
        logWarningToConsole("Logging for UDP enabled, but sender could not be created.");
      }
    }

    // set logging receive network attributes
    const eCAL::UDP::SReceiverAttr attr = Logging::UDP::ConvertToIOUDPReceiverAttributes(m_attributes.udp_receiver);

    // start logging receiver
    m_log_receiver = std::make_shared<UDP::CSampleReceiver>(attr, std::bind(&CLog::HasSample, this, std::placeholders::_1), std::bind(&CLog::ApplySample, this, std::placeholders::_1, std::placeholders::_2));

    if(m_log_receiver == nullptr)
    {
      logWarningToConsole("Logging receiver could not be created.");
    }

    m_created = true;
  }

  void CLog::Stop()
  {
    if(!m_created) return;

    const std::lock_guard<std::mutex> lock(m_log_mtx);

    m_udp_logging_sender.reset();

    m_file_logger.reset();
    m_console_logger.reset();
    
    m_created = false;
  }

  void CLog::SetLogLevel(const eCAL_Logging_eLogLevel level_)
  {
    const std::lock_guard<std::mutex> lock(m_log_mtx);
    m_attributes.level = level_;
  }

  void CLog::SetFileLogFilter(eCAL_Logging_Filter filter_)
  {
    m_attributes.file.filter_log = filter_;
  }

  void CLog::SetUDPLogFilter(eCAL_Logging_Filter filter_)
  {
    m_attributes.udp.filter_log = filter_;
  }

  void CLog::SetConsoleLogFilter(eCAL_Logging_Filter filter_)
  {
    m_attributes.console.filter_log = filter_;
  }

  eCAL_Logging_eLogLevel CLog::GetLogLevel()
  {
    const std::lock_guard<std::mutex> lock(m_log_mtx);
    return(m_attributes.level);
  }

  void CLog::Log(const eCAL_Logging_eLogLevel level_, const std::string& msg_)
  {
    const std::lock_guard<std::mutex> lock(m_log_mtx);

    if(!m_created) return;
    if(msg_.empty()) return;

    const eCAL_Logging_Filter log_con  = level_ & m_attributes.console.filter_log;
    const eCAL_Logging_Filter log_file = level_ & m_attributes.file.filter_log;
    const eCAL_Logging_Filter log_udp  = level_ & m_attributes.udp.filter_log;
    if((log_con | log_file | log_udp) == 0) return;

    auto log_time = eCAL::Time::ecal_clock::now();

    const bool log_to_console = m_attributes.console.enabled && log_con != 0;
    const bool log_to_file    = m_attributes.file.enabled && log_file != 0;

    if(log_to_console && m_console_logger != nullptr)
    {
      triggerLogging(m_console_logger, msg_, level_);
    }

    if (log_to_file && m_file_logger != nullptr)
    {
      triggerLogging(m_file_logger, msg_, level_);
    }

    if(m_attributes.udp.enabled && log_udp != 0 && m_udp_logging_sender)
    {
        // set up log message
        Logging::SLogMessage log_message;
        log_message.time    = std::chrono::duration_cast<std::chrono::microseconds>(log_time.time_since_epoch()).count();
        log_message.hname   = m_attributes.host_name;
        log_message.pid     = m_attributes.process_id;
        log_message.pname   = m_attributes.process_name;
        log_message.uname   = m_attributes.unit_name;
        log_message.level   = level_;
        log_message.content = msg_;

        // sent it
        m_log_message_vec.clear();
        SerializeToBuffer(log_message, m_log_message_vec);
        m_udp_logging_sender->Send("_log_message_", m_log_message_vec);
    }
  }

  void CLog::Log(const std::string& msg_)
  {
    Log(m_attributes.level, msg_);
  }

  void CLog::GetLogging(std::string& log_msg_list_string_)
  {
    // clear target list string
    log_msg_list_string_.clear();

    // serialize message list
    {
      const std::lock_guard<std::mutex> lock(m_log_mtx);
      // serialize message list to target list string
      SerializeToBuffer(m_log_msglist, log_msg_list_string_);
      // clear message list
      m_log_msglist.log_messages.clear();
    }
  }

  void CLog::GetLogging(Logging::SLogging& log_)
  {
    // copy message list
    {
      const std::lock_guard<std::mutex> lock(m_log_mtx);
      log_ = m_log_msglist;
      // clear message list
      m_log_msglist.log_messages.clear();
    }
  }

  bool CLog::HasSample(const std::string& sample_name_)
  {
    return (sample_name_ == "_log_message_");
  }

  bool CLog::ApplySample(const char* serialized_sample_data_, size_t serialized_sample_size_)
  {
    // TODO: Limit maximum size of collected log messages !
    Logging::SLogMessage log_message;
    if (DeserializeFromBuffer(serialized_sample_data_, serialized_sample_size_, log_message))
    {
      // in "network mode" we accept all log messages
      // in "local mode" we accept log messages from this host only
      if ((m_attributes.host_name == log_message.hname) || m_attributes.network_enabled)
      {
        const std::lock_guard<std::mutex> lock(m_log_mtx);
        m_log_msglist.log_messages.emplace_back(log_message);
      }
      return true;
    }
    return false;
  }
}

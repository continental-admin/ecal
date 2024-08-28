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

#include <ecal/ecal_core.h>
#include <ecal/ecal_config.h>
#include <ecal/ecal_log.h>
#include <ecal/ecal_log_level.h>
#include <ecal/ecal_process.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <cstdio>
#include <thread>

constexpr std::chrono::milliseconds UDP_WAIT_TIME(1);

class CoutRedirect {
public:
    CoutRedirect(std::ostream& newStream) : originalBuffer(std::cout.rdbuf(newStream.rdbuf())) {}
    ~CoutRedirect() { std::cout.rdbuf(originalBuffer); }

private:
    std::streambuf* originalBuffer;
};

TEST(logging_sinks /*unused*/, file_logging /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "file_logging_test";
  const std::string log_message  = "Logging test for file.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.udp.enable           = false;
  ecal_config.logging.sinks.console.enable       = false;
  ecal_config.logging.sinks.file.enable          = true;
  ecal_config.logging.sinks.file.path            = logging_path;
  ecal_config.logging.sinks.file.filter_log_file = log_level_all;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  eCAL::Logging::Log(log_message);

  eCAL::Finalize();

  std::string filepath;
  for (const auto& entry : std::filesystem::directory_iterator(logging_path))
  {
    if (entry.is_regular_file())
    {
      if (entry.path().string().find(unit_name) != std::string::npos)
      {
        filepath = entry.path().string();
      }
    }
  }

  EXPECT_NE(filepath, "");

  std::ifstream logfile(filepath);
  
  std::string line;
  std::getline(logfile, line);
  EXPECT_NE(line, "");
  
  auto find_message = line.find(log_message);
  EXPECT_NE(find_message, std::string::npos);
    
  
  logfile.close();

  // if (!filepath.empty()) std::remove(filepath.c_str());
}

TEST(logging_sinks /*unused*/, udp_logging /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "udp_logging_test";
  const std::string log_message  = "Logging test for udp.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.file.enable        = false;
  ecal_config.logging.sinks.console.enable     = false;
  ecal_config.logging.sinks.udp.enable         = true;
  ecal_config.logging.sinks.udp.filter_log_udp = log_level_all;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);  

  eCAL::Logging::Log(log_level_info, log_message);

  std::this_thread::sleep_for(UDP_WAIT_TIME);
  
  eCAL::Logging::SLogging log;
  eCAL::Logging::GetLogging(log);

  EXPECT_EQ(log.log_messages.size(), 1);
  
  if (log.log_messages.size() > 0)
  {
    EXPECT_EQ(log.log_messages.front().hname, eCAL::Process::GetHostName());
    EXPECT_EQ(log.log_messages.front().pid,   eCAL::Process::GetProcessID());
    EXPECT_EQ(log.log_messages.front().uname, unit_name);
    EXPECT_EQ(log.log_messages.front().level, log_level_info);
    EXPECT_TRUE(log.log_messages.front().content.find(log_message) != std::string::npos);
  }

  eCAL::Finalize();
}

TEST(logging_sinks /*unused*/, console_logging /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "udp_logging_test";
  const std::string log_message  = "Logging test for udp.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.file.enable            = false;
  ecal_config.logging.sinks.udp.enable             = false;
  ecal_config.logging.sinks.console.enable         = true;
  ecal_config.logging.sinks.console.filter_log_con = log_level_all;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  {
    std::stringstream ss;
    CoutRedirect redirect(ss);
    eCAL::Logging::Log(log_message);
    std::string console_output = ss.str();
    EXPECT_TRUE(console_output.find(log_message) != std::string::npos);
  }

  eCAL::Finalize();
}

int getLogging(eCAL::Logging::SLogging& log_)
{
  std::this_thread::sleep_for(UDP_WAIT_TIME);

  return eCAL::Logging::GetLogging(log_);
}

TEST(logging_levels /*unused*/, set_level_test /*unused*/)
{
  const std::string logging_path = "./";
  const std::string unit_name    = "udp_logging_test";
  const std::string log_message  = "Logging test for udp.";
  auto& ecal_config              = eCAL::GetConfiguration();

  ecal_config.logging.sinks.file.enable        = false;
  ecal_config.logging.sinks.console.enable     = false;
  ecal_config.logging.sinks.udp.enable         = true;
  ecal_config.logging.sinks.udp.filter_log_udp = log_level_warning;

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);

  eCAL::Logging::SLogging log;
  
  // should not log
  eCAL::Logging::Log(log_level_info, log_message);
  EXPECT_EQ(getLogging(log), 0);

  // should log
  eCAL::Logging::Log(log_level_warning, log_message);
  EXPECT_EQ(getLogging(log), 1);

  eCAL::Finalize();

  eCAL::Initialize(ecal_config, unit_name.c_str(), eCAL::Init::Logging);
}
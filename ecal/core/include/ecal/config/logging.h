/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

/**
 * @file   ecal_logging_config.h
 * @brief  eCAL configuration for logging
**/

#pragma once

#include <ecal/ecal_log_level.h>

#include <vector>
#include <string>

namespace
{
  // After switchting to c++17, this can be replaced by an inline constexpr
  static const eCAL_Logging_Filter log_filter_default = log_level_info | log_level_warning | log_level_error | log_level_fatal;
}

namespace eCAL
{
  namespace Logging
  {
    namespace Sinks
    {
      struct Sink
      {
        bool enable;                                    //!< Enable sink
        eCAL_Logging_Filter filter_log;                 //!< Log messages logged (all, info, warning, error, fatal, debug1, debug2, debug3, debug4)
      };

      namespace File
      {
        struct Configuration
        {
          std::string path { "" };                      //!< Path to log file (Default: "")
        };
      }

      namespace UDP
      {
        struct ReceiverConfiguration
        {
          bool         enable { false };                //!< Enable UDP receiver (Default: false)
          unsigned int port   { 14001 };                //!< UDP port number (Default: 14001)
        };

        struct ProviderConfiguration
        {
          unsigned int port { 14001 };                  //!< UDP port number (Default: 14001)
        };

        struct Configuration
        {
          ReceiverConfiguration receiver;
          ProviderConfiguration provider;
        };
      }

      struct Configuration
      {
        Sink                console { true,  log_level_error | log_level_fatal}; //!< default: true, log_level_error
        Sink                file    { false, log_level_none };                   //!< default: false, log_level_none
        Sink                udp     { true,  log_filter_default };               //!< default: true, log_filter_default
        
        File::Configuration file_config;
        UDP::Configuration  udp_config;
        
      };
    }
    
    struct Configuration
    {
      Sinks::Configuration sinks;
    };
  }
}
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
 * @file   ecal_log.h
 * @brief  eCAL logging interface
**/

#pragma once

#include <ecal/ecal_os.h>
#include <ecal/ecal_log_level.h>
#include <ecal/types/logging.h>

#include <list>
#include <string>

namespace eCAL
{  
  namespace Logging
  {
    /**
     * @brief Log a message.
     *
     * @param level_  The level.
     * @param msg_    The log message string.
    **/
    ECAL_API void Log(eCAL_Logging_eLogLevel level_, const std::string& msg_);

    /**
     * @brief Get logging as serialized protobuf string.
     *
     * @param [out] log_  String to store the logging information.
     *
     * @return  Logging buffer length or zero if failed.
    **/
    ECAL_API int GetLogging(std::string& log_);

    /**
     * @brief Get logging as struct.
     *
     * @param [out] log_  Target struct to store the logging information.
     *
     * @return Number of log messages.
    **/
    ECAL_API int GetLogging(Logging::SLogging& log_);
  }
}

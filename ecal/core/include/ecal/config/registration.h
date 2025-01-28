/* =========================== LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
 * @file   config/registration.h
 * @brief  eCAL configuration for the registration layer
**/

#pragma once

#include "ecal/os.h"

#include <stdexcept>
#include <string>

namespace eCAL
{
  namespace Registration
  {
    namespace Layer
    {
      namespace SHM
      {
        struct Configuration
        {
          bool        enable     { false };      /*!< Enable shared memory based registration (Default: false) */
          std::string domain     { "ecal_mon" }; //!< Domain name for shared memory based registration (Default: ecal_mon)
          size_t      queue_size { 1024 };       //!< Queue size of registration events (Default: 1024)
        };
      }

      namespace UDP
      {
        struct Configuration
        {
          bool         enable { true };          /*!< Enable UDP based registration (Default: true) */
          unsigned int port   { 14000 };         /*!< UDP multicast port number (Default: 14000) */
        };
      }

      struct Configuration
      {
        SHM::Configuration shm;                  /*!< Shared memory based registration configuration */
        UDP::Configuration udp;                  /*!< UDP based registration configuration */
      };
    }

    struct Configuration
    {
      unsigned int         registration_timeout { 10000U }; //!< Timeout for topic registration in ms (internal) (Default: 10000)
      unsigned int         registration_refresh { 1000U };  //!< Topic registration refresh cylce (has to be smaller then registration timeout!) (Default: 1000)                                   

      bool                 network_enabled      { false };  /*!< true  = all eCAL components communicate over network boundaries
                                                                 false = local host only communication (Default: false) */
      bool                 loopback             { true };   //!< enable to receive udp messages on the same local machine (Default: true)
      std::string          shm_transport_domain { "" };     /*!< Common shm transport domain that enables interprocess mechanisms across
                                                                 (virtual) host borders (e.g, Docker); by default equivalent to local host name (Default: "") */
      Layer::Configuration layer;
    };
  }
}
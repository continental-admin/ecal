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

#ifndef REGISTRATION_PROVIDER_ATTR_H
#define REGISTRATION_PROVIDER_ATTR_H

#include <string>
#include <ecal/types/ecal_custom_data_types.h>

namespace eCAL
{
  namespace Registration
  {
    struct SUDPModeAttr
    {
      std::string group;
      int         ttl;
    };

    struct SUDPAttr
    {
      Types::UDPMode mode;
      int            port;
      int            sendbuffer;
      int            receivebuffer;
      SUDPModeAttr   network;
      SUDPModeAttr   local;
    };

    struct SSHMAttr
    {    
      std::string domain;
      size_t      queue_size;
    };

    struct SAttr
    {
      bool         network_enabled;
      bool         loopback;
      bool         shm_enabled;
      bool         udp_enabled;
      unsigned int refresh;
      std::string  host_group_name;
      int          process_id;

      SUDPAttr     udp;
      SSHMAttr     shm;
    };
  }
}

#endif // REGISTRATION_PROVIDER_ATTR_H
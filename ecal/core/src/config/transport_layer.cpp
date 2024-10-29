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

#include "ecal/config/transport_layer.h"

namespace eCAL
{
  namespace TransportLayer
  {
    namespace UDP
    {
      Configuration& Configuration::operator=(const Configuration& other)
      {
        config_version      = other.config_version;
        join_all_interfaces = other.join_all_interfaces;
        mask                = other.mask;
        mode                = other.mode;
        network             = other.network;
        npcap_enabled       = other.npcap_enabled;
        port                = other.port;
        receive_buffer      = other.receive_buffer;
        send_buffer         = other.send_buffer;

        return *this;
      }
    }

    bool Validate(Configuration& config_)
    {
      bool isValid = true;
      isValid &= config_.udp.send_buffer % 1024 == 0;
      isValid &= config_.udp.receive_buffer % 1024 == 0;
      return isValid;
    }
  }
}
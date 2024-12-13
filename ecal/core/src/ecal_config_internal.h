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

#pragma once

#include <cstddef>
#include <ecal/ecal_log_level.h>
#include <ecal/ecal_os.h>
#include <ecal/ecal_tlayer.h>
#include "ecal/config/configuration.h"

#include <string>

namespace eCAL
{
  ECAL_API TransportLayer::Configuration& GetTransportLayerConfiguration ();
  ECAL_API Registration::Configuration&   GetRegistrationConfiguration   ();
  ECAL_API Monitoring::Configuration&     GetMonitoringConfiguration     ();
  ECAL_API Logging::Configuration&        GetLoggingConfiguration        ();
  ECAL_API Time::Configuration&           GetTimesyncConfiguration       ();
  ECAL_API Application::Configuration&    GetApplicationConfiguration    ();
}
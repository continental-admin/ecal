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
 * @brief  eCAL client gateway class
**/

#pragma once

#include "ecal_def.h"
#include "serialization/ecal_struct_sample_registration.h"
#include "util/ecal_expmap.h"

#include <ecal/ecal_callback.h>

#include <atomic>
#include <map>
#include <set>
#include <shared_mutex>
#include <string>
#include <vector>

namespace eCAL
{
  class CServiceClientImpl;

  class CClientGate
  {
  public:
    CClientGate();
    ~CClientGate();

    void Start();
    void Stop();

    bool Register  (const std::string& service_name_, const std::shared_ptr<CServiceClientImpl>& client_);
    bool Unregister(const std::string& service_name_, const std::shared_ptr<CServiceClientImpl>& client_);

    void ApplyServiceRegistration(const Registration::Sample& ecal_sample_);

    void GetRegistrations(Registration::SampleList& reg_sample_list_);

  protected:
    static std::atomic<bool>    m_created;

    using ServiceNameServiceImplMapT = std::multimap<std::string, std::shared_ptr<CServiceClientImpl>>;
    std::shared_timed_mutex     m_service_client_map_sync;
    ServiceNameServiceImplMapT  m_service_client_map;

    using ConnectedMapT = Util::CExpirationMap<std::string, SServiceAttr>;
    std::shared_timed_mutex     m_service_register_map_sync;
    ConnectedMapT               m_service_register_map;
  };
}

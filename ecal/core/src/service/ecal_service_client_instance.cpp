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
 * @brief  eCAL service client instance implementation
**/

#include <ecal/ecal_client_instance.h>
#include "ecal_service_client_id_impl.h"

namespace eCAL
{
  CServiceClientInstance::CServiceClientInstance(const Registration::SEntityId& entity_id_, std::shared_ptr<eCAL::CServiceClientIDImpl>& service_client_id_impl)
    : m_entity_id(entity_id_), m_service_client_impl(service_client_id_impl)
  {
  }

  std::pair<bool, SServiceResponse> CServiceClientInstance::CallWithResponse(const std::string& method_name, const std::string& request, int timeout)
  {
    return m_service_client_impl->CallWithResponse(m_entity_id, method_name, request, timeout);
  }

  bool CServiceClientInstance::CallWithCallback(const std::string& method_name, const std::string& request, int timeout)
  {
    return m_service_client_impl->CallWithCallback(m_entity_id, method_name, request, timeout);
  }

  bool CServiceClientInstance::IsConnected() const
  {
    return m_service_client_impl->IsConnected(m_entity_id);
  }

  Registration::SEntityId CServiceClientInstance::GetClientID() const
  {
    return m_entity_id;
  }
}

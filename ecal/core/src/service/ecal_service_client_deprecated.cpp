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
 * @brief  eCAL service client interface
**/

#include <ecal/ecal.h>
#include <ecal/ecal_client_deprecated.h>
#include <ecal/ecal_service_info.h>

#include "ecal_service_client_deprecated_impl.h"

namespace eCAL
{
  CServiceClient::CServiceClient()
    : m_service_client_depreceated_impl(nullptr)
  {
  }

  CServiceClient::CServiceClient(const std::string& service_name_)
    : m_service_client_depreceated_impl(nullptr)
  {
    Create(service_name_);
  }

  CServiceClient::CServiceClient(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_)
    : m_service_client_depreceated_impl(nullptr)
  {
    Create(service_name_, method_information_map_);
  }

  CServiceClient::~CServiceClient()
  {
    Destroy();
  }

  bool CServiceClient::Create(const std::string& service_name_)
  {
    return Create(service_name_, ServiceMethodInformationMapT());
  }

  bool CServiceClient::Create(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_)
  {
    if (m_service_client_depreceated_impl != nullptr) return false;
    m_service_client_depreceated_impl = std::make_shared<eCAL::CServiceClientDeprecatedImpl>(service_name_, method_information_map_);
    return m_service_client_depreceated_impl != nullptr;
  }

  bool CServiceClient::Destroy()
  {
    if (m_service_client_depreceated_impl == nullptr) return false;
    m_service_client_depreceated_impl.reset();
    return true;
  }

  bool CServiceClient::SetHostName(const std::string& host_name_)
  {
    if (m_service_client_depreceated_impl == nullptr) return false;
    return m_service_client_depreceated_impl->SetHostName(host_name_);
  }

  bool CServiceClient::Call(const std::string& method_name_, const std::string& request_, int timeout_)
  {
    if (m_service_client_depreceated_impl == nullptr) return false;
    return m_service_client_depreceated_impl->Call(method_name_, request_, timeout_);
  }

  bool CServiceClient::Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_)
  {
    if (m_service_client_depreceated_impl == nullptr) return false;
    return m_service_client_depreceated_impl->Call(method_name_, request_, timeout_, service_response_vec_);
  }

  bool CServiceClient::CallAsync(const std::string& method_name_, const std::string& request_, int /*timeout_*/)
  {
    if (m_service_client_depreceated_impl == nullptr) return false;
    return m_service_client_depreceated_impl->CallAsync(method_name_, request_);
  }

  bool CServiceClient::AddResponseCallback(const ResponseCallbackT& callback_)
  {
    if (m_service_client_depreceated_impl == nullptr) return false;
    return m_service_client_depreceated_impl->AddResponseCallback(callback_);
  }

  bool CServiceClient::RemResponseCallback()
  {
    if (m_service_client_depreceated_impl == nullptr) return false;
    return m_service_client_depreceated_impl->RemResponseCallback();
  }

  bool CServiceClient::AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_)
  {
    if (m_service_client_depreceated_impl == nullptr) return false;
    return m_service_client_depreceated_impl->AddEventCallback(type_, callback_);
  }

  bool CServiceClient::RemEventCallback(eCAL_Client_Event type_)
  {
    if (m_service_client_depreceated_impl == nullptr) return false;
    return m_service_client_depreceated_impl->RemEventCallback(type_);
  }

  std::string CServiceClient::GetServiceName()
  {
    if (m_service_client_depreceated_impl == nullptr) return "";
    return m_service_client_depreceated_impl->GetServiceName();
  }

  bool CServiceClient::IsConnected()
  {
    if (m_service_client_depreceated_impl == nullptr) return false;
    return m_service_client_depreceated_impl->IsConnected();
  }
}

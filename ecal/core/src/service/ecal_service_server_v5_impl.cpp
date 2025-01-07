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
 * @brief  eCAL service server implementation (deprecated eCAL5 version)
**/

#include <ecal/ecal_log.h>

#include "ecal_service_server_v5_impl.h"

namespace eCAL
{
  namespace v5
  {
    CServiceServerImpl::CServiceServerImpl()
      : m_service_server_impl(nullptr)
    {
      Logging::Log(log_level_debug2, "v5::CServiceServerImpl: Initializing default service server implementation.");
    }

    CServiceServerImpl::CServiceServerImpl(const std::string& service_name_)
      : m_service_server_impl(nullptr)
    {
      Logging::Log(log_level_debug2, "v5::CServiceServerImpl: Initializing service server with name: " + service_name_);
      Create(service_name_);
    }

    CServiceServerImpl::~CServiceServerImpl()
    {
      Logging::Log(log_level_debug2, "v5::CServiceServerImpl: Destroying service server implementation.");
      Destroy();
    }

    bool CServiceServerImpl::Create(const std::string& service_name_)
    {
      if (m_service_server_impl)
      {
        Logging::Log(log_level_warning, "v5::CServiceServerImpl: Service server already created: " + service_name_);
        return false;
      }

      // Define the event callback to pass to CServiceClient
      ServerEventIDCallbackT event_callback = [this](const Registration::SServiceMethodId& service_id_, const struct SServerEventCallbackData& data_)
        {
          // Lock the mutex to safely access m_event_callbacks
          std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);

          // Check if there's a callback registered for the event type
          const auto& callback = m_event_callback_map.find(data_.type);
          if (callback != m_event_callback_map.end())
          {
            // Call the user's callback
            callback->second(service_id_.service_name.c_str(), &data_);
          }
        };

      m_service_server_impl = std::make_shared<eCAL::CServiceServer>(service_name_, event_callback);
      Logging::Log(log_level_debug1, "v5::CServiceServerImpl: Service server created with name: " + service_name_);
      return true;
    }

    bool CServiceServerImpl::Destroy()
    {
      if (!m_service_server_impl)
      {
        Logging::Log(log_level_warning, "v5::CServiceServerImpl: Service server not initialized, cannot destroy.");
        return false;
      }

      m_service_server_impl.reset();
      Logging::Log(log_level_debug2, "v5::CServiceServerImpl: Service server destroyed.");
      return true;
    }

    bool CServiceServerImpl::AddDescription(const std::string& method_, const std::string& req_type_, const std::string& req_desc_, const std::string& resp_type_, const std::string& resp_desc_)
    {
      Logging::Log(log_level_debug1, "v5::CServiceServerImpl: Adding description for method: " + method_);

      if (!m_service_server_impl)
      {
        Logging::Log(log_level_error, "v5::CServiceServerImpl: Service server not initialized, cannot add description.");
        return false;
      }

      SServiceMethodInformation method_info;
      method_info.request_type.name        = req_type_;
      method_info.request_type.descriptor  = req_desc_;
      method_info.response_type.name       = resp_type_;
      method_info.response_type.descriptor = resp_desc_;

      return m_service_server_impl->SetMethodCallback(method_, method_info, nullptr);
    }

    bool CServiceServerImpl::AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_)
    {
      Logging::Log(log_level_debug2, "v5::CServiceServerImpl: Adding method callback for method: " + method_);

      if (!m_service_server_impl)
      {
        Logging::Log(log_level_error, "v5::CServiceServerImpl: Service server not initialized, cannot add method callback.");
        return false;
      }

      SServiceMethodInformation method_info;
      method_info.request_type.name  = req_type_;
      method_info.response_type.name = resp_type_;

      MethodInfoCallbackT callback =
        [req_type_, resp_type_, callback_](
          const std::string&          method,
          const SDataTypeInformation& req_type_info,
          const SDataTypeInformation& resp_type_info,
          const std::string&          request,
          std::string&                response) -> int
        {
          return callback_(method, req_type_info.name, resp_type_info.name, request, response);
        };

      return m_service_server_impl->SetMethodCallback(method_, method_info, callback);
    }

    bool CServiceServerImpl::RemMethodCallback(const std::string& method_)
    {
      Logging::Log(log_level_debug2, "v5::CServiceServerImpl: Removing method callback for method: " + method_);

      if (!m_service_server_impl)
      {
        Logging::Log(log_level_error, "v5::CServiceServerImpl: Service server not initialized, cannot remove method callback.");
        return false;
      }

      return m_service_server_impl->RemoveMethodCallback(method_);
    }

    bool CServiceServerImpl::AddEventCallback(eCAL_Server_Event type_, ServerEventCallbackT callback_)
    {
      if (!m_service_server_impl)
      {
        Logging::Log(log_level_error, "v5::CServiceServerImpl: Service server not initialized, cannot add event callback.");
        return false;
      }
      Logging::Log(log_level_debug2, "v5::CServiceServerImpl: Adding event callback for event type: " + std::to_string(type_));

      {
        const std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);
        m_event_callback_map[type_] = callback_;
      }

      return true;
    }

    bool CServiceServerImpl::RemEventCallback(eCAL_Server_Event type_)
    {
      if (!m_service_server_impl)
      {
        Logging::Log(log_level_error, "v5::CServiceServerImpl: Service server not initialized, cannot remove event callback.");
        return false;
      }
      Logging::Log(log_level_debug2, "v5::CServiceServerImpl: Removing event callback for event type: " + std::to_string(type_));

      {
        const std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);
        m_event_callback_map.erase(type_);
      }

      return false;
    }

    std::string CServiceServerImpl::GetServiceName()
    {
      if (!m_service_server_impl)
      {
        Logging::Log(log_level_error, "v5::CServiceServerImpl: Service server not initialized, cannot get service name.");
        return "";
      }

      return m_service_server_impl->GetServiceName();
    }

    bool CServiceServerImpl::IsConnected()
    {
      if (!m_service_server_impl)
      {
        Logging::Log(log_level_error, "v5::CServiceServerImpl: Service server not initialized, cannot check connection status.");
        return false;
      }

      return m_service_server_impl->IsConnected();
    }
  }
}

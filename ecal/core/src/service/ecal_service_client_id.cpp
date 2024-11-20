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
#include <string>

#include "ecal_clientgate.h"
#include "ecal_global_accessors.h"
#include "ecal_service_client_id_impl.h"

namespace eCAL
{
  /**
   * @brief Constructor.
  **/
  CServiceClientID::CServiceClientID() :
                    m_service_client_impl(nullptr)
  {
  }

  /**
   * @brief Constructor. 
   *
   * @param service_name_  Service name. 
  **/
  CServiceClientID::CServiceClientID(const std::string& service_name_) :
                    m_service_client_impl(nullptr)
  {
    Create(service_name_);
  }

  /**
   * @brief Constructor.
   *
   * @param service_name_  Service name.
   * @param method_information_map_  Map of method names and corresponding datatype information.
  **/
  CServiceClientID::CServiceClientID(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_) :
    m_service_client_impl(nullptr)
  {
    Create(service_name_, method_information_map_);
  }

  /**
   * @brief Destructor. 
  **/
  CServiceClientID::~CServiceClientID()
  {
    Destroy();
  }

  /**
   * @brief Creates this object. 
   *
   * @param service_name_  Service name. 
   *
   * @return  True if successful. 
  **/
  bool CServiceClientID::Create(const std::string& service_name_)
  {
    return Create(service_name_, ServiceMethodInformationMapT());
  }

  /**
   * @brief Creates this object.
   *
   * @param service_name_  Service name.
   * @param method_information_map_  Map of method names and corresponding datatype information.
   *
   * @return  True if successful.
  **/
  bool CServiceClientID::Create(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_)
  {
    if (m_service_client_impl) return(false);

    // create client
    m_service_name = service_name_;
    m_service_client_impl = CServiceClientIDImpl::CreateInstance(service_name_, method_information_map_);

    // register client
    if (g_clientgate() != nullptr) g_clientgate()->Register(m_service_name, m_service_client_impl);

    // we made it :-)
    return true;
  }

  /**
   * @brief Destroys this object. 
   *
   * @return  True if successful. 
  **/
  bool CServiceClientID::Destroy()
  {
    if(!m_service_client_impl) return(false);

    // unregister client
    if (g_clientgate() != nullptr) g_clientgate()->Unregister(m_service_name, m_service_client_impl);

    // stop & destroy client
    m_service_client_impl.reset();
    m_service_name = "";

    return true;
  }

  /**
   * @brief Add client event callback function.
   *
   * @param type_      The event type to react on.
   * @param callback_  The callback function to add.
   *
   * @return  True if succeeded, false if not.
  **/
  bool CServiceClientID::AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_)
  {
    if (!m_service_client_impl) return false;
    return m_service_client_impl->AddEventCallback(type_, callback_);
  }

  /**
   * @brief Remove client event callback function.
   *
   * @param type_  The event type to remove.
   *
   * @return  True if succeeded, false if not.
  **/
  bool CServiceClientID::RemEventCallback(eCAL_Client_Event type_)
  {
    if (!m_service_client_impl) return false;
    return m_service_client_impl->RemoveEventCallback(type_);
  }

  /**
   * @brief Get the client instances for all matching services
   *
   * @return  Vector of client instances
  **/
  std::vector<CServiceClientInstance> CServiceClientID::GetServiceClientInstances()
  {
    std::vector<CServiceClientInstance> instances;
    auto entity_ids = m_service_client_impl->GetServiceIDs();
    for (const auto& entity_id : entity_ids)
    {
      instances.emplace_back(entity_id, m_service_client_impl);
    }
    return instances;
  }

  /**
   * @brief Blocking call of a service method for all existing service instances, using callback
   *
   * @param method_name_        Method name.
   * @param request_            Request string.
   * @param timeout_            Maximum time before operation returns (in milliseconds, -1 means infinite).
   * @param response_callback_  Callback function for the service method response.
   *
   * @return  True if successful.
  **/
  void CServiceClientID::CallWithCallback(const std::string& method_name_, const std::string& request_, int timeout_, const ResponseIDCallbackT& repsonse_callback_)
  {
    auto instances = GetServiceClientInstances();
    for (auto& instance : instances)
    {
      instance.CallWithCallback(method_name_, request_, timeout_, repsonse_callback_);
    }
  }

  /**
   * @brief Retrieve service name.
   *
   * @return  The service name.
  **/
  std::string CServiceClientID::GetServiceName() const
  {
    return m_service_name;
  }

  /**
   * @brief Check connection state.
   *
   * @return  True if connected, false if not.
  **/
  bool CServiceClientID::IsConnected() const
  {
    if (!m_service_client_impl) return false;
    return m_service_client_impl->IsConnected();
  }
}

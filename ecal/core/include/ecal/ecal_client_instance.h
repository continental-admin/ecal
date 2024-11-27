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
 * @file   ecal_client_instance.h
 * @brief  eCAL client instance interface
**/

#pragma once

#include <ecal/ecal_callback.h>
#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_os.h>
#include <ecal/ecal_types.h>
#include <ecal/ecal_service_info.h>

#include <memory>
#include <string>

#pragma once

namespace eCAL
{
  class CServiceClientIDImpl;

  class CServiceClientInstance final
  {
  public:
    // Constructor
    ECAL_API CServiceClientInstance(const Registration::SEntityId& entity_id_, const std::shared_ptr<CServiceClientIDImpl>& service_client_id_impl_);

    // Defaulted destructor
    ~CServiceClientInstance() = default;

    // Deleted copy constructor and copy assignment operator
    CServiceClientInstance(const CServiceClientInstance&) = delete;
    CServiceClientInstance& operator=(const CServiceClientInstance&) = delete;

    // Defaulted move constructor and move assignment operator
    ECAL_API CServiceClientInstance(CServiceClientInstance&& rhs) noexcept = default;
    ECAL_API CServiceClientInstance& operator=(CServiceClientInstance&& rhs) noexcept = default;

    /**
     * @brief Blocking call of a service method, response will be returned as pair<bool, SServiceReponse>
     *
     * @param       method_name_  Method name.
     * @param       request_      Request string.
     * @param       timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
     *
     * @return  success state and service response
    **/
    ECAL_API std::pair<bool, SServiceResponse> CallWithResponse(const std::string& method_name_, const std::string& request_, int timeout_ = -1);

    /**
     * @brief Blocking call of a service method, using callback
     *
     * @param method_name_        Method name.
     * @param request_            Request string.
     * @param timeout_            Maximum time before operation returns (in milliseconds, -1 means infinite).
     * @param response_callback_  Callback function for the service method response.
     *
     * @return  True if successful.
    **/
    ECAL_API bool CallWithCallback(const std::string& method_name_, const std::string& request_, int timeout_, const ResponseIDCallbackT& response_callback_);

    /**
     * @brief Asynchronous call of a service method, using callback
     *
     * @param method_name_        Method name.
     * @param request_            Request string.
     * @param response_callback_  Callback function for the service method response.
     *
     * @return  True if successful.
    **/
    ECAL_API bool CallWithCallbackAsync(const std::string& method_name_, const std::string& request_, const ResponseIDCallbackT& response_callback_);

    /**
     * @brief Check connection state.
     *
     * @return  True if connected, false if not.
    **/
    ECAL_API bool IsConnected() const;

    /**
     * @brief Get unique client ide.
     *
     * @return  The client id.
    **/
    ECAL_API Registration::SEntityId GetClientID() const;

  private:
    Registration::SEntityId                           m_entity_id;
    const std::shared_ptr<eCAL::CServiceClientIDImpl> m_service_client_impl;
  };
}

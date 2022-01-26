/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
 * @file   ecal_client.h
 * @brief  eCAL client interface
**/

#pragma once

#include <ecal/ecal_os.h>
#include <ecal/ecal_callback.h>
#include <ecal/ecal_service_info.h>

#include <iostream>
#include <string>
#include <vector>

#ifndef ECAL_C_DLL

namespace eCAL
{
  class CServiceClientImpl;

  /**
   * @brief Service client wrapper class.
  **/
  class ECAL_API CServiceClient
  {
  public:
    /**
     * @brief Constructor. 
    **/
    CServiceClient();

    /**
     * @brief Constructor. 
     *
     * @param service_name_  Unique service name.
    **/
    CServiceClient(const std::string& service_name_);

    /**
     * @brief Destructor. 
    **/
    virtual ~CServiceClient();

    /**
     * @brief CServiceClients are non-copyable
    **/
    CServiceClient(const CServiceClient&) = delete;

    /**
     * @brief CServiceClients are non-copyable
    **/
    CServiceClient& operator=(const CServiceClient&) = delete;

    /**
     * @brief Creates this object. 
     *
     * @param service_name_  Unique service name.
     *
     * @return  True if successful. 
    **/
    bool Create(const std::string& service_name_);

    /**
     * @brief Destroys this object. 
     *
     * @return  True if successful. 
    **/
    bool Destroy();

    /**
     * @brief Change the host name filter for that client instance
     *
     * @param host_name_  Host name filter (empty == all hosts)
     *
     * @return  True if successful.
    **/
    bool SetHostName(const std::string& host_name_);

    /**
     * @brief Call a method of this service, responses will be returned by callback. 
     *
     * @param method_name_  Method name.
     * @param request_      Request string. 
     * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
     *
     * @return  True if successful. 
    **/
    bool Call(const std::string& method_name_, const std::string& request_, int timeout_ = -1);

    /**
     * @brief Call a method of this service, all responses will be returned in service_response_vec_. 
     *
     * @param       method_name_           Method name.
     * @param       request_               Request string.
     * @param       timeout_               Maximum time before operation returns (in milliseconds, -1 means infinite).
     * @param [out] service_response_vec_  Response vector containing service responses from every called service (null pointer == no response).
     *
     * @return  True if successful.
    **/
    bool Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_);

    /**
     * @brief Call method of this service, for specific host (deprecated). 
     *
     * @param       host_name_         Host name.
     * @param       method_name_       Method name.
     * @param       request_           Request string.
     * @param [out] service_info_      Service response struct for detailed informations.
     * @param [out] response_          Response string.
     *
     * @return  True if successful.
    **/
    [[deprecated]]
    bool Call(const std::string& host_name_, const std::string& method_name_, const std::string& request_, struct SServiceResponse& service_info_, std::string& response_);

    /**
     * @brief Call a method of this service asynchronously, responses will be returned by callback. 
     *
     * @param method_name_  Method name.
     * @param request_      Request string. 
     * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite) - NOT SUPPORTED YET.
     *
     * @return  True if successful.
    **/
    bool CallAsync(const std::string& method_name_, const std::string& request_, int timeout_ = -1);

    /**
     * @brief Add server response callback. 
     *
     * @param callback_  Callback function for server response.  
     *
     * @return  True if successful.
    **/
    bool AddResponseCallback(const ResponseCallbackT& callback_);

    /**
     * @brief Remove server response callback. 
     *
     * @return  True if successful.
    **/
    bool RemResponseCallback();

    /**
     * @brief Add client event callback function.
     *
     * @param type_      The event type to react on.
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not.
    **/
    bool AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_);

    /**
     * @brief Remove client event callback function.
     *
     * @param type_  The event type to remove.
     *
     * @return  True if succeeded, false if not.
    **/
    bool RemEventCallback(eCAL_Client_Event type_);

    /**
     * @brief Retrieve service name.
     *
     * @return  The service name.
    **/
    std::string GetServiceName();

    /**
     * @brief Check connection state.
     *
     * @return  True if connected, false if not.
    **/
    bool IsConnected();

  protected:
    CServiceClientImpl* m_service_client_impl;
    bool                m_created;
  };
}

#else /* ! ECAL_C_DLL */

#include <ecal/ecalc.h>

namespace eCAL
{
  class CServiceClient
  {
  public:
    CServiceClient() :
      m_service(nullptr),
      m_created(false)
    {
    }

    CServiceClient(const std::string& service_name_) :
      m_service(nullptr),
      m_created(false)
    {
      Create(service_name_);
    }

    virtual ~CServiceClient()
    {
      Destroy();
    }

    // this object must not be copied.
    CServiceClient(const CServiceClient&) = delete;
    CServiceClient& operator=(const CServiceClient&) = delete;

    bool Create(const std::string& service_name_)
    {
      if (m_created) return(false);
      m_service = eCAL_Client_Create(service_name_.c_str());
      if (!m_service) return(false);

      m_service_name = service_name_;
      m_created = true;
      return(true);
    }

    bool Destroy()
    {
      if (!m_created) return(false);
      if (m_service) eCAL_Client_Destroy(m_service);

      m_service = nullptr;
      m_service_name.clear();
      m_created = false;
      return(true);
    }

    bool SetHostName(const std::string& host_name_)
    {
      if (!m_service) return(false);
      return(eCAL_Client_SetHostName(m_service, host_name_.c_str()) != 0);
    }

    bool Call(const std::string& method_name_, const std::string& request_, int timeout_ = -1)
    {
      if (!m_service) return(false);
      return(eCAL_Client_Call(m_service, method_name_.c_str(), request_.c_str(), static_cast<int>(request_.size()), timeout_) != 0);
    }

    // The C API variant is not able to return all service responses but only the first one !
    bool Call(const std::string& method_name_, const std::string& request_, int timeout_, SServiceResponse* service_response_)
    {
      if (!m_service) return(false);
      void* response = NULL;
      struct SServiceResponseC service_info;
      int response_len = eCAL_Client_Call_Wait(m_service, method_name_.c_str(), request_.c_str(), static_cast<int>(request_.size()), timeout_, &service_info, &response, ECAL_ALLOCATE_4ME);
      if (response_len > 0)
      {
        if (service_response_)
        {
          service_response_->host_name = "";                         //  host_name not yet available in C API
          service_response_->service_name = m_service_name;
          service_response_->method_name  = method_name_;
          service_response_->error_msg    = "";                      // error_msg not yet available in C API
          service_response_->ret_state    = service_info.ret_state;
          service_response_->call_state   = service_info.call_state;
          service_response_->response     = std::string(static_cast<const char*>(response), response_len);
        }
        eCAL_FreeMem(response);
        return(true);
      }
      return(false);
    }

    bool CallAsync(const std::string& method_name_, const std::string& request_, int timeout_ = -1)
    {
      if (!m_service) return(false);
      return(eCAL_Client_Call_Async(m_service, method_name_.c_str(), request_.c_str(), static_cast<int>(request_.size()), timeout_) != 0);
    }
      
    static void ResponseCallback(const struct SServiceResponseC* service_response_, void* par_)
    {
      if(par_ == nullptr) return;
      CServiceClient* client = static_cast<CServiceClient*>(par_);
      if (client->m_response_callback == nullptr) return;

      SServiceResponse service_response;
      service_response.host_name    = service_response_->host_name;
      service_response.service_name = service_response_->service_name;
      service_response.method_name  = service_response_->method_name;
      service_response.error_msg    = service_response_->error_msg;
      service_response.ret_state    = service_response_->ret_state;
      service_response.call_state   = service_response_->call_state;
      service_response.response     = std::string(static_cast<const char*>(service_response_->response), service_response_->response_len);
      client->m_response_callback(service_response);
    }
    bool AddResponseCallback(ResponseCallbackT callback_)
    {
      if(!m_service) return false;
      m_response_callback = callback_;
      return(eCAL_Client_AddResponseCallback(m_service, &CServiceClient::ResponseCallback, this) != 0);
    }

    bool RemResponseCallback()
    {
      if(!m_service) return false;
      return(eCAL_Client_RemResponseCallback(m_service) != 0);
    }

    static void EventCallback(const char* name_, const struct SClientEventCallbackDataC* data_, void* par_)
    {
      if (par_ == nullptr) return;
      CServiceClient* client = static_cast<CServiceClient*>(par_);
      if (client->m_event_callback == nullptr) return;

      SClientEventCallbackData data;
      data.type = data_->type;
      data.time = data_->time;
      client->m_event_callback(name_, &data);
    }
    bool AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_)
    {
      if (!m_service) return false;
      m_event_callback = callback_;
      return(eCAL_Client_AddEventCallback(m_service, type_ , &CServiceClient::EventCallback, this) != 0);
    }

    bool RemEventCallback(eCAL_Client_Event type_)
    {
      if (!m_service) return false;
      return(eCAL_Client_RemEventCallback(m_service, type_) != 0);
    }

    std::string GetServiceName()
    {
      if (!m_created) return "";
      return(m_service_name);
    }

  protected:
    ECAL_HANDLE           m_service;
    ResponseCallbackT     m_response_callback;
    ClientEventCallbackT  m_event_callback;

    std::string           m_service_name;
    bool                  m_created;
  };
} 

#endif /* ! ECAL_C_DLL */

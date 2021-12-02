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
 * @brief  eCAL service client implementation
**/

#include <ecal/ecal.h>
#include <ecal/ecal_callback.h>

#include "service/ecal_tcpclient.h"

#include <map>
#include <mutex>

namespace eCAL
{
  /**
   * @brief Service client implementation class. 
  **/
  class CServiceClientImpl
  {
  public:
    CServiceClientImpl();
    CServiceClientImpl(const std::string& service_name_);

    ~CServiceClientImpl();

    bool Create(const std::string& service_name_);

    bool Destroy();

    bool SetHostName(const std::string& host_name_);

    bool AddResponseCallback(const ResponseCallbackT& callback_);
    bool RemResponseCallback();

    bool AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_);
    bool RemEventCallback(eCAL_Client_Event type_);
      
    // call service on a specific host, no broadcast, first will react service only
    [[deprecated]]
    bool Call(const std::string& host_name_, const std::string& method_name_, const std::string& request_, struct SServiceResponse& service_response_, std::string& response_);
    
    // call all services on a specific host
    bool Call(const std::string& host_name_, const std::string& method_name_, const std::string& request_, ServiceResponseVecT* service_response_vec_);

    // callback service using callback, broadcast possible
    bool Call(const std::string& method_name_, const std::string& request_);

    void CallAsync(const std::string& method_name_, const std::string& request_);
    
    void CallAsync(const std::string& host_name_, const std::string& method_name_, const std::string& request_);

    void RegisterService(const std::string& key_, const SServiceAttr& service_);
    void RefreshRegistration();

    bool IsConnected();

    // this object must not be copied.
    CServiceClientImpl(const CServiceClientImpl&) = delete;
    CServiceClientImpl& operator=(const CServiceClientImpl&) = delete;

  protected:
    void CheckForNewServices();
    bool SendRequests(const std::string& method_name_, const std::string& request_);
    bool SendRequest(std::shared_ptr<CTcpClient> client_, const std::string& method_name_, const std::string& request_, struct SServiceResponse& service_response_);
    void SendRequestsAsync(const std::string& method_name_, const std::string& request_);
    void SendRequestAsync(std::shared_ptr<CTcpClient> client_, const std::string& method_name_, const std::string& request_);

    void ErrorCallback(const std::string &method_name_, const std::string &error_message_);

    typedef std::map<std::string, std::shared_ptr<CTcpClient>> ClientMapT;
    std::mutex         m_client_map_sync;
    ClientMapT         m_client_map;

    enum { max_length = 64 * 1024 };
    char m_reply[max_length];

    std::mutex         m_response_callback_sync;
    ResponseCallbackT  m_response_callback;

    std::mutex         m_event_callback_map_sync;
    typedef std::map<eCAL_Client_Event, ClientEventCallbackT> EventCallbackMapT;
    EventCallbackMapT  m_event_callback_map;

    std::mutex         m_connected_services_map_sync;
    typedef std::map<std::string, SServiceAttr> ServiceAttrMapT;
    ServiceAttrMapT    m_connected_services_map;


    std::string        m_service_hname;
    std::string        m_service_name;
    bool               m_created;
  };
}

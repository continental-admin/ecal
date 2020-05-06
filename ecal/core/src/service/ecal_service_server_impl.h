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
 * @brief  eCAL service server interface
**/

#pragma once

#include <ecal/ecal.h>
#include <ecal/ecal_service_info.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4800) // disable proto warnings
#endif
#include "ecal/pb/service.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecal_tcpserver.h"

#include <map>

namespace eCAL
{
  /**
   * @brief Service server implementation class.
  **/
  class CServiceServerImpl
  {
  public:
    CServiceServerImpl();
    CServiceServerImpl(const std::string& service_name_);

    ~CServiceServerImpl();

    bool Create(const std::string& service_name_);

    bool Destroy();

    bool AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_);
    bool RemMethodCallback(const std::string& method_);

    void RefreshRegistration();

    std::string GetServiceName() { return m_service_name; };

    // this object must not be copied.
    CServiceServerImpl(const CServiceServerImpl&) = delete;
    CServiceServerImpl& operator=(const CServiceServerImpl&) = delete;

  protected:
    int RequestCallback(const std::string& request_, std::string& response_);

    CTcpServer          m_tcp_server;

    std::string         m_service_name;
    struct SMethodCallback
    {
      eCAL::pb::Method method;
      MethodCallbackT  callback;
    };
    typedef std::map<std::string, SMethodCallback> MethodCallbackMapT;
    MethodCallbackMapT  m_callback_map;
    bool                m_created;
  };
}

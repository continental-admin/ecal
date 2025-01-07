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
 * @brief  eCAL service server implementation
**/

#include <ecal/ecal_config.h>
#include <ecal/ecal_log.h>
#include <ecal/ecal_process.h>

#include "ecal_global_accessors.h"
#include "ecal_service_server_impl.h"
#include "ecal_service_singleton_manager.h"
#include "registration/ecal_registration_provider.h"
#include "serialization/ecal_serialize_service.h"

namespace eCAL
{
  // Factory method to create a new instance of CServiceServerImpl
  std::shared_ptr<CServiceServerImpl> CServiceServerImpl::CreateInstance(
    const std::string& service_name_, const ServerEventIDCallbackT& event_callback_)
  {
#ifndef NDEBUG
    Logging::Log(log_level_debug2, "CServiceServerImpl::CreateInstance: Creating instance of CServiceServerImpl for service: " + service_name_);
#endif
    auto instance = std::shared_ptr<CServiceServerImpl>(new CServiceServerImpl(service_name_, event_callback_));
    if (instance != nullptr)
    {
      instance->Start();
    }
    return instance;
  }

  // Constructor
  CServiceServerImpl::CServiceServerImpl(const std::string& service_name_, const ServerEventIDCallbackT& event_callback_)
    : m_created(false), m_service_name(service_name_), m_event_callback(event_callback_)
  {
#ifndef NDEBUG
    Logging::Log(log_level_debug2, "CServiceServerImpl::CServiceServerImpl: Initializing service server for: " + m_service_name);
#endif
  }

  // Destructor
  CServiceServerImpl::~CServiceServerImpl()
  {
#ifndef NDEBUG
    Logging::Log(log_level_debug1, "CServiceServerImpl::~CServiceServerImpl: Destroying service server for: " + m_service_name);
#endif
    Stop();
  }

  bool CServiceServerImpl::AddMethodCallback(const std::string& method_, const SServiceMethodInformation& method_info_, const MethodCallbackT& callback_)
  {
#ifndef NDEBUG
    Logging::Log(log_level_debug1, "CServiceServerImpl::AddMethodCallback: Adding method callback for method: " + method_);
#endif
    const std::lock_guard<std::mutex> lock(m_method_map_mutex);

    auto iter = m_method_map.find(method_);
    if (iter != m_method_map.end())
    {
      Logging::Log(log_level_warning, "CServiceServerImpl::AddMethodCallback: Method already exists, updating callback: " + method_);
      iter->second.method.req_type  = method_info_.request_type.name;
      iter->second.method.req_desc  = method_info_.request_type.descriptor;
      iter->second.method.resp_type = method_info_.response_type.name;
      iter->second.method.resp_desc = method_info_.response_type.descriptor;
      iter->second.callback = callback_;
    }
    else
    {
#ifndef NDEBUG
      Logging::Log(log_level_debug1, "CServiceServerImpl::AddMethodCallback: Registering new method: " + method_);
#endif
      SMethod method;
      method.method.mname     = method_;
      method.method.req_type  = method_info_.request_type.name;
      method.method.req_desc  = method_info_.request_type.descriptor;
      method.method.resp_type = method_info_.response_type.name;
      method.method.resp_desc = method_info_.response_type.descriptor;
      method.callback         = callback_;
      m_method_map[method_] = method;
    }

    return true;
  }

  bool CServiceServerImpl::RemoveMethodCallback(const std::string& method_)
  {
#ifndef NDEBUG
    Logging::Log(log_level_debug1, "CServiceServerImpl::RemoveMethodCallback: Removing method callback for method: " + method_);
#endif
    const std::lock_guard<std::mutex> lock(m_method_map_mutex);

    auto iter = m_method_map.find(method_);
    if (iter != m_method_map.end())
    {
      m_method_map.erase(iter);
#ifndef NDEBUG
      Logging::Log(log_level_debug1, "CServiceServerImpl::RemoveMethodCallback: Successfully removed method callback: " + method_);
#endif
      return true;
    }

    Logging::Log(log_level_warning, "CServiceServerImpl::RemoveMethodCallback: Attempt to remove non-existent method callback: " + method_);
    return false;
  }

  bool CServiceServerImpl::IsConnected() const
  {
    if (!m_created)
    {
#ifndef NDEBUG
      Logging::Log(log_level_debug2, "CServiceServerImpl: Service is not created; cannot check connection state for: " + m_service_name);
#endif
      return false;
    }

    bool connected = m_tcp_server && m_tcp_server->is_connected();
#ifndef NDEBUG
    Logging::Log(log_level_debug2, "CServiceServerImpl: Connection state for service " + m_service_name + ": " + (connected ? "connected" : "disconnected"));
#endif
    return connected;
  }

  void CServiceServerImpl::RegisterClient(const std::string& /*key_*/, const SClientAttr& /*client_*/)
  {
    // client registration logic is not implemented, as it is not required for service servers
  }

  Registration::Sample CServiceServerImpl::GetRegistration()
  {
#ifndef NDEBUG
    Logging::Log(log_level_debug2, "CServiceServerImpl:::GetRegistration: Generating registration sample for: " + m_service_name);
#endif
    return GetRegistrationSample();
  }

  Registration::SServiceMethodId CServiceServerImpl::GetServiceId() const
  {
    Registration::SServiceMethodId service_id;

    service_id.service_id.entity_id = m_service_id;
    service_id.service_id.entity_id = Process::GetProcessID();
    service_id.service_id.host_name = Process::GetHostName();

    service_id.service_name = m_service_name;
    service_id.method_name  = "";

    return service_id;
  }

  std::string CServiceServerImpl::GetServiceName() const
  {
    return m_service_name;
  }

  void CServiceServerImpl::Start()
  {
    if (m_created)
    {
      Logging::Log(log_level_warning, "CServiceServerImpl: Service is already started: " + m_service_name);
      return;
    }

#ifndef NDEBUG
    Logging::Log(log_level_debug1, "CServiceServerImpl: Starting service server for: " + m_service_name);
#endif

    // Create service ID
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_service_id = counter.str();

    // Get global server manager
    auto server_manager = eCAL::service::ServiceManager::instance()->get_server_manager();
    if (!server_manager || server_manager->is_stopped())
    {
      Logging::Log(log_level_error, "CServiceServerImpl: Failed to start service: Global server manager is unavailable or stopped for: " + m_service_name);
      return;
    }

    // Create callback functions
    const eCAL::service::Server::EventCallbackT event_callback =
      [weak_me = std::weak_ptr<CServiceServerImpl>(shared_from_this())](eCAL::service::ServerEventType event, const std::string& message)
      {
        if (auto me = weak_me.lock())
        {
          Registration::SServiceMethodId service_id;
          service_id.service_name         = me->m_service_name;
          service_id.service_id.entity_id = me->m_service_id;
          me->NotifyEventCallback(service_id, event == eCAL::service::ServerEventType::Connected
            ? eCAL_Server_Event::server_event_connected
            : eCAL_Server_Event::server_event_disconnected, message);
        }
      };

    const eCAL::service::Server::ServiceCallbackT service_callback =
      [weak_me = std::weak_ptr<CServiceServerImpl>(shared_from_this())](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> int
      {
        if (auto me = weak_me.lock())
          return me->RequestCallback(*request, *response);
        return -1;
      };

    // Start service
    m_tcp_server = server_manager->create_server(1, 0, service_callback, true, event_callback);

    if (!m_tcp_server)
    {
      Logging::Log(log_level_error, "CServiceServerImpl: Failed to create TCP server for service: " + m_service_name);
      return;
    }

    // Send registration sample
    if (g_registration_provider())
      g_registration_provider()->RegisterSample(GetRegistrationSample());

    m_created = true;
#ifndef NDEBUG
    Logging::Log(log_level_debug1, "CServiceServerImpl: Service started successfully: " + m_service_name);
#endif
  }

  void CServiceServerImpl::Stop()
  {
    if (!m_created)
    {
      Logging::Log(log_level_warning, "CServiceServerImpl::Stop: Service is not running; cannot stop: " + m_service_name);
      return;
    }

#ifndef NDEBUG
    Logging::Log(log_level_debug1, "CServiceServerImpl::Stop: Stopping service server for: " + m_service_name);
#endif

    // Stop TCP server
    if (m_tcp_server)
    {
      m_tcp_server->stop();
#ifndef NDEBUG
      Logging::Log(log_level_debug1, "CServiceServerImpl::Stop: TCP server stopped for: " + m_service_name);
#endif
    }
    m_tcp_server.reset();

    // Reset method callbacks
    {
      const std::lock_guard<std::mutex> lock(m_method_map_mutex);
      m_method_map.clear();
    }

    // Reset event callback
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_mutex);
      m_event_callback = nullptr;
    }

    // Send unregistration sample
    if (g_registration_provider())
      g_registration_provider()->UnregisterSample(GetUnregistrationSample());

    m_created = false;
#ifndef NDEBUG
    Logging::Log(log_level_debug1, "CServiceServerImpl::Stop: Service stopped successfully: " + m_service_name);
#endif
  }

  Registration::Sample CServiceServerImpl::GetRegistrationSample()
  {
    Registration::Sample ecal_reg_sample;
    ecal_reg_sample.cmd_type = bct_reg_service;

    // might be zero in contruction phase
    const unsigned short server_tcp_port(m_tcp_server ? m_tcp_server->get_port() : 0);
    if (server_tcp_port == 0) return ecal_reg_sample;

    auto& identifier = ecal_reg_sample.identifier;
    identifier.entity_id  = m_service_id;
    identifier.process_id = Process::GetProcessID();
    identifier.host_name  = Process::GetHostName();

    auto& service = ecal_reg_sample.service;
    service.version     = m_server_version;
    service.pname       = Process::GetProcessName();
    service.uname       = Process::GetUnitName();
    service.sname       = m_service_name;
    service.tcp_port_v0 = 0;
    service.tcp_port_v1 = server_tcp_port;

    {
      const std::lock_guard<std::mutex> lock(m_method_map_mutex);
      for (const auto& iter : m_method_map)
      {
        Service::Method method;
        method.mname      = iter.first;
        method.req_type   = iter.second.method.req_type;
        method.req_desc   = iter.second.method.req_desc;
        method.resp_type  = iter.second.method.resp_type;
        method.resp_desc  = iter.second.method.resp_desc;
        method.call_count = iter.second.method.call_count;
        service.methods.push_back(method);
      }
    }

    return ecal_reg_sample;
  }

  Registration::Sample CServiceServerImpl::GetUnregistrationSample()
  {
    Registration::Sample ecal_reg_sample;
    ecal_reg_sample.cmd_type = bct_unreg_service;

    auto& identifier = ecal_reg_sample.identifier;
    identifier.entity_id  = m_service_id;
    identifier.process_id = Process::GetProcessID();
    identifier.host_name  = Process::GetHostName();

    auto& service = ecal_reg_sample.service;
    service.version = m_server_version;
    service.pname   = Process::GetProcessName();
    service.uname   = Process::GetUnitName();
    service.sname   = m_service_name;

    return ecal_reg_sample;
  }

  int CServiceServerImpl::RequestCallback(const std::string& request_pb_, std::string& response_pb_)
  {
#ifndef NDEBUG
    Logging::Log(log_level_debug2, "CServiceServerImpl::RequestCallback: Processing request callback for: " + m_service_name);
#endif

    // prepare response
    Service::Response response;
    auto& response_header = response.header;
    response_header.hname = Process::GetHostName();
    response_header.sname = m_service_name;
    response_header.sid   = m_service_id;

    // try to parse request
    Service::Request request;
    if (!DeserializeFromBuffer(request_pb_.c_str(), request_pb_.size(), request))
    {
      Logging::Log(log_level_error, m_service_name + "::CServiceServerImpl::RequestCallback: Failed to parse request message");

      response_header.state = Service::eMethodCallState::failed;
      const std::string emsg = "Service '" + m_service_name + "' request message could not be parsed.";
      response_header.error = emsg;

      // TODO: The next version of the service protocol should omit the double-serialization (i.e. copying the binary data in a protocol buffer and then serializing that again)
      // serialize response and return "request message could not be parsed"
      SerializeToBuffer(response, response_pb_);

      // Return Failed (error_code = -1), as parsing the request failed. The
      // return value is not propagated to the remote caller.
      return -1;
    }

    // get method
    SMethod method;
    const auto& request_header = request.header;
    response_header.mname = request_header.mname;
    {
      const std::lock_guard<std::mutex> lock(m_method_map_mutex);
      auto requested_method_iterator = m_method_map.find(request_header.mname);
      if (requested_method_iterator == m_method_map.end())
      {
        // set method call state 'failed'
        response_header.state = Service::eMethodCallState::failed;
        // set error message
        const std::string emsg = "CServiceServerImpl: Service '" + m_service_name + "' has no method named '" + request_header.mname + "'";
        response_header.error = emsg;

        // TODO: The next version of the service protocol should omit the double-serialization (i.e. copying the binary data in a protocol buffer and then serializing that again)
        // serialize response and return "method not found"
        SerializeToBuffer(response, response_pb_);

        // Return Success (error_code = 0), as parsing the request worked. The
        // return value is not propagated to the remote caller.
        return 0;
      }
      else
      {
        requested_method_iterator->second.method.call_count++;
        method = requested_method_iterator->second;
      }
    }

    // execute method (outside lock guard)
    const std::string& request_s = request.request;
    std::string response_s;
    const int service_return_state = method.callback(method.method.mname, method.method.req_type, method.method.resp_type, request_s, response_s);

    // set method call state 'executed'
    response_header.state = Service::eMethodCallState::executed;
    // set method response and return state
    response.response = response_s;
    response.ret_state = service_return_state;

    // TODO: The next version of the service protocol should omit the double-serialization (i.e. copying the binary data in a protocol buffer and then serializing that again)
    // serialize response and return "method not found"
    SerializeToBuffer(response, response_pb_);

    // return success (error code 0)
    return 0;
  }

  void CServiceServerImpl::NotifyEventCallback(const Registration::SServiceMethodId& service_id_, eCAL_Server_Event event_type_, const std::string& /*message_*/)
  {
#ifndef NDEBUG
    Logging::Log(log_level_debug1, "CServiceServerImpl::NotifyEventCallback: Notifying event callback for: " + m_service_name + " Event Type: " + std::to_string(event_type_));
#endif

    const std::lock_guard<std::mutex> lock_cb(m_event_callback_mutex);
    if (m_event_callback)
    {
      SServerEventCallbackData callback_data;
      callback_data.type = event_type_;
      callback_data.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
      m_event_callback(service_id_, callback_data);
    }
  }
}

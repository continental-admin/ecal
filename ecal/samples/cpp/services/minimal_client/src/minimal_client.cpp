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

#include <ecal/ecal.h>

#include <iostream>
#include <chrono>
#include <thread>

// callback for service response
void OnServiceResponse(const eCAL::Registration::SEntityId& entity_id_, const struct eCAL::SServiceResponse& service_response_)
{
  switch (service_response_.call_state)
  {
    // service successful executed
  case call_state_executed:
  {
    std::cout << "Received response for method " << service_response_.method_name << " : " << service_response_.response << " from service id " << entity_id_.entity_id << " from host " << service_response_.host_name << std::endl;
  }
  break;
  // service execution failed
  case call_state_failed:
    std::cout << "Received error for method " << service_response_.method_name << " : " << service_response_.error_msg << " from service id " << entity_id_.entity_id << " from host " << service_response_.host_name << std::endl;
    break;
  default:
    break;
  }
}

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "minimal client");

  // create minimal service client
  eCAL::CServiceClientID minimal_client("service1", { {"echo", eCAL::SServiceMethodInformation()} });
  minimal_client.AddResponseCallback(OnServiceResponse);

  // are we connected to at least one service?
  while (!minimal_client.IsConnected())
  {
    std::cout << "Waiting for a service .." << std::endl;

    // sleep a second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  while(eCAL::Ok())
  {
    eCAL::SServiceResponse service_info;
    std::string method_name("echo");
    std::string request("Hello");

    // call all existing services
    for (const auto& entity_id : minimal_client.GetServiceIDs())
    {
      //////////////////////////////////////
      // Service call (blocking)
      //////////////////////////////////////
      const auto service_response = minimal_client.CallWithResponse(entity_id, method_name, request, -1);
      if (std::get<0>(service_response))
      {
        std::cout << std::endl << "Method 'echo' called with message : " << request << std::endl;
        switch (service_info.call_state)
        {
          // service successful executed
        case call_state_executed:
        {
          std::cout << "Received response : " << service_response.second.response << " from service id " << entity_id.entity_id << " from host " << service_info.host_name << std::endl;
        }
        break;
        // service execution failed
        case call_state_failed:
          std::cout << "Received error : " << service_info.error_msg << " from service id " << entity_id.entity_id << " from host " << service_info.host_name << std::endl;
          break;
        default:
          break;
        }
      }
      else
      {
        std::cout << "Method blocking call failed .." << std::endl << std::endl;
      }

      //////////////////////////////////////
      // Service call (with callback)
      //////////////////////////////////////
      if (minimal_client.CallWithCallback(entity_id, method_name, request))
      {
        std::cout << std::endl << "Method 'echo' called with message : " << request << std::endl;
      }
      else
      {
        std::cout << "Method callback call failed .." << std::endl << std::endl;
      }
    }

    // sleep a second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}

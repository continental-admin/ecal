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

#pragma once

#include <memory>
#include <functional>

#include <string>
#include <sstream>

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable: 4834)
#endif

#include <asio.hpp>
#include <utility>

#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#include "ecal/cimpl/ecal_callback_cimpl.h"

namespace eCAL
{
  namespace service
  {
    class ServerSessionBase
    {
    /////////////////////////////////////
    // Custom types for API
    /////////////////////////////////////
    //TODO: Remove or add again
    public:
      using ServiceCallbackT = std::function<int (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response)>; // TODO: make the request a const std::shared_ptr<const std::string>&
      using EventCallbackT   = std::function<void (eCAL_Server_Event, const std::string &)>;
      using DeleteCallbackT  = std::function<void (const std::shared_ptr<ServerSessionBase>&)>;

    /////////////////////////////////////
    // Constructor, Destructor, Create
    /////////////////////////////////////
    public:
      virtual ~ServerSessionBase() = default;

    protected:
      ServerSessionBase(asio::io_context& io_context_, const ServiceCallbackT& service_callback, const EventCallbackT& event_callback, const DeleteCallbackT& delete_callback) // TODO: remove callbacks
        : socket_          (io_context_)
        , service_callback_(service_callback)
        , event_callback_  (event_callback)
        , delete_callback_ (delete_callback)
      {}

    /////////////////////////////////////
    // Public API
    /////////////////////////////////////
    public:
      asio::ip::tcp::socket& socket() { return socket_; }
      virtual void start() = 0;
      virtual void stop()  = 0;

    /////////////////////////////////////
    // Member variables
    /////////////////////////////////////
    protected:
      asio::ip::tcp::socket socket_;

      const ServiceCallbackT service_callback_;
      const EventCallbackT   event_callback_;
      const DeleteCallbackT  delete_callback_; // TODO: The SessionBase actually doesn't call this callback at all. So it may be a good idea to move it to the implementations
    };

    } // namespace service
} // namespace eCAL
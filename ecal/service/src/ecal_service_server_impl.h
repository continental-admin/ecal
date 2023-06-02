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

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4834)
#endif
#include <asio.hpp>
#include <utility>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <ecal/cimpl/ecal_callback_cimpl.h>

#include <ecal/service/ecal_service_logger.h>
#include <ecal/service/ecal_service_server_session_types.h>

#include "ecal_service_server_session_impl_base.h"

namespace eCAL
{
  namespace service
  {
    class ServerImpl : public std::enable_shared_from_this<ServerImpl>
    {
    ///////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////

    public:
      static std::shared_ptr<ServerImpl> create(asio::io_context&             io_context
                                              , unsigned int                  protocol_version
                                              , std::uint16_t                 port
                                              , const ServerServiceCallbackT& service_callback
                                              , const ServerEventCallbackT&   event_callback
                                              , const LoggerT&                logger = default_logger("Service Server"));

    protected:
      ServerImpl(asio::io_context&              io_context
                , std::uint16_t                 port
                , const ServerServiceCallbackT& service_callback
                , const ServerEventCallbackT&   event_callback
                , const LoggerT&                logger);

      ServerImpl(const ServerImpl&)            = delete;                  // Copy construct
      ServerImpl(ServerImpl&&)                 = delete;                  // Move construct

      ServerImpl& operator=(const ServerImpl&) = delete;                  // Copy assign
      ServerImpl& operator=(ServerImpl&&)      = delete;                  // Move assign
  
    public:
      ~ServerImpl();

    ///////////////////////////////////////////
    // API
    ///////////////////////////////////////////
  
    public:
      bool          is_connected()         const;
      int           get_connection_count() const;
      std::uint16_t get_port()             const;

    private:
      void start_accept(unsigned int version);

    ///////////////////////////////////////////
    // Member Variables
    ///////////////////////////////////////////
  
    private:
      asio::io_context&             io_context_;
      asio::ip::tcp::acceptor       acceptor_;

      const ServerServiceCallbackT  service_callback_;
      const ServerEventCallbackT    event_callback_;

      mutable std::mutex            session_list_mutex_;
      std::vector<std::weak_ptr<ServerSessionBase>> session_list_; // TODO: decide whether std::vector is a good idea, as I will have to delete from the middle, when a session is closed.

      const LoggerT                 logger_;
    };
  } // namespace service
} // namespace eCAL

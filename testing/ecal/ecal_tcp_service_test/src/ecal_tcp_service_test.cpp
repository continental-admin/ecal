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

#include <gtest/gtest.h>

#include <asio.hpp>

#include <iostream>
#include <atomic>

#include <service/ecal_service_tcp_server.h>
#include <service/ecal_service_tcp_client_session.h>

eCAL::service::LoggerT critical_logger(const std::string& node_name)
{
      return [node_name](const eCAL::service::LogLevel log_level, const std::string& message)
                        {
                          switch (log_level)
                          {
                          case eCAL::service::LogLevel::Warning:
                            std::cerr << "[" + node_name + "] [Warning] " + message + "\n";
                            break;
                          case eCAL::service::LogLevel::Error:
                            std::cerr << "[" + node_name + "] [Error]   " + message + "\n";
                            break;
                          case eCAL::service::LogLevel::Fatal:
                            std::cerr << "[" + node_name + "] [Fatal]   " + message + "\n";
                            break;
                          default:
                            break;
                          }
                        };
}

constexpr std::uint8_t protocol_version = 1;

#if 1
TEST(RAII, TcpServiceServer)
{
  asio::io_context io_context;

  const eCAL::service::Server::ServiceCallbackT service_callback
          = [](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              std::cout << "Server got request: " << *request << std::endl;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };


  const eCAL::service::Server::EventCallbackT event_callback
          = [](eCAL_Server_Event event, const std::string& message) -> void
            {
              std::cout << "Event " << event << ": " << message << std::endl;
            };

  std::unique_ptr<std::thread> io_thread;

  // Test auto-destruction when the shared_ptr goes out of scope

  {
    std::weak_ptr<eCAL::service::Server> tcp_server_weak;

    {
      std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, protocol_version, 0, service_callback, event_callback);
      tcp_server_weak = tcp_server;

      EXPECT_NE(nullptr, tcp_server);
      EXPECT_NE(nullptr, tcp_server_weak.lock());

      io_thread = std::make_unique<std::thread>([&io_context]()
                                                {
                                                  io_context.run();
                                                });
    }

    EXPECT_EQ(nullptr, tcp_server_weak.lock());
  }

  io_context.stop();
  io_thread->join();
}
#endif

#if 1
TEST(RAII, TcpServiceServerAndClient)
{
  asio::io_context io_context;

  std::atomic<bool> response_callback_called(false);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
            = [](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
              {
                std::cout << "Server got request: " << *request << std::endl;
                *response = "Response on \"" + *request + "\"";
                return 0;
              };

  const eCAL::service::Server::EventCallbackT server_event_callback
            = [](eCAL_Server_Event event, const std::string& message) -> void
              {
                std::cout << "Event " << event << ": " << message << std::endl;
              };

  const eCAL::service::ClientSession::ResponseCallbackT client_slow_response_callback
            = [&response_callback_called](const eCAL::service::Error& /*error*/, const std::shared_ptr<std::string>& /*response*/) -> void
              {
                // This callback just wastes some time
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                response_callback_called = true;
              };

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};


  std::unique_ptr<std::thread> io_thread;

  // Test auto-destruction when the shared_ptr goes out of scope

  {
    std::weak_ptr<eCAL::service::Server>          tcp_server_weak;
    std::weak_ptr<eCAL::service::ClientSession>   tcp_client_weak;

    std::chrono::steady_clock::time_point         start_time;

    {
      std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
      tcp_server_weak = tcp_server;

      io_thread = std::make_unique<std::thread>([&io_context]()
                                                {
                                                  io_context.run();
                                                });

      EXPECT_EQ(tcp_server->get_connection_count(), 0);

      auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", tcp_server->get_port(), client_event_callback);
      tcp_client_weak = client_v1;

      client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_slow_response_callback);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      EXPECT_EQ(tcp_server->get_connection_count(), 1);

      // Start the "timer", when we are starting to delete the server and client
      start_time = std::chrono::steady_clock::now();
    }

    // The Client and server should both be deleted now.
    EXPECT_EQ(nullptr, tcp_server_weak.lock());
    EXPECT_EQ(nullptr, tcp_client_weak.lock());

    // The response callback should be finished, yet
    EXPECT_FALSE(response_callback_called);
  }

  io_context.stop();
  io_thread->join();

  // Now the response callback should be finished
  EXPECT_TRUE(response_callback_called);
}
#endif

#if 1
TEST(RAII, StopDuringServiceCall)
{
  asio::io_context io_context;

  std::atomic<bool> response_callback_called(false);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
            = [](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
              {
                std::cout << "Server got request: " << *request << std::endl;
                *response = "Response on \"" + *request + "\"";
                return 0;
              };

  const eCAL::service::Server::EventCallbackT server_event_callback
            = [](eCAL_Server_Event event, const std::string& message) -> void
              {
                std::cout << "Event " << event << ": " << message << std::endl;
              };

  const eCAL::service::ClientSession::ResponseCallbackT client_slow_response_callback
            = [&response_callback_called](const eCAL::service::Error /*error*/, const std::shared_ptr<std::string>& /*response*/) -> void
              {
                // This callback just wastes some time
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                response_callback_called = true;
              };

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};

  std::unique_ptr<std::thread> io_thread;

  // Test auto-destruction when the shared_ptr goes out of scope

  {
    std::weak_ptr<eCAL::service::Server>          tcp_server_weak;
    std::weak_ptr<eCAL::service::ClientSession> tcp_client_weak;

    {
      std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
      tcp_server_weak = tcp_server;

      io_thread = std::make_unique<std::thread>([&io_context]()
                                                {
                                                  io_context.run();
                                                });

      auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", tcp_server->get_port(), client_event_callback);
      tcp_client_weak = client_v1;

      client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_slow_response_callback);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    io_context.stop();

    // The client and server should be deleted already
    EXPECT_EQ(nullptr, tcp_server_weak.lock());
    EXPECT_EQ(nullptr, tcp_client_weak.lock());

    // The callback is probably still running
    EXPECT_FALSE(response_callback_called);

    // The callback should be finished by now and the session should be deleted
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(response_callback_called);
  }

  io_thread->join();
}
#endif

#if 1
TEST(Communication, SlowCommunication)
{
  asio::io_context io_context;

  std::atomic<int> num_server_service_callback_called           (0);
  std::atomic<int> num_client_response_callback_called          (0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called]
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              std::cout << "Server got request: " << *request << std::endl;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = []
            (eCAL_Server_Event event, const std::string& message) -> void
            {};

  const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = [&num_client_response_callback_called]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_FALSE(bool(error));
              num_client_response_callback_called++;
              std::cout << "Client got Response: " << *response << std::endl;
            };

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);

  EXPECT_NE(server->get_port(), 0);

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_client_response_callback_called          , 0);

    EXPECT_EQ(server->get_connection_count(), 0);
  }

  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  // Run the io_service. The server will always have work for the io_service, so we don't need a stub object
  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_client_response_callback_called          , 0);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  
  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_client_response_callback_called          , 1);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  // Call service again and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Called again"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 2);
    EXPECT_EQ(num_client_response_callback_called          , 2);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  // delete all objects
  client_v1 = nullptr;
  server    = nullptr;

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 1
TEST(CallbacksConnectDisconnect, ServerDisconnectsFirst)
{
  asio::io_context io_context;

  std::atomic<int> num_server_event_callback_called             (0);
  std::atomic<int> num_server_event_callback_called_connected   (0);
  std::atomic<int> num_server_event_callback_called_disconnected(0);
  
  std::atomic<int> num_client_event_callback_called             (0);
  std::atomic<int> num_client_event_callback_called_connected   (0);
  std::atomic<int> num_client_event_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = []
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };

  const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = []
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {};

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_event_callback_called++; 
              if (event == eCAL_Client_Event::client_event_connected)
                  num_client_event_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                  num_client_event_callback_called_disconnected++;
            };

  auto server    = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  // Run the io_service. The server will always have work for the io_service, so we don't need a stub object
  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);
  }

  // Server goes away
  server = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  client_v1 = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 1
TEST(CallbacksConnectDisconnect, ClientDisconnectsFirst)
{
  asio::io_context io_context;

  std::atomic<int> num_server_event_callback_called             (0);
  std::atomic<int> num_server_event_callback_called_connected   (0);
  std::atomic<int> num_server_event_callback_called_disconnected(0);
  
  std::atomic<int> num_client_event_callback_called             (0);
  std::atomic<int> num_client_event_callback_called_connected   (0);
  std::atomic<int> num_client_event_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = []
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };

  const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = []
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {};

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_event_callback_called++; 
              if (event == eCAL_Client_Event::client_event_connected)
                num_client_event_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                num_client_event_callback_called_disconnected++;
            };

  auto server    = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  // Run the io_service. The server will always have work for the io_service, so we don't need a stub object
  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);
  }

  // Client goes away
  client_v1 = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  server = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif


#if 1
TEST(CommunicationAndCallbacks, ClientsDisconnectFirst)
{
  asio::io_context io_context;

  std::atomic<int> num_server_service_callback_called           (0);
  std::atomic<int> num_server_event_callback_called             (0);
  std::atomic<int> num_server_event_callback_called_connected   (0);
  std::atomic<int> num_server_event_callback_called_disconnected(0);
  
  std::atomic<int> num_client_response_callback_called          (0);
  std::atomic<int> num_client_event_callback_called             (0);
  std::atomic<int> num_client_event_callback_called_connected   (0);
  std::atomic<int> num_client_event_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called]
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              std::cout << "Server got request: " << *request << std::endl;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };

  const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = [&num_client_response_callback_called]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_FALSE(bool(error));
              num_client_response_callback_called++;
              std::cout << "Client got Response: " << *response << std::endl;
            };

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_event_callback_called++; 
                
              if (event == eCAL_Client_Event::client_event_connected)
                  num_client_event_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                  num_client_event_callback_called_disconnected++;
            };

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);

  EXPECT_NE(server->get_port(), 0);

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_server_event_callback_called             , 0);
    EXPECT_EQ(num_server_event_callback_called_connected   , 0);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 0);
    EXPECT_EQ(num_client_event_callback_called             , 0);
    EXPECT_EQ(num_client_event_callback_called_connected   , 0);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), 0);
  }

  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  // Run the io_service. The server will always have work for the io_service, so we don't need a stub object
  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 0);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  
  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 1);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  // Call service again and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Called again"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 2);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 2);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  // delete all objects
  client_v1 = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_service_callback_called           , 2);
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_response_callback_called          , 2);
    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);

    EXPECT_EQ(server->get_connection_count(), 0);
  }

  server = nullptr;

  {
    EXPECT_EQ(num_server_service_callback_called           , 2);
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_response_callback_called          , 2);
    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 1
TEST(CommunicationAndCallbacks, ServerDisconnectsFirst)
{
  asio::io_context io_context;

  std::atomic<int> num_server_service_callback_called           (0);
  std::atomic<int> num_server_event_callback_called             (0);
  std::atomic<int> num_server_event_callback_called_connected   (0);
  std::atomic<int> num_server_event_callback_called_disconnected(0);
  
  std::atomic<int> num_client_response_callback_called          (0);
  std::atomic<int> num_client_event_callback_called             (0);
  std::atomic<int> num_client_event_callback_called_connected   (0);
  std::atomic<int> num_client_event_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called]
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };

  const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = [&num_client_response_callback_called]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_FALSE(bool(error));
              num_client_response_callback_called++;
            };

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_event_callback_called++; 
              if (event == eCAL_Client_Event::client_event_connected)
                  num_client_event_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                  num_client_event_callback_called_disconnected++;
            };

  auto server    = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  // Run the io_service. The server will always have work for the io_service, so we don't need a stub object
  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 0);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  
  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 1);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);
  }

  // Server goes away
  server = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_response_callback_called          , 1);
    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  client_v1 = nullptr;

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_response_callback_called          , 1);
    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

// TODO: Test get_connection_count() here
#if 1
TEST(CommunicationAndCallbacks, StressfulCommunication)
{
  constexpr int num_io_threads       = 10;
  constexpr int num_clients          = 10;
  constexpr int num_calls_per_client = 15;

  asio::io_context io_context;

  // TODO: test event callback
  std::atomic<int> num_server_service_callback_called           (0);
  std::atomic<int> num_server_event_callback_called             (0);
  std::atomic<int> num_server_event_callback_called_connected   (0);
  std::atomic<int> num_server_event_callback_called_disconnected(0);

  std::atomic<int> num_client_response_callback_called          (0);
  std::atomic<int> num_client_event_callback_called             (0);
  std::atomic<int> num_client_event_callback_called_connected   (0);
  std::atomic<int> num_client_event_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT service_callback
          = [&num_server_service_callback_called](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, service_callback, server_event_callback, critical_logger("Server"));

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_server_event_callback_called             , 0);
    EXPECT_EQ(num_server_event_callback_called_connected   , 0);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 0);
    EXPECT_EQ(num_client_event_callback_called             , 0);
    EXPECT_EQ(num_client_event_callback_called_connected   , 0);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);
  }

  // Run the io service a bunch of times, so we hopefully trigger any race condition that may exist
  std::vector<std::unique_ptr<std::thread>> io_threads;
  io_threads.reserve(num_io_threads);
  for (int i = 0; i < num_io_threads; i++)
  {
    io_threads.emplace_back(std::make_unique<std::thread>([&io_context]() { io_context.run(); }));
  }

  // Create all the clients
  std::vector<std::shared_ptr<eCAL::service::ClientSession>> client_list;
  client_list.reserve(num_clients);
  for (int c = 0; c < num_clients; c++)
  {
    const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_event_callback_called++; 
                
              if (event == eCAL_Client_Event::client_event_connected)
                num_client_event_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                num_client_event_callback_called_disconnected++;
            };
    client_list.push_back(eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback, critical_logger("Client " + std::to_string(c))));
  }

  // Directly run a bunch of clients and call each client a bunch of times
  for (int c = 0; c < client_list.size(); c++)
  {
    for (int i = 0; i < num_calls_per_client; i++)
    {
      const std::shared_ptr<std::string> request_string = std::make_shared<std::string>("Client " + std::to_string(c) + ", Call " + std::to_string(i));

      const eCAL::service::ClientSession::ResponseCallbackT response_callback
            = [&num_client_response_callback_called, request_string]
              (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
              {
                ASSERT_FALSE(error);
                num_client_response_callback_called++;
                const std::string expected_response = "Response on \"" + *request_string + "\"";
                ASSERT_EQ(*response, expected_response);
              };

      client_list[c]->async_call_service(request_string, response_callback);
    }
  }

  // Now wait a short time for the clients to connect to the server and all requests getting executed
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  {
    EXPECT_EQ(num_server_service_callback_called           , num_clients * num_calls_per_client);
    EXPECT_EQ(num_server_event_callback_called             , num_clients);
    EXPECT_EQ(num_server_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , num_clients * num_calls_per_client);
    EXPECT_EQ(num_client_event_callback_called             , num_clients);
    EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), num_clients);
  }

  // delete all objects
  server = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , num_clients * num_calls_per_client);
    EXPECT_EQ(num_server_event_callback_called             , num_clients * 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_server_event_callback_called_disconnected, num_clients);

    EXPECT_EQ(num_client_response_callback_called          , num_clients * num_calls_per_client);
    EXPECT_EQ(num_client_event_callback_called             , num_clients * 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_client_event_callback_called_disconnected, num_clients);
  }

  client_list.clear();

  // join all io_threads
  io_context.stop();
  for (int i = 0; i < io_threads.size(); i++)
  {
    io_threads[i]->join();
  }
  io_threads.clear();
}
#endif

#if 1
TEST(Callback, ServiceCallFromCallback)
{
  asio::io_context io_context;

  // TODO: test event callback
  std::atomic<int> num_server_service_callback_called(0);
  std::atomic<int> num_client_response_callback1_called(0);
  std::atomic<int> num_client_response_callback2_called(0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [](eCAL_Server_Event event, const std::string& message) -> void
            {};

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);

  EXPECT_EQ(num_server_service_callback_called, 0);
  EXPECT_EQ(num_client_response_callback1_called, 0);
  EXPECT_EQ(num_client_response_callback2_called, 0);

  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  const eCAL::service::ClientSession::ResponseCallbackT response_callback
          = [&num_client_response_callback1_called, &num_client_response_callback2_called, client_v1]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& /*response*/) -> void
            {
              EXPECT_FALSE(bool(error));
              num_client_response_callback1_called++;
              client_v1->async_call_service(std::make_shared<std::string>("2")
                                           , [&num_client_response_callback2_called](const eCAL::service::Error& error, const auto& /*response*/) -> void
                                              {
                                                EXPECT_FALSE(bool(error));
                                                num_client_response_callback2_called++;
                                              });
            };


  // Run the io_service. The server will always have work for the io_service, so we don't need a stub object
  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("1"), response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_EQ(num_server_service_callback_called, 2);
  EXPECT_EQ(num_client_response_callback1_called, 1);
  EXPECT_EQ(num_client_response_callback2_called, 1);

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 0
TEST(Communication, ErrorCallback)
{
  asio::io_context io_context;

  // TODO: test event callback
  std::atomic<int> num_server_service_callback_called(0);
  std::atomic<int> num_client_response_callback_called(0);

  const eCAL::service::Server::ServiceCallbackT service_callback
          = [&num_server_service_callback_called](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              return 0;
            };

  const eCAL::service::Server::EventCallbackT event_callback
          = [](eCAL_Server_Event event, const std::string& message) -> void
            {
            };

  const eCAL::service::ClientSessionV1::ResponseCallbackT response_callback
          = [&num_client_response_callback_called](const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_TRUE(bool(error));
              num_client_response_callback_called++;
            };

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, service_callback, event_callback);

  EXPECT_NE(server->get_port(), 0);
  EXPECT_EQ(num_server_service_callback_called, 0);
  EXPECT_EQ(num_client_response_callback_called, 0);

  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"NonExistingEndpoint", 12345);

  // Run the io_service. The server will always have work for the io_service, so we don't need a stub object
  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Hello World"), response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  EXPECT_EQ(num_server_service_callback_called, 0);
  EXPECT_EQ(num_client_response_callback_called, 1);

  // delete all objects
  server = nullptr;
  client_v1 = nullptr;

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif



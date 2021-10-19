#pragma once

#include <functional>
#include <deque>

#include <asio.hpp>

#include "tcpub_logger_abstraction.h"

namespace tcpub
{
  class PublisherSession
    : public std::enable_shared_from_this<PublisherSession>
  {
  //////////////////////////////////////////////
  /// Constructor & Destructor
  //////////////////////////////////////////////
  public:
    PublisherSession(const std::shared_ptr<asio::io_service>&                               io_service
                    , const std::function<void(const std::shared_ptr<PublisherSession>&)>&  session_closed_handler
                    , const tcpub::logger::logger_t&                                   log_function);

    // Copy
    PublisherSession(const PublisherSession&)            = delete;
    PublisherSession& operator=(const PublisherSession&) = delete;

    // Move
    PublisherSession& operator=(PublisherSession&&)      = delete;
    PublisherSession(PublisherSession&&)                 = delete;

    ~PublisherSession();

  //////////////////////////////////////////////
  /// Start & Stop
  //////////////////////////////////////////////
  
  public:
    void start();
    void cancel();

  private:
    void sessionClosedHandler();
  
  //////////////////////////////////////////////
  /// Send Data
  //////////////////////////////////////////////
  
  public:
    void sendDataBuffer(const std::shared_ptr<std::vector<char>>& buffer);
  private:
    void sendBufferToClient(const std::shared_ptr<std::vector<char>>& buffer);

  //////////////////////////////////////////////
  /// (Status-) getters
  //////////////////////////////////////////////
  
  public:
    asio::ip::tcp::socket& getSocket();
    std::string localEndpointToString() const;
    std::string remoteEndpointToString() const;
    std::string endpointToString() const;

  //////////////////////////////////////////////
  /// Member variables
  //////////////////////////////////////////////
  private:
    // Asio IO Service
    std::shared_ptr<asio::io_service> io_service_;

    // Whether the session has been canceled
    std::atomic<bool>  canceled_;

    // Handlers
    const std::function<void(const std::shared_ptr<PublisherSession>&)>  session_closed_handler_;
    // Logger                                    
    const logger::logger_t                                               log_;                        /// Function for logging

    // TCP Socket & Queue (protected by the strand!)
    asio::ip::tcp::socket     data_socket_;
    asio::io_service::strand  data_strand_;

    // Variable holding if we are currently sending any data and what data to send next
    std::mutex                         next_buffer_mutex_;
    bool                               sending_in_progress_;
    std::shared_ptr<std::vector<char>> next_buffer_to_send_;
  };
}
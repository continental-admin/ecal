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
 * @file   dynamic.h
 * @brief  eCAL dynamic reflection exception
**/

#pragma once

#include <exception>
#include <optional>
#include <mutex>
#include <string>

#include <ecal/ecal_subscriber_v5.h>
#include <ecal/ecal_util.h>

namespace eCAL
{
  /* @cond */
  class DynamicReflectionException : public std::exception
  {
  public:
    DynamicReflectionException(const std::string& message) : message_(message) {}
    virtual const char* what() const noexcept { return message_.c_str(); }
  private:
    std::string message_;
  };
  /* @endcond */

  /* @cond */
  inline bool StrEmptyOrNull(const std::string& str)
  {
    if (str.empty())
    {
      return true;
    }
    else
    {
      for (auto c : str)
      {
        if (c != '\0')
        {
          return false;
        }
      }
      return true;
    }
  }
  /* @endcond */


  template <typename T, typename DynamicDeserializer>
  class CDynamicMessageSubscriber final : public v5::CSubscriber
  {
  public:
    /**
     * @brief  Constructor.
    **/
    CDynamicMessageSubscriber() : v5::CSubscriber()
    {
    }

    /**
    * @brief  Constructor.
    *
    * @param topic_name_  Unique topic name.
    **/
    CDynamicMessageSubscriber(const std::string& topic_name_) : v5::CSubscriber()
      , m_deserializer()
    {
      v5::CSubscriber::Create(topic_name_);
    }

    ~CDynamicMessageSubscriber() noexcept
    {
      Destroy();
    };

    /**
    * @brief  Copy Constructor is not available.
    **/
    CDynamicMessageSubscriber(const CDynamicMessageSubscriber&) = delete;

    /**
    * @brief  Copy Constructor is not available.
    **/
    CDynamicMessageSubscriber& operator=(const CDynamicMessageSubscriber&) = delete;

    /**
    * @brief  Move Constructor
    **/
    CDynamicMessageSubscriber(CDynamicMessageSubscriber&& rhs)
      : v5::CSubscriber(std::move(rhs))
      , m_cb_callback(std::move(rhs.m_cb_callback))
      , m_deserializer(std::move(rhs.m_deserializer))
    {
      bool has_callback = (m_cb_callback != nullptr);

      if (has_callback)
      {
        // the callback bound to the CSubscriber belongs to rhs, bind to this callback instead
        v5::CSubscriber::RemReceiveCallback();
        auto callback = std::bind(&CDynamicMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
        v5::CSubscriber::AddReceiveCallback(callback);
      }
    }

    /**
    * @brief  Move assignment not available
    **/
    CDynamicMessageSubscriber& operator=(CDynamicMessageSubscriber&& rhs) = delete;

    /**
     * @brief Destroys this object.
     *
     * @return  true if it succeeds, false if it fails.
    **/
    bool Destroy()
    {
      RemReceiveCallback();
      return(v5::CSubscriber::Destroy());
    }

    /**
     * @brief eCAL message receive callback function
     *
     * @param topic_id_  Topic id of the data source (publisher).
     * @param msg_       Message content.
     * @param time_      Message time stamp.
     * @param clock_     Message writer clock.
     * @param id_        Message id.
     **/
    using MsgReceiveCallbackT = std::function<void(const Registration::STopicId& topic_id_, const T& msg_, long long time_, long long clock_, long long id_)>;

    /**
     * @brief  Add receive callback for incoming messages.
     *
     * @param callback_  The callback function.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool AddReceiveCallback(MsgReceiveCallbackT callback_)
    {
      RemReceiveCallback();

      {
        std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
        m_cb_callback = callback_;
      }

      ReceiveIDCallbackT callback = std::bind(&CDynamicMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
      return(v5::CSubscriber::AddReceiveCallback(callback));
    }

    /**
     * @brief  Remove receive callback for incoming messages.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool RemReceiveCallback()
    {
      bool ret = v5::CSubscriber::RemReceiveCallback();

      std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
      if (m_cb_callback == nullptr) return(false);
      m_cb_callback = nullptr;
      return(ret);
    }

    /**
     * @brief Callback function in case an error occurs.
     *
     * @param error  The error message string.
    **/
    using ErrorCallbackT = std::function<void(const std::string& error)>;

    /**
     * @brief Add callback function in case an error occurs.
     *
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not.
    **/
    bool AddErrorCallback(ErrorCallbackT callback_)
    {
      std::lock_guard<std::mutex> callback_lock(m_error_callback_mutex);
      m_error_callback = callback_;

      return true;
    }
    
    /**
     * @brief Remove callback function in case an error occurs.
     *
     * @return  True if succeeded, false if not.
    **/
    bool RemErrorCallback()
    {
      std::lock_guard<std::mutex> callback_lock(m_error_callback_mutex);
      m_error_callback = nullptr;

      return true;
    }

  private:
    void ReceiveCallback(const Registration::STopicId& topic_id_, const SDataTypeInformation& topic_info_, const struct SReceiveCallbackData& data_)
    {
      MsgReceiveCallbackT fn_callback = nullptr;
      {
        std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
        fn_callback = m_cb_callback;
      }

      if (fn_callback == nullptr) return;

      try
      {
        auto msg = m_deserializer.Deserialize(data_.buf, data_.size, topic_info_);
        fn_callback(topic_id_, msg, data_.time, data_.clock, data_.id);
      }
      catch (const DynamicReflectionException& e)
      {
        CallErrorCallback(std::string("Dynamic Deserialization: Error deserializing data: ") + e.what() );
      }
    }

    void CallErrorCallback(const std::string& message)
    {
      ErrorCallbackT error_callback = nullptr;
      {
        std::lock_guard<std::mutex> callback_lock(m_error_callback_mutex);
        error_callback = m_error_callback;
      }
      if (error_callback)
      {
        error_callback(message);
      }
    }

    std::mutex           m_cb_callback_mutex;
    MsgReceiveCallbackT  m_cb_callback;
    std::mutex           m_error_callback_mutex;
    ErrorCallbackT       m_error_callback;
    DynamicDeserializer  m_deserializer;
  };

}

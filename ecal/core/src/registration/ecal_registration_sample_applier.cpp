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

#include "registration/ecal_registration_sample_applier.h"

namespace eCAL
{
  namespace Registration
  {
    //////////////////////////////////////////////////////////////////
    // CSampleApplier
    //////////////////////////////////////////////////////////////////
    CSampleApplier::CSampleApplier(const SampleApplier::SAttributes& attr_)
      : m_attributes(attr_)
    {
    }

    void CSampleApplier::EnableLoopback(bool state_)
    {
      m_attributes.loopback = state_;
    }

    bool CSampleApplier::ApplySample(const Registration::Sample& sample_)
    {
      if (!AcceptRegistrationSample(sample_))
      {
        Logging::Log(log_level_debug1, "CSampleApplier::ApplySample : Incoming sample discarded");
        return false;
      }

      // forward all registration samples to outside "customer" (e.g. monitoring, descgate, pub/subgate/client/service gates)
      {
        const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
        for (const auto& iter : m_callback_custom_apply_sample_map)
        {
          iter.second(sample_);
        }
      }
      return true;
    }

    bool CSampleApplier::IsHostGroupMember(const Registration::Sample& sample_) const
    {
      std::string host_group_name;
      const std::string host_name = sample_.identifier.host_name;
      switch (sample_.cmd_type)
      {
      case bct_reg_publisher:
      case bct_unreg_publisher:
      case bct_reg_subscriber:
      case bct_unreg_subscriber:
        host_group_name = sample_.topic.hgname;
        break;
      case bct_reg_service:
      case bct_unreg_service:
        //host_group_name = sample_.service.hgname;  // TODO: we need to add hgname attribute to services
        break;
      case bct_reg_client:
      case bct_unreg_client:
        //host_group_name = sample_.client.hgname;  // TODO: we need to add hgname attribute to clients
        break;
      default:
        break;
      }

      const std::string& sample_host_group_name = host_group_name.empty() ? host_name : host_group_name;

      if (sample_host_group_name.empty() || m_attributes.host_group_name.empty())
        return false;
      if (sample_host_group_name != m_attributes.host_group_name)
        return false;

      return true;
    }

    bool CSampleApplier::IsSameProcess(const Registration::Sample& sample_) const
    {
      // is this actually sufficient? should we also check host_name?
      const int32_t pid = sample_.identifier.process_id;
      return pid == m_pid;
    }

    bool CSampleApplier::AcceptRegistrationSample(const Registration::Sample& sample_)
    {
      // check if the sample is from the same host group
      if (IsHostGroupMember(sample_))
      {
        // register if the sample is from another process
        // or if loopback mode is enabled
        return !IsSameProcess(sample_) || m_attributes.loopback;
      }
      else
      {
        // if the sample is from an external host, register only if network mode is enabled
        return m_attributes.network_enabled;
      }
    }

    void CSampleApplier::SetCustomApplySampleCallback(const std::string& customer_, const ApplySampleCallbackT& callback_)
    {
      const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
      m_callback_custom_apply_sample_map[customer_] = callback_;
    }

    void CSampleApplier::RemCustomApplySampleCallback(const std::string& customer_)
    {
      const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
      auto iter = m_callback_custom_apply_sample_map.find(customer_);
      if (iter != m_callback_custom_apply_sample_map.end())
      {
        m_callback_custom_apply_sample_map.erase(iter);
      }
    }
  }
}
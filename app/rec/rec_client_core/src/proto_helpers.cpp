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

#include <rec_client_core/proto_helpers.h>

namespace eCAL
{
  namespace rec
  {
    namespace proto_helpers
    {
      void ToProtobuf(const eCAL::rec::RecHdf5JobStatus&        hdf5_job_status,             eCAL::pb::rec_client::State::RecHdf5Status&            hdf5_status_pb)
      {
        // total_length_secs
        hdf5_status_pb.set_total_length_secs    (std::chrono::duration_cast<std::chrono::duration<double>>(hdf5_job_status.total_length_).count());
        
        // total_frame_count
        hdf5_status_pb.set_total_frame_count    (hdf5_job_status.total_frame_count_);
        
        // unflushed_frame_count
        hdf5_status_pb.set_unflushed_frame_count(hdf5_job_status.unflushed_frame_count_);
        
        // info_ok
        hdf5_status_pb.set_info_ok              (hdf5_job_status.info_.first);
        
        // info_message
        hdf5_status_pb.set_info_message         (hdf5_job_status.info_.second);
      }

      void ToProtobuf(const eCAL::rec::RecAddonJobStatus&       rec_addon_job_status,        eCAL::pb::rec_client::State::RecAddonJobStatus&        rec_addon_job_status_pb)
      {
        // state
        rec_addon_job_status_pb.set_state                (ToProtobuf(rec_addon_job_status.state_));

        // total_frame_count
        rec_addon_job_status_pb.set_total_frame_count    (rec_addon_job_status.total_frame_count_);

        // unflushed_frame_count
        rec_addon_job_status_pb.set_unflushed_frame_count(rec_addon_job_status.unflushed_frame_count_);

        // info_ok
        rec_addon_job_status_pb.set_info_ok              (rec_addon_job_status.info_.first);

        //info_message
        rec_addon_job_status_pb.set_info_message         (rec_addon_job_status.info_.second);
      }

      void ToProtobuf(const eCAL::rec::RecAddonJobStatus::State rec_addon_job_status_state , eCAL::pb::rec_client::State::RecAddonJobStatus::State& rec_addon_job_status_state_pb)
      {
        rec_addon_job_status_state_pb = ToProtobuf(rec_addon_job_status_state);
      }

      void ToProtobuf(const eCAL::rec::UploadStatus&            upload_status,               eCAL::pb::rec_client::State::UploadStatus&             upload_status_pb)
      {
        // bytes_total_size
        upload_status_pb.set_bytes_total_size(upload_status.bytes_total_size_);

        // bytes_uploaded
        upload_status_pb.set_bytes_uploaded  (upload_status.bytes_uploaded_);

        // info_ok
        upload_status_pb.set_info_ok         (upload_status.info_.first);

        // info_message
        upload_status_pb.set_info_message    (upload_status.info_.second);
      }

      void ToProtobuf(const eCAL::rec::JobState                 job_state,                   eCAL::pb::rec_client::State::JobState&                 job_state_pb)
      {
        job_state_pb = ToProtobuf(job_state);
      }

      void ToProtobuf(const eCAL::rec::JobStatus&               job_status,                  eCAL::pb::rec_client::State::JobStatus&                job_status_pb)
      {
        // job_id
        job_status_pb.set_job_id(job_status.job_id_);

        // state
        job_status_pb.set_state(ToProtobuf(job_status.state_));

        // rec_hdf5_status
        ToProtobuf(job_status.rec_hdf5_status_, *job_status_pb.mutable_rec_hdf5_status());

        // rec_addon_statuses
        for (const auto& rec_addon_status : job_status.rec_addon_statuses_)
        {
          (*job_status_pb.mutable_rec_addon_statuses())[rec_addon_status.first] = ToProtobuf(rec_addon_status.second);
        }

        // upload_status
        ToProtobuf(job_status.upload_status_, *job_status_pb.mutable_upload_status());

        // is_deleted
        job_status_pb.set_is_deleted(job_status.is_deleted_);
      }

      void ToProtobuf(const eCAL::rec::RecorderAddonStatus&     rec_addon_status,            eCAL::pb::rec_client::State::RecorderAddonStatus&      rec_addon_status_pb)
      {
        // addon_executable_path
        rec_addon_status_pb.set_addon_executable_path        (rec_addon_status.addon_executable_path_);

        // addon_id
        rec_addon_status_pb.set_addon_id                     (rec_addon_status.addon_id_);

        // initialized
        rec_addon_status_pb.set_initialized                  (rec_addon_status.initialized_);

        // name
        rec_addon_status_pb.set_name                         (rec_addon_status.name_);

        // pre_buffer_length_frame_count
        rec_addon_status_pb.set_pre_buffer_length_frame_count(rec_addon_status.pre_buffer_length_frame_count_);

        // info_ok
        rec_addon_status_pb.set_info_ok                      (rec_addon_status.info_.first);

        // info_message
        rec_addon_status_pb.set_info_message                 (rec_addon_status.info_.second);
      }

      void ToProtobuf(const eCAL::rec::RecorderStatus& rec_status, const std::string& hostname, eCAL::pb::rec_client::State&                        rec_status_pb)
      {
        // hostname
        rec_status_pb.set_hostname                            (hostname);

        // pid
        rec_status_pb.set_pid                                 (rec_status.pid_);

        // initialized
        rec_status_pb.set_initialized                         (rec_status.initialized_);

        // pre_buffer_length_frames_count
        rec_status_pb.set_pre_buffer_length_frames_count      (rec_status.pre_buffer_length_.first);

        // pre_buffer_length_secs
        rec_status_pb.set_pre_buffer_length_secs              (std::chrono::duration_cast<std::chrono::duration<double>>(rec_status.pre_buffer_length_.second).count());

        // subscribed_topics
        for (const std::string& subscribed_topic : rec_status.subscribed_topics_)
        {
          rec_status_pb.add_subscribed_topics(subscribed_topic);
        }

        // addon_statuses
        for (const eCAL::rec::RecorderAddonStatus& rec_addon_status : rec_status.addon_statuses_)
        {
          eCAL::pb::rec_client::State_RecorderAddonStatus* new_addon_status_pb = rec_status_pb.add_addon_statuses();
          ToProtobuf(rec_addon_status, *new_addon_status_pb);
        }

        // job_statuses
        for (const eCAL::rec::JobStatus& job_status : rec_status.job_statuses_)
        {
          auto job_status_pb = rec_status_pb.add_job_statuses();
          ToProtobuf(job_status, *job_status_pb);
        }

        // info_ok
        rec_status_pb.set_info_ok     (rec_status.info_.first);

        // info_message
        rec_status_pb.set_info_message(rec_status.info_.second);

        // timestamp_nsecs
        rec_status_pb.set_timestamp_nsecs                     (std::chrono::duration_cast<std::chrono::nanoseconds>(rec_status.timestamp_.time_since_epoch()).count());
      }


      eCAL::pb::rec_client::State::RecHdf5Status            ToProtobuf(const eCAL::rec::RecHdf5JobStatus&        hdf5_job_status)
      {
        eCAL::pb::rec_client::State::RecHdf5Status result;
        ToProtobuf(hdf5_job_status, result);
        return result;
      }

      eCAL::pb::rec_client::State::RecAddonJobStatus        ToProtobuf(const eCAL::rec::RecAddonJobStatus&       rec_addon_job_status)
      {
        eCAL::pb::rec_client::State::RecAddonJobStatus result;
        ToProtobuf(rec_addon_job_status, result);
        return result;
      }

      eCAL::pb::rec_client::State::RecAddonJobStatus::State ToProtobuf(const eCAL::rec::RecAddonJobStatus::State rec_addon_job_status_state)
      {
        switch (rec_addon_job_status_state)
        {
        case eCAL::rec::RecAddonJobStatus::State::NotStarted:
          return eCAL::pb::rec_client::State::RecAddonJobStatus::State::State_RecAddonJobStatus_State_NotStarted;
        case eCAL::rec::RecAddonJobStatus::State::Recording:
          return eCAL::pb::rec_client::State::RecAddonJobStatus::State::State_RecAddonJobStatus_State_Recording;
        case eCAL::rec::RecAddonJobStatus::State::Flushing:
          return eCAL::pb::rec_client::State::RecAddonJobStatus::State::State_RecAddonJobStatus_State_Flushing;
        case eCAL::rec::RecAddonJobStatus::State::FinishedFlushing:
          return eCAL::pb::rec_client::State::RecAddonJobStatus::State::State_RecAddonJobStatus_State_FinishedFlushing;

        default:
          return eCAL::pb::rec_client::State::RecAddonJobStatus::State::State_RecAddonJobStatus_State_NotStarted;
        }

      }

      eCAL::pb::rec_client::State::UploadStatus             ToProtobuf(const eCAL::rec::UploadStatus&            upload_status)
      {
        eCAL::pb::rec_client::State::UploadStatus result;
        ToProtobuf(upload_status, result);
        return result;
      }

      eCAL::pb::rec_client::State::JobState                 ToProtobuf(const eCAL::rec::JobState                 job_state)
      {
        switch (job_state)
        {
        case eCAL::rec::JobState::NotStarted:
          return eCAL::pb::rec_client::State::JobState::State_JobState_NotStarted;
        case eCAL::rec::JobState::Recording:
          return eCAL::pb::rec_client::State::JobState::State_JobState_Recording;
        case eCAL::rec::JobState::Flushing:
          return eCAL::pb::rec_client::State::JobState::State_JobState_Flushing;
        case eCAL::rec::JobState::FinishedFlushing:
          return eCAL::pb::rec_client::State::JobState::State_JobState_FinishedFlushing;
        case eCAL::rec::JobState::Uploading:
          return eCAL::pb::rec_client::State::JobState::State_JobState_Uploading;
        case eCAL::rec::JobState::FinishedUploading:
          return eCAL::pb::rec_client::State::JobState::State_JobState_FinishedUploading;

        default:
          return eCAL::pb::rec_client::State::JobState::State_JobState_NotStarted;
        }
      }

      eCAL::pb::rec_client::State::JobStatus                ToProtobuf(const eCAL::rec::JobStatus&               job_status)
      {
        eCAL::pb::rec_client::State::JobStatus result;
        ToProtobuf(job_status, result);
        return result;
      }

      eCAL::pb::rec_client::State::RecorderAddonStatus      ToProtobuf(const eCAL::rec::RecorderAddonStatus&     rec_addon_status)
      {
        eCAL::pb::rec_client::State::RecorderAddonStatus result;
        ToProtobuf(rec_addon_status, result);
        return result;
      }

      eCAL::pb::rec_client::State                           ToProtobuf(const eCAL::rec::RecorderStatus& rec_status, const std::string& hostname)
      {
        eCAL::pb::rec_client::State result;
        ToProtobuf(rec_status, hostname, result);
        return result;
      }

    }
  }
}
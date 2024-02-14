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
 * @file   writer.h
 * @brief  Base class for low level measurement writing operations
**/

#pragma once

#include <functional>
#include <set>
#include <string>
#include <memory>

#include <ecal/measurement/base/types.h>

namespace eCAL
{
  namespace experimental
  {
    namespace measurement
    {
      namespace base
      {
        /**
         * @brief eCAL Measurement Writer API
        **/
        class Writer
        {
        public:
          /**
           * @brief Constructor
          **/
          Writer() = default;

          /**
           * @brief Destructor
          **/
          virtual ~Writer() = default;

          /**
           * @brief Copy operator
          **/
          Writer(const Writer& other) = delete;
          Writer& operator=(const Writer& other) = delete;

          /**
          * @brief Move operator
          **/
          Writer(Writer&&) = default;
          Writer& operator=(Writer&&) = default;

          /**
           * @brief Open file
           *
           * @param path     Input file path / measurement directory path.
           *
           *                 Default measurement directory structure:
           *                  - root directory e.g.: M:\measurement_directory\measurement01
           *                  - documents directory:                                |_doc
           *                  - hosts directories:                                  |_Host1 (e.g.: CARPC01)
           *                                                                        |_Host2 (e.g.: CARPC02)
           *
           *                 File path as output
           *                  - full path to  measurement directory (recommended with host name) (e.g.: M:\measurement_directory\measurement01\CARPC01),
           *                  - to set the name of the actual hdf5 file use SetFileBaseName method.
           *
           * @param access   Access type
           *
           * @return         true if output measurement directory structure can be accessed/created, false otherwise.
          **/
          virtual bool Open(const std::string& path) = 0;

          /**
           * @brief Close file
           *
           * @return         true if succeeds, false if it fails
          **/
          virtual bool Close() = 0;

          /**
           * @brief Checks if file/measurement is ok
           *
           * @return  true if location is accessible, false otherwise
          **/
          virtual bool IsOk() const = 0;

          /**
           * @brief Gets maximum allowed size for an individual file
           *
           * @return       maximum size in MB
          **/
          virtual size_t GetMaxSizePerFile() const = 0;

          /**
           * @brief Sets maximum allowed size for an individual file
           *
           * @param size   maximum size in MB
          **/
          virtual void SetMaxSizePerFile(size_t size) = 0;

          /**
          * @brief Whether each Channel shall be writte in its own file
          *
          * When enabled, data is clustered by channel and each channel is written
          * to its own file. The filenames will consist of the basename and the
          * channel name.
          *
          * @return true, if one file per channel is enabled
          */
          virtual bool IsOneFilePerChannelEnabled() const = 0;

          /**
          * @brief Enable / disable the creation of one individual file per channel
          *
          * When enabled, data is clustered by channel and each channel is written
          * to its own file. The filenames will consist of the basename and the
          * channel name.
          *
          * @param enabled   Whether one file shall be created per channel
          */
          virtual void SetOneFilePerChannelEnabled(bool enabled) = 0;

          /**
           * @brief Set data type information of the given channel
           *
           * @param channel_name  channel name
           * @param info          datatype info of the channel
           *
           * @return              channel type
          **/
          virtual void SetChannelDataTypeInformation(const std::string & channel_name, const DataTypeInformation& info) = 0;

          /**
           * @brief Set measurement file base name (desired name for the actual hdf5 files that will be created)
           *
           * @param base_name        Name of the hdf5 files that will be created.
          **/
          virtual void SetFileBaseName(const std::string& base_name) = 0;

          /**
           * @brief Add entry to file
           *
           * @param data           data to be added
           * @param size           size of the data
           * @param snd_timestamp  send time stamp
           * @param rcv_timestamp  receive time stamp
           * @param channel_name   channel name
           * @param id             message id
           * @param clock          message clock
           *
           * @return              true if succeeds, false if it fails
          **/
          virtual bool AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock) = 0;

        };
      }
    }
  }
}

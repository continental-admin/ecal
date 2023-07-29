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
 * @file   ecal_payload_writer.h
 * @brief  eCAL payload writer base class
**/

#pragma once

#include <cstddef>
#include <cstring>

namespace eCAL
{
  /**
   * @brief Base payload writer class to allow zero copy memory operations.
   *
   * This class serves as the base class for payload writers, allowing zero-copy memory
   * operations. The `Write` and `Update` calls may operate on the target memory file
   * directly in zero-copy mode.
   */
  class CPayloadWriter
  {
  public:
    /**
     * @brief Default constructor for CPayloadWriter.
     */
    CPayloadWriter() = default;

    /**
     * @brief Virtual destructor for CPayloadWriter.
     */
    virtual ~CPayloadWriter() = default;

    /**
     * @brief Copy constructor (deleted).
     */
    CPayloadWriter(const CPayloadWriter&) = default;

    /**
     * @brief Move constructor (deleted).
     */
    CPayloadWriter(CPayloadWriter&&) = default;

    /**
     * @brief Copy assignment operator (deleted).
     */
    CPayloadWriter& operator=(const CPayloadWriter&) = default;

    /**
     * @brief Move assignment operator (deleted).
     */
    CPayloadWriter& operator=(CPayloadWriter&&) = default;

    /**
     * @brief Perform a full write operation with uninitialized memory.
     *
     * This virtual function allows derived classes to perform a full write operation
     * when the provisioned memory is uninitialized.
     *
     * @param buffer_ Pointer to the buffer containing the data to be written.
     * @param size_   Size of the data to be written.
     *
     * @return True if the write operation is successful, false otherwise.
     */
    virtual bool Write(void* buffer_, size_t size_) = 0;

    /**
     * @brief Perform a partial write operation or modify existing data.
     *
     * This virtual function allows derived classes to perform a partial write operation
     * or modify existing data when the provisioned memory is already initialized and
     * contains the data from the last write operation. By default, this operation will
     * just call the `Write` function.
     *
     * @param buffer_ Pointer to the buffer containing the data to be written or modified.
     * @param size_   Size of the data to be written or modified.
     *
     * @return True if the write/update operation is successful, false otherwise.
     */
    virtual bool Update(void* buffer_, size_t size_) { return Write(buffer_, size_); };

    /**
     * @brief Get the size of the required memory.
     *
     * This virtual function allows derived classes to provide the size of the memory
     * that eCAL needs to allocate.
     *
     * @return The size of the required memory.
     */
    virtual size_t GetSize() = 0;
  };

  /**
   * @brief Payload writer class that wraps a classic (void*, size_t) interface.
   *
   * This class is a payload writer that wraps a classic interface using `void*` and `size_t`
   * arguments. It inherits from the base class CPayloadWriter, allowing zero-copy memory
   * operations.
   */
  class CBufferPayloadWriter : public CPayloadWriter
  {
  public:
    /**
     * @brief Constructor for CBufferPayloadWriter.
     *
     * @param buffer_ Pointer to the buffer containing the data to be written.
     * @param size_   Size of the data to be written.
     */
    CBufferPayloadWriter(const void* const buffer_, size_t size_) : m_buffer(buffer_), m_size(size_) {};

    /**
     * @brief Make a dump memory copy of the stored buffer.
     *
     * This function performs a dump memory copy of the stored buffer to the provided
     * memory location (buffer_) with the specified size (size_). The size of the provided
     * memory buffer should be equal to or greater than the stored buffer size to avoid
     * memory corruption.
     *
     * @param buffer_  Pointer to the target buffer where the data will be copied.
     * @param size_    Size of the target buffer.
     *
     * @return True if the copy operation is successful, false otherwise.
     */
    bool Write(void* buffer_, size_t size_) override {
      if (buffer_ == nullptr) return false;
      if (size_ < m_size) return false;
      if (m_buffer == nullptr) return false;
      if (m_size == 0) return false;
      memcpy(buffer_, m_buffer, m_size);
      return true;
    }

    /**
     * @brief Get the size of the memory that needs to be copied.
     *
     * This function returns the size of the memory buffer that needs to be copied during
     * the write operation. It is used by the base class CPayloadWriter to allocate the
     * required memory for eCAL.
     *
     * @return The size of the memory that needs to be copied.
     */
    size_t GetSize() override { return m_size; };

  private:
    const void* m_buffer = nullptr;  ///< Pointer to the buffer containing the data to be written.
    size_t      m_size = 0;          ///< Size of the data to be written.
  };

} // namespace eCAL

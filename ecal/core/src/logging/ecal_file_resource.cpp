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

#include "ecal_file_resource.h"

#include <cstdarg>

namespace eCAL
{
  namespace Logging
  {
    void FileDeleter::operator()(FILE* file) const 
    {
      if (file) {
        fclose(file);
      }
    }

    FileResource::FileResource() : m_file(nullptr)
    {
    }

    bool FileResource::fopen(const char* filename, const char* mode)
    {
      m_file.reset(std::fopen(filename, mode));

      if (m_file == nullptr)
      {
        return false;
      }
      else
      {
        return true;
      }
    }

    void FileResource::fprintf(const char* format, ...) 
    {
      va_list args;
      va_start(args, format);
      vfprintf(m_file.get(), format, args);
      va_end(args);
    }

    void FileResource::fflush() 
    {
      std::fflush(m_file.get());
    }

    void FileResource::fclose() 
    {
      m_file.reset();
    }

    bool FileResource::isOpen() const
    {
      return m_file != nullptr;
    }
  }
}

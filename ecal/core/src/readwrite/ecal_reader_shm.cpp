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
 * @brief  memory file data reader
**/

#include <ecal/ecal.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/layer.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "io/ecal_memfile_pool.h"
#include "readwrite/ecal_reader_shm.h"

#include <iostream>

namespace eCAL
{
  template<> std::shared_ptr<CSHMLayer> CReaderLayer<eCAL::CSHMLayer>::layer(nullptr);

  void CSHMLayer::UpdateParameter(SReaderLayerPar& par_)
  {
    // read connection parameters for the shared memory layer
    // as a google protobuf message
    std::vector<std::string> memfile_names;

    eCAL::pb::ConnnectionPar connection_par;
    if (connection_par.ParseFromString(par_.parameter))
    {
      for (auto memfile_name : connection_par.ecal_shm_par().memory_file_names())
      {
        memfile_names.push_back(memfile_name);
      }
    }
    else
    {
      std::cout << "Could not parse layer parameter !" << std::endl;
      return;
    }

    for (auto memfile_name : memfile_names)
    {
      // start memory file receive thread if topic is subscribed in this process
      if (g_memfile_pool())
      {
        std::string process_id = std::to_string(Process::GetProcessID());
        std::string memfile_event = memfile_name + "_" + process_id;
        g_memfile_pool()->AssignThread(par_.topic_id, memfile_event, memfile_name, par_.topic_name);
      }
    }
  }
}

/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
 * @brief  Generate an ecal.yaml file with default configuration.
 *         Use --dump or -d to write the default configuration to file in current folder.
 *         Use with no arguments to create the eCAL directory in the first available of the following paths:
 *         1. ECAL_DATA environment variable path if set
 *         2. local user path
 *         3. system path like etc, ProgramData
**/

#include "config/configuration_writer.h"
#include "config/default_configuration.h"
#include "config/ecal_path_processing.h"

#include <iostream>

int main(int argc, char* argv[]) {
  bool dump = false;

  for (int i = 1; i < argc; i++)
  {
    const std::string arg = argv[i];
    if (arg == "--dump" || arg == "-d")
    {
      dump = true;
    }
    else
    {
      std::cerr << "Unknown argument: " << arg << "\n";
      return 1;
    }
  }
  
  if (dump)
  {
    if(!eCAL::Config::dumpConfigToFile())
    {
      std::cerr << "Failed to write default configuration to file." << "\n";
      return 1;
    }

    if(!eCAL::Config::dumpToFile(eCAL::Config::getTimeConfigAsYamlSS(), "time.yaml"))
    {
      std::cerr << "Failed to write time configuration to file." << "\n";
      return 1;
    }

    return 0;
  }
  
  const std::vector<std::string> dir_paths = 
  {
    eCAL::Config::eCALDataEnvPath(),
    eCAL::Config::eCALLocalUserDir(),
    eCAL::Config::eCALDataSystemDir() 
  };

  std::string created_path;

  // create the ecal paths
  for (const std::string& path : dir_paths)
  {
    if (!path.empty() && eCAL::Config::createEcalDirStructure(path))
    {
      created_path = path;
      std::cout << "Created eCAL directory structure in: " << path << "\n";
      break;
    }
  }

  // dump config to file
  if (!created_path.empty())
  {
    if (!eCAL::Config::dumpDefaultConfig(created_path))
    {
      std::cerr << "Failed to write default configuration to file." << "\n";
      return 1;
    }
    if(!eCAL::Config::dumpToFile(eCAL::Config::getTimeConfigAsYamlSS(), created_path + "/time.yaml"))
    {
      std::cerr << "Failed to write time configuration to file." << "\n";
      return 1;
    }

    std::cout << "Created eCAL configuration files in: " << created_path << "\n";

    return 0;
  } 
  else
  {
    std::cerr << "Failed to create eCAL directory structure." << "\n";
    return 1;
  }
}
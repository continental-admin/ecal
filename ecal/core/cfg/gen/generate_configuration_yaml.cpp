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

    return static_cast<int>(!eCAL::Config::dumpToFile(eCAL::Config::getTimeConfigAsYamlSS(), "time.yaml"));
  }
  
  const std::vector<std::string> dir_paths = 
  {
    eCAL::Config::eCALConfigEnvPath(),
    eCAL::Config::eCALLocalUserDir(),
    eCAL::Config::eCALDataSystemDir() 
  };

  bool created = false;
  std::string created_path;

  // create the ecal paths
  for (const std::string& path : dir_paths)
  {
    if (!path.empty() && eCAL::Config::createEcalDirStructure(path))
    {
      created = true;
      created_path = path;
      std::cout << "Created eCAL directory structure in: " << path << "\n";
      break;
    }
  }

  // dump config to file
  if (!created_path.empty())
  {
    const eCAL::Configuration default_config{};
    if (!eCAL::Config::dumpDefaultConfig(created_path))
    {
      std::cerr << "Failed to write default configuration to file." << "\n";
      return 1;
    }
    return static_cast<int>(!eCAL::Config::dumpToFile(eCAL::Config::getTimeConfigAsYamlSS(), created_path + "/time.yaml"));
  }

  if (!created)
  {
    std::cerr << "Failed to create eCAL directory structure." << "\n";
    return 1;
  }
}
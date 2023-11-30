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

#ifdef __unix__

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>

#include "../include/linux/mma_linux.h"

#define B_IN_KB 1024.0
#define B_IN_MB 1048576.0

#define MMA_CPU_CMD "cat /proc/uptime"
#define MMA_NET_CMD "cat /proc/net/dev"
#define MMA_DISK_CMD "cat /proc/diskstats"
#define MMA_PS_CMD "cat /proc/[0-9]*/stat 2>/dev/null"

MMALinux::MMALinux():
  cpu_pipe_(std::make_unique<PipeRefresher>(MMA_CPU_CMD,500)),
  network_pipe_(std::make_unique<PipeRefresher>(MMA_NET_CMD,500)),
  disk_pipe_(std::make_unique<PipeRefresher>(MMA_DISK_CMD,500)),
  process_pipe_(std::make_unique<PipeRefresher>(MMA_PS_CMD,500))
{
  cpu_pipe_->AddCallback(std::bind(&MMALinux::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2));
  network_pipe_->AddCallback(std::bind(&MMALinux::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2));
  disk_pipe_->AddCallback(std::bind(&MMALinux::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2));
  process_pipe_->AddCallback(std::bind(&MMALinux::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2));

  os_name = GetOsName();
  nr_of_cpu_cores = GetCpuCores();

}

MMALinux::~MMALinux()
{
}

void MMALinux::OnDataReceived(const std::string& pipe_result, const std::string& command)
{
  std::lock_guard<std::mutex> guard(mutex);
  if (command == MMA_CPU_CMD)
  {
    cpu_pipe_result_ = pipe_result;
    cpu_pipe_count_++;
  }
  else if (command == MMA_NET_CMD)
  {
    network_pipe_result_ = pipe_result;
    network_pipe_count_++;
  }
  else if (command == MMA_DISK_CMD)
  {
    disk_pipe_result_ = pipe_result;
    disk_pipe_count_++;
  }
  else if (command == MMA_PS_CMD)
  {
    process_pipe_result_ = pipe_result;
    process_pipe_count_++;
  }

}

void MMALinux::OpenPipe(const std::string& command, std::string& result)
{
  FILE* file = popen(command.data(), "r");
  char buffer[4096];

  if (file != nullptr)
  {
    while (fgets(buffer, sizeof(buffer), file) != nullptr)
      result += buffer;

    pclose(file);
  }
  strcpy(buffer, "");
}

std::string MMALinux::FileToString(const std::string& command)
{
  FILE* file = fopen(command.data(), "r");
  char buffer[4096];
  std::string result;

  if (file != nullptr)
  {
    while (fgets(buffer, sizeof(buffer), file) != nullptr)
      result += buffer;

    fclose(file);
  }

  return result;
}

double MMALinux::GetCPULoad()
{
  std::string local_copy;
  unsigned int local_count;
  static unsigned int previous_count;
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = cpu_pipe_result_;
    local_count = cpu_pipe_count_;
  }

  double cpu_time = 0.0;

  if (local_copy.length() > 0)
  {
    std::vector<std::string> cpu_line = SplitLine(local_copy);
    const double idle_time = stod(cpu_line.back());
    const unsigned int delta_count = (local_count - previous_count) * nr_of_cpu_cores;
    static double previous_idle_time = idle_time - delta_count / 2;
    const double current_idle = idle_time - previous_idle_time;

    if (delta_count > 0)
      cpu_time = 100.0 * (1 - 2 * current_idle / delta_count);
    previous_idle_time = idle_time;
    previous_count = local_count;
  }

  return cpu_time;
}

ResourceLinux::Memory MMALinux::GetMemory()
{
  // Ubuntu 12.04 and 14.04 provide different information than Ubuntu 16.04

  ResourceLinux::Memory memory;

  std::string memfile_str = FileToString("/proc/meminfo");
  std::unordered_map<std::string, uint64_t> memory_map;

  std::stringstream stream(memfile_str);

  for (int i = 0; i < 5; i++)
  {
    std::string title;
    uint64_t title_data;
    std::string size_type;
    stream >> title >> title_data >> size_type;
    memory_map[title] = title_data;
  }

  if (memory_map.find("MemAvailable:") != memory_map.end())
  { // since Linux kernel 3.14
    memory.available = memory_map["MemAvailable:"];
    memory.total = memory_map["MemTotal:"];
  }
  else
  { // before Linux kernel 3.14
    memory.available = memory_map["MemFree:"] + memory_map["Buffers:"] + memory_map["Cached:"];
    memory.total = memory_map["MemTotal:"];
  }

  memory.total *= B_IN_KB;
  memory.available *= B_IN_KB;

  return memory;
}

ResourceLinux::Temperature MMALinux::GetTemperature()
{
  ResourceLinux::Temperature temperature;

#ifdef __arm__
  // FixMe: this seems very specific for some dedicated devices
  std::string result;
  OpenPipe("/opt/vc/bin/vcgencmd measure_temp", result);

  std::smatch match;
  const std::regex expression("\\d{2,}.\\d{1,}");

  if (std::regex_search(result, match, expression) && match.size() > 0)
  {
    temperature.cpu_temperature = stof(match.str(0));
  }

#endif

  return temperature;
}

ResourceLinux::DiskStatsList MMALinux::GetDisks()
{
  ResourceLinux::DiskStatsList disk_stats_info;

  SetDiskInformation(disk_stats_info);

  SetDiskIOInformation(disk_stats_info);

  return disk_stats_info;
}

ResourceLinux::NetworkStatsList MMALinux::GetNetworks()
{
  std::string local_copy;
  unsigned int local_count;
  static unsigned int previous_count;
  typedef struct
  {
    unsigned long rec;
    unsigned long snd;
  } t_io;
  static std::unordered_map<std::string, t_io> previous;
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = network_pipe_result_;
    local_count = network_pipe_count_;
  }

  ResourceLinux::NetworkStatsList networks;
  ResourceLinux::Network network_stats;

  auto lines = TokenizeIntoLines(local_copy);
  if (lines.size() > 0)
  {
    const unsigned int delta_count = (local_count - previous_count);

    // skip the first 2 lines
    auto iterator = lines.begin();
    for(std::advance(iterator, 2); iterator != lines.end(); iterator++)
    {
      std::vector<std::string> network_io = SplitLine(*iterator);
      t_io cur = {0, 0};
      t_io prev;
      network_stats.name=network_io[0];
      // remove the colon at the end
      network_stats.name.pop_back();

      try { cur.rec = stoul(network_io[1]); }
      catch (...) { }

      try { cur.snd = stoul(network_io[9]); }
      catch (...) { }

      if (previous.find(network_stats.name) != previous.end())
      {
        prev = previous[network_stats.name];
      }
      else
      {
        prev = cur;
      }

      if (delta_count > 0)
      {
        network_stats.receive = (cur.rec - prev.rec) / (0.5 * delta_count);
        network_stats.send = (cur.snd - prev.snd) / (0.5 * delta_count);
      }
      else
      {
        network_stats.receive = 0;
        network_stats.send = 0;
      }

      networks.push_back(network_stats);
      previous[network_stats.name] = cur;
    }
    previous_count = local_count;
  }

  return networks;
}

ResourceLinux::ProcessStatsList MMALinux::GetProcesses()
{
  std::string local_copy;
  static const long page_size = sysconf(_SC_PAGE_SIZE);
  static const long ticks_per_second = sysconf(_SC_CLK_TCK);
  static unsigned int previous_count;
  typedef struct
  {
    unsigned long utime;
    unsigned int count;
    ResourceLinux::Process process_stats;
  } t_item;
  static std::map<uint32_t, t_item> previous;
  t_item item;
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = process_pipe_result_;
    item.count = process_pipe_count_;
  }

  ResourceLinux::ProcessStatsList processes;
  ResourceLinux::Process process_stats;

  auto lines = TokenizeIntoLines(local_copy);
  if (lines.size() > 0)
  {
    for(auto iterator = lines.begin(); iterator!=lines.end(); iterator++)
    {
      std::vector<std::string> process_data = SplitLine(*iterator);
      process_stats.id=std::stoi(process_data[0]);
      // process utime
      const unsigned long cur = std::stoul(process_data[13]);
      unsigned long prev;
      if (previous.find(process_stats.id) != previous.end())
      {
        prev = previous[process_stats.id].utime;
        process_stats = previous[process_stats.id].process_stats;
      }
      else
      {
        prev = cur;
        struct stat info;
        std::string filename = "/proc/" + process_data[0];
        if (0 == stat(filename.c_str(), &info))
        {
          static std::map<uid_t, std::string> uname_map;
          if (uname_map.find(info.st_uid) == uname_map.end())
          {
            struct passwd *pw = getpwuid(info.st_uid);
            if (pw)
            {
              process_stats.user=pw->pw_name;
              uname_map[info.st_uid] = pw->pw_name;
            }
            else
            {
              process_stats.user = "";
            }
          }
          else
          {
            process_stats.user = uname_map[info.st_uid];
          }
        }
      }

      process_stats.memory.current_used_memory=stoull(process_data[23]) * page_size;
      if (process_data[1].length() > 2)
        process_stats.commandline=process_data[1].substr(1, process_data[1].length()-2);
      else
        process_stats.commandline = process_data[1];

      const unsigned int delta_count = (item.count - previous_count) * ticks_per_second * nr_of_cpu_cores;

      if (delta_count > 0)
        process_stats.cpu_load.cpu_load = (cur - prev) / (0.005 * delta_count);
      item.utime = cur;
      item.process_stats = process_stats;
      previous[process_stats.id] = item;
    }
    previous_count = item.count;

    // garbage collection
    for (auto it = previous.begin(), next = it; it != previous.end(); it = next)
    {
      next++;
      if (it->second.count < item.count)
      { // process disappeared
        previous.erase(it);
      }
      else
      { // push to sorted list
        processes.push_back(it->second.process_stats);
      }
    }
  }
  return processes;
}

bool MMALinux::Get(eCAL::pb::mma::State& state)
{
  state.clear_disks();
  state.clear_networks();
  state.clear_process();
  SetResourceData(state);
  return true;
}

std::vector<std::string> MMALinux::SplitLine(const std::string& input)
{
  std::istringstream buffer(input);
  std::vector<std::string> ret((std::istream_iterator<std::string>(buffer)),
    std::istream_iterator<std::string>());
  return ret;
}

bool MMALinux::IsDigit(const char c)
{
  return ('0' <= c && c <= '9');
}

bool MMALinux::FormatListData(std::vector<std::string>& the_list)
{
  bool return_value = true;

  if (!the_list.empty())
  {
    // delete the unnecessary columns from the pipe result
    auto iterator = the_list.begin();
    std::advance(iterator, 1);
    the_list.erase(iterator);

    iterator = the_list.begin();
    std::advance(iterator, 3);
    the_list.erase(iterator);

    iterator = the_list.begin();
    std::string name = *iterator;

    // /dev/root needs special handling
    if (name == "/dev/root")
    {
      static std::string root_dev;
      if (root_dev.empty())
      {
        std::string result;
        OpenPipe("cat /proc/cmdline", result);
        std::size_t found = result.find("root=");
        result = result.substr(found + 5);
        found = result.find(" ");
        root_dev = result.substr(0, found);
      }
      name = root_dev;
    }

    // if last character of string is digit, delete the digit
    // because it is the partition number
    if (IsDigit(name.back()))
    {
      // exception: mtdblock
      if (name.find("mtdblock") == std::string::npos)
      {
        name.pop_back();
        // delete last character p as well
        if (name.back() == 'p')
          name.pop_back();
      }
    }

    *iterator = name;

    // format the path to include just the name
    std::size_t found = name.find_last_of("/");
    // only accept devices which are indicated by the / in name
    if (found == std::string::npos)
      return_value = false;
    *iterator = name.substr(found + 1);
  }
  else
  {
    return_value = false;
  }

  return return_value;
}

std::list<std::string> MMALinux::TokenizeIntoLines(const std::string& str)
{
  std::list<std::string> result;

  std::stringstream ss(str);
  std::string item;

  while (std::getline(ss, item))
  {
    if (item.empty() == false)
      result.push_back(std::move(item));
  }

  return result;
}

bool MMALinux::SetDiskInformation(ResourceLinux::DiskStatsList& disks)
{
  bool return_value = true;

  std::string result;
  OpenPipe("df", result);

  // disks sorted in map
  std::map<std::string, ResourceLinux::DiskStats> disks_map;

  auto lines = TokenizeIntoLines(result);

  auto it = lines.begin();
  std::advance(it, 1);

  lines.erase(lines.begin(), it);

  for (const auto& line : lines)
  {
    std::vector<std::string> partition = SplitLine(line);
    if (FormatListData(partition))
    {
      if (partition.size() == 4)
      {
        const uint64_t KiB = 1024;
        ResourceLinux::DiskStats disk_stats;
        uint64_t used = stoll(partition[1]) * KiB;
        disk_stats.name = partition[0];
        disk_stats.available = stoll(partition[2]) * KiB;
        disk_stats.capacity = disk_stats.available + used;
        disk_stats.mount_point = partition[3];
        // ensure initialization of transfer rates
        disk_stats.read = 0.0;
        disk_stats.write = 0.0;
        if (disks_map.find(partition[0]) == disks_map.end())
        {
          disks_map[partition[0]] = disk_stats;
        }
        else
        {
          // sum up partitions of same disk
          disks_map[partition[0]].available += disk_stats.available;
          disks_map[partition[0]].capacity += disk_stats.capacity;
        }
      }
    }
  }

  for (const auto& disk : disks_map)
  {
    disks.push_back(disk.second);
  }

  return return_value;
}

bool MMALinux::SetDiskIOInformation(ResourceLinux::DiskStatsList& disk_stats_info)
{
  std::string local_copy;
  unsigned int local_count;
  static unsigned int previous_count;
  typedef struct
  {
    unsigned long read;
    unsigned long write;
  } t_io;
  static std::unordered_map<std::string, t_io> previous;
  {
    std::lock_guard<std::mutex> guard(mutex);
    local_copy = disk_pipe_result_;
    local_count = disk_pipe_count_;
  }

  bool return_value = true;
  const unsigned int delta_count = (local_count - previous_count);

  auto lines = TokenizeIntoLines(local_copy);

  for (const auto& line : lines)
  {
    std::vector<std::string> partition = SplitLine(line);

    for (auto& disk : disk_stats_info)
    {
      if (disk.name.find(partition[2]) != std::string::npos)
      {
        t_io cur = {0, 0};
        t_io prev;

        try { cur.read = static_cast<double>(stod(partition[5])); }
        catch (...) { }

        try { cur.write = static_cast<double>(stod(partition[9])); }
        catch (...) { }

        if (previous.find(disk.name) != previous.end())
        {
          prev = previous[disk.name];
        }
        else
        {
          prev = cur;
        }

        if (delta_count > 0)
        { // the factor 4 is a 2 * 2 because:
          // 2 reads every second and
          // value is in sectors of 512 bytes
          disk.read = (cur.read - prev.read) * 4 * B_IN_KB / delta_count;
          disk.write = (cur.write - prev.write) * 4 * B_IN_KB / delta_count;
        }

        previous[disk.name] = cur;
      }
    }
  }
  previous_count = local_count;

  return return_value;
}

int MMALinux::GetCpuCores(void)
{
  return get_nprocs();
}

std::string MMALinux::GetOsName()
{
  std::string os_release = FileToString("/etc/os-release");
  std::size_t found = os_release.find("PRETTY_NAME=\"");
  os_release = os_release.substr(found + 13);
  found = os_release.find("\"");
  std::string name = "LINUX ";
  name.append(os_release.substr(0, found));
  return name;
}

void MMALinux::SetResourceData(eCAL::pb::mma::State& state)
{
  state.set_cpu_load(GetCPULoad());
  state.set_number_of_cpu_cores(nr_of_cpu_cores);
  state.set_operating_system(os_name);

  eCAL::pb::mma::Memory memory;
  ResourceLinux::Memory mem = GetMemory();
  memory.set_total(mem.total);
  memory.set_available(mem.available);
  state.mutable_memory()->CopyFrom(memory);

  ResourceLinux::ProcessStatsList processes=GetProcesses();
  for (const auto& process : processes)
  {
    eCAL::pb::mma::Process* new_process = state.add_process();
    new_process->set_id(process.id);
    new_process->set_commandline(process.commandline);
    new_process->set_user(process.user);

    eCAL::pb::mma::ProcessesMemory proc_memory;
    ResourceLinux::ProcessesMemory proc_mem;
    proc_mem.current_used_memory=process.memory.current_used_memory;
    proc_memory.set_current_working_set_size(proc_mem.current_used_memory);
    new_process->mutable_memory()->CopyFrom(proc_memory);

    eCAL::pb::mma::ProcessesCPU proc_cpu_load;
    ResourceLinux::ProcessesCPU cpu_load;
    cpu_load.cpu_load=process.cpu_load.cpu_load;
    proc_cpu_load.set_cpu_load(cpu_load.cpu_load);
    new_process->mutable_cpu()->CopyFrom(proc_cpu_load);
  }


  ResourceLinux::DiskStatsList disks = GetDisks();

  for (const auto& disk : disks)
  {
    eCAL::pb::mma::Disk* disk_logical = state.add_disks();
    disk_logical->set_name(disk.name);
    disk_logical->set_capacity(disk.capacity);
    disk_logical->set_available(disk.available);
    disk_logical->set_read(disk.read);
    disk_logical->set_write(disk.write);
  }

  ResourceLinux::NetworkStatsList networks = GetNetworks();
  for (const auto& network : networks)
  {
    eCAL::pb::mma::Network* nic = state.add_networks();
    nic->set_name(network.name);
    nic->set_send(network.send);
    nic->set_receive(network.receive);
  }

  ResourceLinux::Temperature temp = GetTemperature();

  if (temp.IsCPUTempValid() == true)
  {
    eCAL::pb::mma::Temperature temperature;
    temperature.set_cpu(temp.cpu_temperature);

    state.mutable_temperature()->CopyFrom(temperature);
  }
  else
  {
    state.clear_temperature();
  }
}
#endif  // __unix__

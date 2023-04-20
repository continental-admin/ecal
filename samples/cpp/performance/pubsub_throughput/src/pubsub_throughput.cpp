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

#include <ecal/ecal.h>

#include <atomic>
#include <chrono>
#include <iostream>

#include "binary_payload_writer.h"

const auto g_snd_size (8* 1024 * 1024);
const auto g_snd_loops(1000);

// subscriber callback function
std::atomic<size_t> g_callback_received;
void OnReceive(const char* /*topic_name_*/, const struct eCAL::SReceiveCallbackData* data_)
{
  g_callback_received += data_->size;
}

void throughput_test(int snd_size, int snd_loops, eCAL::TLayer::eTransportLayer layer, bool zero_copy = false)
{
  // create payload
  CBinaryPayload payload(snd_size);

  // create publisher
  eCAL::CPublisher pub("throughput");
  // set transport layer
  pub.SetLayerMode(eCAL::TLayer::tlayer_all, eCAL::TLayer::smode_off);
  pub.SetLayerMode(layer, eCAL::TLayer::smode_on);
  // set attributes
  pub.ShmEnableZeroCopy(zero_copy);
  pub.ShmSetAcknowledgeTimeout(100);

  // create subscriber
  eCAL::CSubscriber sub("throughput");
  // add callback
  sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2));

  // let's match them
  eCAL::Process::SleepMS(2000);

  // initial call to allocate memory file
  pub.Send(payload);

  // reset received bytes counter
  g_callback_received = 0;

  // start time
  auto start = std::chrono::high_resolution_clock::now();

  // do some work
  for (auto i = 0; i < snd_loops; ++i)
  {
    pub.Send(payload);
  }

  // end time
  auto finish = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double> elapsed = finish - start;
  std::cout << "Elapsed time : " << elapsed.count() << " s" << std::endl;

  const size_t sum_snd_bytes = payload.GetSize() * snd_loops;
  const size_t sum_rcv_bytes = g_callback_received;
  std::cout << "Sent         : " << sum_snd_bytes / (1024 * 1024) << " MB" << std::endl;
  std::cout << "Received     : " << sum_rcv_bytes / (1024 * 1024) << " MB" << std::endl;
  std::cout << "Lost         : " << sum_snd_bytes - sum_rcv_bytes << " bytes";
  std::cout << " (" << (sum_snd_bytes - sum_rcv_bytes) / (1024 * 1024) << " MB, ";
  std::cout << (sum_snd_bytes - sum_rcv_bytes) * 100.0f / sum_snd_bytes << " %)" << std::endl;
  std::cout << "Throughput   : " << int((sum_snd_bytes / (1024.0 * 1024.0)) / elapsed.count()) << " MB/s " << std::endl;
  std::cout << "Throughput   : " << int((sum_snd_bytes / (1024.0 * 1024.0 * 1024.0)) / elapsed.count()) << " GB/s " << std::endl;
}

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "pubsub_throughput");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  std::cout << "---------------------------" << std::endl;
  std::cout << "LAYER: SHM"                  << std::endl;
  std::cout << "---------------------------" << std::endl;
  throughput_test(g_snd_size, g_snd_loops, eCAL::TLayer::tlayer_shm, false);
  std::cout << std::endl << std::endl;

  std::cout << "---------------------------" << std::endl;
  std::cout << "LAYER: SHM ZERO-COPY"        << std::endl;
  std::cout << "---------------------------" << std::endl;
  throughput_test(g_snd_size, g_snd_loops, eCAL::TLayer::tlayer_shm, true);
  std::cout << std::endl << std::endl;

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}

#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

#include <cstdint>
#include <cstdio>

#include "signal.h"
#include "defs.h"

#define WRITE_INTERVAL 10

int main(int argc, char *argv[])
{
  std::ofstream queue;
  queue.open(DEVICE_URI, std::ios::binary | std::ios::out);

  if (!queue.is_open()) {
     std::cerr << "Writer: Unable to open device: " << DEVICE_URI << std::endl;
     return -1;
  }
  std::cout << "Writer: '" << DEVICE_URI << "' queue opened!" << std::endl;

  signal_t s;

  uint64_t cnt = 0;
  while (true) {
     s.id = cnt++;
     s.timestamp = cnt * 10;

     std::snprintf(s.data, sizeof(s.data), "%lu", cnt);

     queue.write((char*)&s, sizeof(s));
     queue << std::flush;
     std::this_thread::sleep_for(std::chrono::milliseconds(WRITE_INTERVAL));
  }

  return 0;
}

#include <fstream>
#include <iostream>
#include <cstdint>

#include "signal.h"
#include "defs.h"

void print_signal(const signal_t* s)
{
   std::cout << "id: " << s->id << ", timestamp: " << s->timestamp
     << ", data: '" << s->data << "'\n";
}


int main(int argc, char *argv[])
{
   std::ifstream queue;

   queue.open(DEVICE_URI, std::ios::binary | std::ios::in);

   if (!queue.is_open()) {
      std::cerr << "Reader: Unable to open device: " << DEVICE_URI << std::endl;
      return -1;
   }
   std::cout << "Reader: '" << DEVICE_URI << "' queue opened!" << std::endl;

   signal_t signal;

   while(!queue.eof())
   {
     queue.read((char*)&signal, sizeof(signal_t));
     print_signal(&signal);
   }

   queue.close();

   return 0;
}

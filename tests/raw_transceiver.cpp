#include "../include/radiocom.h"

using namespace std;
using namespace rfcom;

void* input_processor(void* arg)
{
  Transceiver* t_ptr = static_cast<Transceiver*>(arg);

  string input;
  while(getline(cin, input))
    {
      if(input == "exit")
	break;
      
      t_ptr->sendRaw((byte1_t*)input.c_str(), input.size());
    }
  pthread_exit(NULL);
}

int main(int argc, char** argv)
{
  if(argc != 2)
    throw runtime_error("A serial port name is needed.");

  Transceiver t;
  int initstatus = t.initPort(argv[1], B38400, "raw_log", "packet_log");
  if(initstatus == -1)
    throw runtime_error("Error opening serial port " + string(argv[1]));
  if(initstatus == -2)
    throw runtime_error("Error opening raw_log");
  if(initstatus == -3)
    throw runtime_error("Error opening packet_log");
  
  t.startReceiving();

  pthread_t input_thread_t;
  pthread_create(&input_thread_t, NULL, input_processor, &t);

  pthread_join(input_thread_t, NULL);
  
  t.stopReceiving();
  t.termPort();
  
  return 0;
}

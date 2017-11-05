#include "../include/radiocom.h"

using namespace std;
using namespace rfcom;

volatile bool end_flag = false;

void* exit_detector(void* arg)
{
  string input;
  while(getline(cin, input))
    {
      if(input == "exit")
	break;
    }

  end_flag = true;
  pthread_exit(NULL);
}


int main(int argc, char** argv)
{
  if(argc != 2)
    throw runtime_error("A serial port name is needed.");
  
  Transceiver t;

  if(t.initPort(argv[1]) < 0)
    throw runtime_error("Error opening serial port " + string(argv[1]));
  
  t.startListener();

  pthread_t input_thread_t;
  pthread_create(&input_thread_t, NULL, exit_detector, NULL);
  
  string message = "123456789012345678901234567890123456789012345678901234567890";
  while(!end_flag)
    {
      usleep(10000);
      t.sendRaw((byte1_t*)message.c_str(), message.size());
    }
  

  pthread_join(input_thread_t, NULL);
  
  t.stopListener();
  t.termPort();
  
  return 0;
}

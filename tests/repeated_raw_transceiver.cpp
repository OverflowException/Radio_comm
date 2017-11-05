#include "../include/radiocom.h"

using namespace std;
using namespace rfcom;

volatile bool detector_end_flag = false;
volatile bool repeater_end_flag = false;
volatile bool extract_flag = false;

void* keyboard_detector(void* arg)
{
  string input;
  while(getline(cin, input))
    {
      if(input == "exit")
	break;
      else
	extract_flag = true;
    }

  detector_end_flag = true;
  pthread_exit(NULL);
}

void* repeater(void* arg)
{
  Transceiver* t_ptr = static_cast<Transceiver*>(arg);
  string message = "123456789012345678901234567890123456789012345678901234567890";
  while(!repeater_end_flag)
    {
      usleep(500000);
      t_ptr->sendRaw((byte1_t*)message.c_str(), message.size());
    }

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

  //Create input monitor thread
  pthread_t input_thread_t;
  pthread_create(&input_thread_t, NULL, keyboard_detector, NULL);

  //Create repeater thread
  pthread_t repeater_thread_t;
  pthread_create(&repeater_thread_t, NULL, repeater, &t);

  
  Packet p;
  while(!detector_end_flag)
    {
      if(extract_flag == true)
	{
	  if(t.extractNext(p));
	  packetOut(p, cout);
	  cout << endl;
	  extract_flag = false;
	}
      usleep(10000); //Control CPU load
    }

  repeater_end_flag = true;
  pthread_join(input_thread_t, NULL);

  t.stopListener();
  t.termPort();
  
  return 0;
}

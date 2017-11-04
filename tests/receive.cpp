#include "../include/radiocom.h"
#include <string>

using namespace std;
using namespace rfcom;


void* keyboard_monitor(void* args)
{
  Transceiver* t_ptr = static_cast<Transceiver*>(args);
  string input;
  while(getline(cin, input))
    {
      if(input == "exit")
	break;

      
    }

  pthread_exit(NULL);
}

int main(int argc, char** argv)
{
  Transceiver t;
  //Open up ttyS64, which is connected to ttyS65.
  if(t.initPort("/dev/ttyS64") < 0)
    throw runtime_error("Cannot init ttyS64");

  t.startListener();
  
  string input;
  while(cin >> input)
    {
      if(input == "exit")
	break;

      
    }
  
  t.stopListener();
  t.termPort();  
}

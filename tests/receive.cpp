#include "../src/radiocom.h"
#include <string>

using namespace std;
using namespace rfcom;

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

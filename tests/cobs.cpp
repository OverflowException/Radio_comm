#include <iostream>
#include <iterator>
#include <algorithm>

#include "../src/radiocom.h"

using namespace std;
using namespace rfcom;

int main(int argc, char** argv)
{
  //To test COBS.
  cout << hex;
  ostream_iterator<int> oi(cout, "\t");
  uint8_t
    cobs_test[8] = {0xff, 0x00, 0xac, 0xff, 0x00, 0xaa, 0x00, 0xff};
  copy(cobs_test, cobs_test + sizeof(cobs_test), oi);
  cout << endl;
  
  Protocol::cobsEncode(cobs_test, sizeof(cobs_test), 0x00);
  copy(cobs_test, cobs_test + sizeof(cobs_test), oi);
  cout << endl;

  cout.setf(ios::boolalpha);
  cout << Protocol::cobsDecode(cobs_test, sizeof(cobs_test), 0x00) << endl;
  copy(cobs_test, cobs_test + sizeof(cobs_test), oi);
  cout << endl;
  
}

#include "../src/radiocom.h"

using namespace std;
using namespace rfcom;

int main(int argc, char** argv)
{
  Transmitter t;
  byte1_t data[16] = {0x10, 0x3c, 0x44, 0x56};
  //std::cout << "here" << std::endl;
  t.packPush(0x00, 0xbdaf, data);
  //std::cout << "here" << std::endl;
  t.send(cout);
}

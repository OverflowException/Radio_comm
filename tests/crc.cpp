#include <iostream>
#include <ctime>
#include <cstdio>
#include "../include/radiocom.h"

using namespace std;
using namespace rfcom;

int main(int argc, char** argv)
{
  //To test CRC encode;
  size_t length = 19;
  byte1_t* crc_test = (byte1_t*)malloc(length * sizeof(byte1_t));
  srand((unsigned)time(NULL));
  for(size_t count = 0; count < length; ++count)
    {
      crc_test[count] = (byte1_t)rand() % 0x100;
      //hex can only work with int type. Can not work with char type.
      cout << hex << (int)crc_test[count] << " ";
    }
    cout << endl;

  
  cout << hex << Protocol::crc16Gen(crc_test, length, CRC16_GEN_BUYPASS) << endl;
}

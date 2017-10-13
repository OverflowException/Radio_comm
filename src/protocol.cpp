#include "protocol.h"
#include <iostream>

namespace rfcom
{
  byte2_t Protocol::crc16Gen(const byte1_t* pos, size_t len, byte2_t generator)
  {
    if(pos == NULL)
      return 0;
    
    byte2_t checksum = 0x0000;
    int bit_count;
    
    while(len--)
      {
	checksum ^= *pos++;
	for(bit_count = 0; bit_count < 8; ++bit_count)
	  checksum = checksum & 0x8000 ? (checksum << 1) ^ generator : checksum << 1;
      }

    for(bit_count = 0; bit_count < 8; ++bit_count)
      checksum = checksum & 0x8000 ? (checksum << 1) ^ generator : checksum << 1;
    
    return checksum;
  }

  byte1_t* Protocol::cobsEncode(byte1_t* pos, size_t len, byte1_t delim)
  {
    if(pos == NULL || len < 2)
      return pos;
    
    *(pos + len - 1) = 0x00;
    size_t jump = 0;
    
    for(size_t offset = --len; offset > 0; --offset, ++jump)
      if(pos[offset] == delim)
	{
	  pos[offset] = jump;
	  jump = 0;
	}
    
    *pos = jump;
    return pos;
  }

  bool Protocol::cobsDecode(byte1_t* pos, size_t len, byte1_t delim)
  {
    if(pos == NULL || len < 2)
      return false;
    
    byte1_t* next_pos = pos + *pos;
    size_t offset = 0;
    byte1_t* off_end_pos = pos + len;
    while(*next_pos != 0 && next_pos < off_end_pos)
      {
	offset = *next_pos;
	*next_pos = delim;
	next_pos += offset;
      }

    if(next_pos != off_end_pos - 1)
      return false;

    return true;
  }  
}

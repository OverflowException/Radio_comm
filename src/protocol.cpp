#include "protocol.h"
#include <iostream>

namespace rfcom
{
  byte2_t Protocol::crc16Gen(const byte1_t* pos, size_t len, byte2_t generator)
  {
    if(pos == NULL)
      return 0;
    
    byte2_t checksum = 0x0000;
    int bits_read = 0;
    bool highest_bit;
    
    while(len > 0)
      {
	highest_bit = (bool)(checksum & 0x8000);
	checksum <<= 1;
	checksum |= ((*pos << bits_read) & 0x80) >> 7;
	
	++bits_read;
	if(bits_read > 7)
	  {
	    bits_read = 0;
	    --len;
	    ++pos;
	  }

	if(highest_bit)
	  checksum ^= generator;
      }

    //The last 2 bytes
    for(bits_read = 0; bits_read < 16; ++bits_read)
      {
	highest_bit = (bool)(checksum & 0x8000);
	checksum <<= 1;
	if(highest_bit)
	  checksum ^= generator;
      }
    
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

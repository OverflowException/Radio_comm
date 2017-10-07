#include "protocol.h"

namespace rfcom
{
  template<class _Tc>
  byte1_t* Protocol::crcGen(const byte1_t* pos, byte1_t* checksum,  _Tc generator, size_t len)
  {
  
  }

  byte1_t* Protocol::cobsEncode(byte1_t* pos, size_t len, byte1_t delim)
  {
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

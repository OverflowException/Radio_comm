#ifndef _PACKET
#define _PACKET

#include <stdint.h>

namespace rfcom
{  
  typedef uint8_t byte1_t;
  typedef uint16_t byte2_t;

  //Make sure this structure is tightly arranged in memory
#pragma pack(push, 1)
  struct Packet
  {
    byte1_t sync;
    byte1_t ohb;
    byte1_t ID;
    byte2_t index;
    byte1_t data[16];
    byte2_t checksum;
    byte1_t eop;
  };
#pragma pack(pop)
}

#endif

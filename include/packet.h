#ifndef _PACKET
#define _PACKET

#include <cstdint>
#include <iostream>
#include <iterator>
#include <algorithm>

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

  
  /**
     Determine the length of actual data in a packet with id
     @params
     id: packet ID
     @return 
     Length of actual data in bytes. Return 0 if id invalid.
  */
  static size_t lengthByID(byte1_t id)
  {
    //Command
    if(id == 0xc0)
      return 1;

    return 0;
  }

  /**
   */
  static void packetOut(const Packet& p, std::ostream& os, std::string delim = " ")
  {
    os << std::hex;
    auto os_it = std::ostream_iterator<int>(os, delim.c_str());
    std::copy((byte1_t*)&p, (byte1_t*)&p + sizeof(Packet), os_it);
  }
}

#endif

#ifndef _PACKET
#define _PACKET

#include <cstdint>

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
    //Status or message
    if(id & 0xc0)
      return 16;
    //Measured data
    else
      {
	switch(id & 0x3f)
	  {
	    //acc/gyr from either IMUs
	  case 0x00:
	  case 0x10:
	    return 12;

	    //mag/time from IMU_1
	  case 0x01:
	    return 10;
	    //mag/imp/time from IMU_2
	  case 0x11:
	    return 12;
	    //Invalid id
	  default:
	    return 0;
	  }
      }
  }
}

#endif

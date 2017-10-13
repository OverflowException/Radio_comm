#ifndef _PROTOCOL
#define _PROTOCOL

#include <cstdio>
#include "packet.h"

#define CRC16_GEN_BUYPASS (0x8005)
#define CRC16_GEN_XMODEM (0x1021)
#define CRC16_GEN_DECTX (0x0589)
#define CRC16_GEN_T10DIFF (0x8BB7)

namespace rfcom
{
  
  //_Tc --- data type of the CRC generator.
  //CRC generator must not contain leading zeros.
  class Protocol
  {
    
  public:
    /**
      Generate CRC16 check sequence. Checksum initial value 0x0000. Non-reflected output and input.
      @params
      pos: starting position of buffer.
      len: total length of buffer. Length should be at least 1 bytes.
      generator: CRC16 generator. Ignore the coefficient 1 of power 16.
      @return
      CRC checksum.
     */
    static byte2_t crc16Gen(const byte1_t* pos, size_t len, byte2_t generator);
    
    /**
      COBS encode. Unencoded data should be placed in range [p+1, p+len-1). *(p+len-1) should be 0.
      @params
      pos: Starting position of buffer being encoded. This buffer should start from ohb, end at eop.
      len: total length of buffer, in bytes. Length should be at least 2 bytes.
      delim: delimiter. The character to be replaced.
      @return
      pos
     */
    static byte1_t* cobsEncode(byte1_t* pos, size_t len, byte1_t delim);

    /**
       COBS decode. *(p+len-1) should be 0.
       @params
       pos: Starting position of buffer being decoded. This buffer should start from ohb, end at eop.
       len: total length of buffer, in bytes. Length should be at least 2 bytes.
       delim: delimiter. The character to be repalced.
       @return
       boolean, indicates the buffer can be correctly decoded.
     */
    static bool cobsDecode(byte1_t* pos, size_t len, byte1_t delim);
  };
}

#endif

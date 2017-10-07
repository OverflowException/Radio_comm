#ifndef _PROTOCOL
#define _PROTOCOL

#include <cstdio>
#include "packet.h"

namespace rfcom
{
  //_Tc --- data type of the CRC generator.
  //CRC generator must not contain leading zeros.
  class Protocol
  {
  public:
    /**
      Generate
      @params
      @return
     */
    template<class _Tc>
    static byte1_t* crcGen(const byte1_t* pos, byte1_t* checksum,  _Tc generator, size_t len);
    template<class _Tc>
    static bool crcCheck(const byte1_t* pos, _Tc generator, size_t len);
    
    /**
      COBS encode. Unencoded data should be placed in range [p+1, p+len-1). *(p+len-1) should be 0.
      @params
      pos: Starting position of buffer being encoded.
      len: total length of buffer, in bytes. Length should be at least 2 bytes.
      delim: delimiter. The character to be replaced.
      @return
      pos
     */
    static byte1_t* cobsEncode(byte1_t* pos, size_t len, byte1_t delim);

    /**
       COBS decode. *(p+len-1) should be 0.
       @params
       pos: Starting position of buffer being decoded.
       len: total length of buffer, in bytes. Length should be at least 2 bytes.
       delim: delimiter. The character to be repalced.
       @return
       bool value, indicates the buffer can be correctly decoded.
     */
    static bool cobsDecode(byte1_t* pos, size_t len, byte1_t delim);
  };
}

#endif

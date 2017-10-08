#ifndef _RADIOCOM
#define _RADIOCOM

#include "protocol.h"

namespace rfcom
{
  class ComModule
  {
  protected:
    //0x1021 --- CRC16/XMODEN
    //0x8005 --- CRC16/BUYPASS
    byte2_t _crc_gen;
  };

  class Receiver : public ComModule
  {
  private:
  };

  class Transmitter : public ComModule
  {
  private:
  };
}
#endif

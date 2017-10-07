#ifndef _RADIOCOM
#define _RADIOCOM

#include "protocol.h"

namespace rfcom
{
  class ComModule
  {
  protected:
    unsigned _crc_gen : 17;
  
  };

  class Receiver : public ComModule
  {
    
  };

  class Transmitter : public ComModule
  {
    
  };
}
#endif

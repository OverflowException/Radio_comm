#ifndef _RADIOCOM
#define _RADIOCOM

#include "protocol.h"
#include <queue>

namespace rfcom
{
  /*Radio communication module base class*/
  class ComModule
  {
  protected:
    byte2_t _crc_gen;  //CRC16 generator polynomial
    std::queue<Packet*> _pdu_queue;  //A queue of protocal data units
    size_t _max_queue_size;  //Maximun queue length
    //Discarding policy?
  public:
    ComModule(byte2_t gen, size_t m_size) { _crc_gen = gen; }
    void setCRCGen(byte2_t gen) { _crc_gen = gen; }
    byte2_t getCRCGen() const { return _crc_gen; }
    void setMaxQueueSize(size_t size){ _max_queue_size = size; }
    size_t getMaxQueueSize() const { return _max_queue_size; }
  };

  /*Receiver, derived from ComModule*/
  class Receiver : public ComModule
  {
  public:
    Receiver(byte2_t gen = CRC16_GEN_BUYPASS, size_t m_size = 1024) : ComModule(gen, m_size){}
    //bool popPacket();
  private:

    
  };

  /*Transmitter, derived from ComModule*/
  class Transmitter : public ComModule
  {
  public:
    Transmitter(byte2_t gen = CRC16_GEN_BUYPASS, size_t m_size = 1024) : ComModule(gen, m_size){}
    //bool pushPacket();
  private:
  };
}
#endif

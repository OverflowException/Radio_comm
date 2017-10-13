#ifndef _RADIOCOM
#define _RADIOCOM

#include "protocol.h"
#include <queue>
#include <iostream>
#include <iterator>
#include <algorithm>

namespace rfcom
{
  /*Radio communication module base class*/
  class ComModule
  {
  protected:
    byte2_t _crc_gen;  //CRC16 generator polynomial
    std::queue<Packet*> _pdu_queue;  //A queue of protocal data units
    
  public:
    ComModule(byte2_t gen) { _crc_gen = gen; }
    virtual ~ComModule(){ while(!_pdu_queue.empty()){delete _pdu_queue.front(); _pdu_queue.pop();} }
    void setCRCGen(byte2_t gen) { _crc_gen = gen; }
    byte2_t getCRCGen() const { return _crc_gen; }
  };

  /*Receiver, derived from ComModule*/
  class Receiver : public ComModule
  {
  public:
    Receiver(byte2_t gen = CRC16_GEN_BUYPASS) : ComModule(gen){}
    ~Receiver() = default;
    /**
       Try to pop the next packet from queue and unpack it.
       @params
       id: the reference of ID.
       index: the refernece packet index.
       p_data: starting position of data buffer. 
       len: the actual length of buffer. Depends on ID
       @return
       0: Success. Delete the packet from queue.
       -1: No more packets in PDU queue.
       -2: COBS decode failure. Keep packet in queue.
       -3: CRC mismatch. Keep packet in queue
     */
    int tryPopUnpack(byte1_t& id, byte2_t& index, byte1_t* p_data);
    
    /**
       Retrieve and pop next packet in PDU queue.
       @params
       p: Retrieved packet
       @return
       boolean, indicates retrieved or not.
     */
    bool retrieveNext(Packet& p);
  private:
    
  };

  /*Transmitter, derived from ComModule*/
  class Transmitter : public ComModule
  {
  public:
    Transmitter(byte2_t gen = CRC16_GEN_BUYPASS) : ComModule(gen){}
    ~Transmitter() = default;
    /**
       Try to pack up and push the packet into the queue.
       @params
       id
       index
       p_data:starting position of data buffer.
       @return
       0: Success.
       -1: invalid id.
     */
    int packPush(byte1_t id, byte2_t index, const byte1_t* p_data);

    /**
       A mock. will only send a packet to a ostream, in hex.
     */
    void send(std::ostream& os)
    {
      if(_pdu_queue.empty())
	{
	  os << "PDU queue empty" << std::endl;
	  return;
	}
      os << std::hex;
      std::ostream_iterator<int> os_it(os, " ");
      std::copy((byte1_t*)(_pdu_queue.front()), (byte1_t*)(_pdu_queue.front()) + 24, os_it);
      
      delete _pdu_queue.front();
      _pdu_queue.pop();
    }
  private:
  };
}
#endif

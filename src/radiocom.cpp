#include <cstring>
#include "radiocom.h"

namespace rfcom
{

  int Receiver::tryPopUnpack(byte1_t& id, byte2_t& index, byte1_t* p_data)
  {
    //PDU queue empty
    if(_pdu_queue.empty())
      return -1;

    Packet p;
    memcpy(&p, _pdu_queue.front(), sizeof(p));

    //COBS decode failure
    if(!Protocol::cobsDecode(&(p.ohb), 23, p.sync))
      return -2;

    //CRC mismatch
    if(Protocol::crc16Gen(&(p.ID), 19, CRC16_GEN_BUYPASS) != p.checksum)
      return -3;

    id = p.ID;
    index = p.index;
    memcpy(p_data, p.data, sizeof(p.data));
  
    delete _pdu_queue.front();
    _pdu_queue.pop();
  
    return 0;
  }

  int Transmitter::packPush(byte1_t id, byte2_t index, const byte1_t* p_data)
  {
    size_t actual_len;
    //id invalid
    if(!(actual_len = lengthByID(id)))
      return -1;
  
    Packet* p_p = new Packet;
    p_p->sync = 0x00;
    p_p->ID = id;
    p_p->index = index;

    memcpy(p_p->data, p_data, actual_len);
    memset(p_p->data + actual_len, '\0', sizeof(p_p->data) - actual_len);

    //CRC
    p_p->checksum = Protocol::crc16Gen(&(p_p->ID), 19, CRC16_GEN_BUYPASS);
    //COBS
    Protocol::cobsEncode(&(p_p->ohb), 23, p_p->sync);
    _pdu_queue.push(p_p);

    return 0;
  }

}

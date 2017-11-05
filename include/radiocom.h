#ifndef _RADIOCOM
#define _RADIOCOM
#include "protocol.h"
#include <queue>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>


namespace rfcom
{
  /*Radio communication module base class*/
  class Transceiver
  {
  public:
    Transceiver(byte2_t gen = CRC16_GEN_BUYPASS)
      : _crc_gen(gen), _listen_stop(true){pthread_mutex_init(&_pdu_lock, NULL);}
    ~Transceiver();

    /**
      Initialize serial port.
      *The configuration of serial port is written in source code.
      @param
      p_name: port name
      @return
      Same as open()
     */
    int initPort(const std::string& p_name);

    /**
       Terminate corresponding serial port. Will automatically stop listener.
     */
    void termPort();
    
    /**
       Create and start listener thread. Do nothing if listener thread has been created and running.
       Listener will automatically push the received data into PDU queue.
       If the received message is longer than sizeof(Packet), then listener will split it into pieces, then push.
       i.e. [50-bit-long-data] -> [24-bit-long-data][24-bit-long-data][2-bit-long-data]
       @return
       0: Listener thread created or already running.
       others: same as pthread_create()
     */
    int startListener();

    /**
       Stop Listener thread. Do nothing if listener thread was never created.
       Will wait for listener thread to join its parent thread.
     */
    void stopListener();
    
    /**
       Try to pop the next packet from queue and unpack it.
       @params
       id: the reference of ID.
       index: the refernece packet index.
       p_data: starting position of data buffer. 
       @return
       0: Success. Delete the packet from queue.
       -1: No more packets in PDU queue.
       -2: COBS decode failure. Keep packet in queue.
       -3: CRC mismatch. Keep packet in queue
     */
    int tryPopUnpack(byte1_t& id, byte2_t& index, byte1_t* p_data);
    
    /**
       Extract the next packet from receiver PDU queue.
       @params
       p: Extracted packet
       @return
       true: PDU queue is not empty and the nect Packet is extracted
       false: PDU queue is empty. Nothing extracted.
     */
    bool extractNext(Packet& p);

    /**
       pack up and send the packet.
       @params
       id: packet id
       index: packet index
       p_data: starting position of data buffer.
       @return
       0: Success.
       -1: write() error. Turn on _COM_DEBUG to see error type
       -2: invalid id.
     */
    int packSend(byte1_t id, byte2_t index, const byte1_t* p_data);

    /**
       Send raw character array.
       @params
       buf: charecter array.
       len: length of array.
       @return
       0: Success.
       -1: write() error. Turn on _COM_DEBUG to see error type.
     */
    int sendRaw(const byte1_t* buf, size_t len);
    
    /**
       Clear PDU queue.
     */
    void clearPDUQueue();
  private:
    int _s_fd;  //serial port file descripter

    byte2_t _crc_gen;  //CRC16 generator polynomial
    
    std::queue<Packet*> _pdu_queue;  //A queue of protocal data units. Listener.
    pthread_mutex_t _pdu_lock; 

    pthread_t _listen_thread_t;
    volatile bool _listen_stop; //listener thread stop flag. Listener checks this flag.
    
    int _init_port();
    inline int _term_port(){ return close(_s_fd); }
    static void* _listener_work(void* arg);
    
  };
}
#endif

#ifndef _RADIOCOM
#define _RADIOCOM
#include "protocol.h"
#include "fsm.h"
#include <queue>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <termio.h>

namespace rfcom
{
  /*Radio communication module base class*/
  class Transceiver
  {
  public:
    Transceiver(byte2_t gen = CRC16_GEN_BUYPASS);
    ~Transceiver();
    
    /**
      Initialize serial port , open raw log file and packet log file.
      @param
      p_name: port name
      raw_log_name: raw log file name. By default the transceiver will not write raw log.
      packet_log_name: packet log file name. By default the tranceiver will not write packet log.
      @return
      0: Successfully initialized serial port and opened log file.
      -1: Fail to initialize serial port. Enable _COM_DEBUG to see details.
      -2: Fail to open raw log file.
      -3: Fail to open packet log file.
     */
    int initPort(const std::string& p_name, const speed_t& baud,
		 const std::string& raw_log_name = "", const std::string& packet_log_name = "");

    /**
       Terminate corresponding serial port. Will invoke stopReceiving().
     */
    void termPort();
    
    /**
       Create and start listener thread and divider thread.
       Do nothing if corresponding thread is already up and running.
       @return
       0: Both listener thread and divider thread is created or already running.
       -1: listener thread fail to start.
       -2: divider thread fail to start.
       Turn on _COM_DEBUG to see error type.
     */
    int startReceiving();

    /**
       Stop Listener thread and divider thread. 
       Do nothing if corresponding thread was never created.
       Will wait for both threads to join its parent thread.
     */
    void stopReceiving();
    
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
    int packSend(byte1_t id, const byte1_t* p_data);

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

    void clearByteStream();
    void clearPDUStream();
    void clearSyncTimeStream();
  private:
    int _s_fd;  //serial port file descripter
    std::ofstream _fs_raw_log;    //output raw log file stream
    std::ofstream _fs_packet_log; //output packet log file stream
    
    byte2_t _crc_gen;  //CRC16 generator polynomial
    byte2_t _send_index;
    
    std::queue<byte1_t> _byte_stream;  //The stream of received bytes. 
    pthread_mutex_t _byte_stream_lock; //Correspoding mutex lock.
    
    std::queue<Packet*> _pdu_stream;  //The stream of protocal data units.
    pthread_mutex_t _pdu_stream_lock;  //Correspoding mutex lock.

    std::queue<timeval> _synctime_stream; //The stream of sync time stamp
    pthread_mutex_t _synctime_stream_lock; //Correspoding mutex lock.

    pthread_t _listen_thread_t;
    volatile bool _listen_stop; //Listener thread stop flag. Listener thread will check this flag.
    pthread_t _divide_thread_t;
    volatile bool _divide_stop; //Divider thread stop flag. Divider thread will check this flag.

    //This is a finite state machine, used to detect packets, Including corrupted ones.
    //char --- state index type
    //bool --- state output type
    //bool --- transition condition type
    fsm::FSM<char, bool, bool> _packet_detector;

    
    /**
       Initialize the packet detector, which is a finite state machine.
       sizeof(Packet) + 1 states, including a init state.
     */
    void _init_packet_detector();

    //inline int _term_port(){ return close(_s_fd); }
    /**
       Read from serial port, push received byte(s) to byte stream.
     */
    static void* _listener_work(void* arg);
    
    /**
       Read from byte stream and push packets to PDU stream.
     */
    static void* _divider_work(void* arg);


    
    /**
       Add new entry to log file
       @params
       buf: packet contents to be written in log file
       len: length of buffer
       entry_type: 0 for received packets, 1 for successfully sent packets, 2 for failed-to-send packets
       @return
     */
    #define LOG_RX  0
    #define LOG_SS  1
    #define LOG_SF  2
    void _new_log_entry(const timeval& t, const byte1_t* buf, size_t len, int entry_type);


    
    /**
       Convert a timeval struct into string
       @param
       t: timeval object
       @return
       time string
     */
    std::string _get_time_str(const timeval& t);
  };
}
#endif

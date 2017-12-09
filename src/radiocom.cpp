#include <cstring>
#include <sstream>
#include <termios.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>
#include "../include/radiocom.h"

#define LOCK_BYTE(X)         pthread_mutex_lock(&(X)->_byte_stream_lock)
#define UNLOCK_BYTE(X)       pthread_mutex_unlock(&(X)->_byte_stream_lock)
  
#define LOCK_SYNCTIME(X)     pthread_mutex_lock(&(X)->_synctime_stream_lock);
#define UNLOCK_SYNCTIME(X)   pthread_mutex_unlock(&(X)->_synctime_stream_lock);
  
#define LOCK_PDU(X)          pthread_mutex_lock(&(X)->_pdu_stream_lock);
#define UNLOCK_PDU(X)        pthread_mutex_unlock(&(X)->_pdu_stream_lock);



namespace rfcom
{
  //pthread_mutex_t Transceiver::_pdu_stream_lock = PTHREAD_MUTEX_INITIALIZER;
  Transceiver::Transceiver(byte2_t gen)
    : _port_connected(false), _crc_gen(gen), _send_index(0), _listen_stop(true), _divide_stop(true)
  {
    pthread_mutex_init(&_byte_stream_lock, NULL);
    pthread_mutex_init(&_pdu_stream_lock, NULL);
    pthread_mutex_init(&_synctime_stream_lock, NULL);
    _init_packet_detector();
  }
  
  Transceiver::~Transceiver()
  {
    termPort();
    pthread_mutex_destroy(&_synctime_stream_lock);
    pthread_mutex_destroy(&_pdu_stream_lock);
    pthread_mutex_destroy(&_byte_stream_lock);
  }

  void Transceiver::_init_packet_detector()
  {
    char index;
    //Set the index and output of each state 
    _packet_detector.setState('0', false);
    for(index = 'a'; index <= 'w'; ++index)
      _packet_detector.setState(index, false);
    _packet_detector.setState('x', true);


    //Set transitions
    _packet_detector.setTrans('0', false, 'a');
    _packet_detector.setTrans('0', true, 'a');
    for(index = 'a'; index <= 'w'; ++index)
      {
	_packet_detector.setTrans(index, true, index + 1);
	_packet_detector.setTrans(index, false, 'x');
      }
    _packet_detector.setTrans('x', false, 'a');
    _packet_detector.setTrans('x', true, 'a');
    

    //Set current state to initial state. Ready to work.
    _packet_detector.setCurrState('0');
  }
  
  
  int Transceiver::initPort(const std::string& p_name, const speed_t& baud,
			    const std::string& raw_log_name, const std::string& packet_log_name)
  {
#ifdef _COM_DEBUG
	std::cout << "Transceiver: initPort() invoked" << std::endl;
#endif

    //Fails to open serial port
    if((_s_fd = open(p_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
      {
#ifdef _COM_DEBUG
	std::cout << "Transceiver: open port " << p_name <<
	  " error code " << std::dec << _s_fd << std::endl;
#endif	
	return -1;
      }

    if(_port_name != p_name)
      _port_name = p_name; //Set private port name. For reconnection when disconnected
    if(_baud_rate != baud)
      _baud_rate = baud;   //Set private baud rate. For reconnection when disconnected
    _port_connected = true;
    
    //Valid raw log name
    if(raw_log_name != "")
      {
	//will rewrite raw log file
	_fs_raw_log.open(raw_log_name, std::ios::trunc);	
	//Fails to open raw log file
	if(!_fs_raw_log.is_open())
	  return -2;
      }

    //Valid packet log name
    if(packet_log_name != "")
      {
	//will rewrite packet log file
	_fs_packet_log.open(packet_log_name, std::ios::trunc);	
	//Fails to open packet log file
	if(!_fs_packet_log.is_open())
	  return -3;
      }
    

    struct termios ttyS_io;
    bzero(&ttyS_io, sizeof(ttyS_io));
    cfsetispeed(&ttyS_io, baud);
    cfsetospeed(&ttyS_io, baud);
    ttyS_io.c_cflag = CS8 | CLOCAL | CREAD;
    ttyS_io.c_iflag = IGNPAR;
    ttyS_io.c_oflag = 0;
    ttyS_io.c_lflag = 0;

    tcflush(_s_fd, TCIFLUSH);
    tcsetattr(_s_fd, TCSANOW, &ttyS_io);
    return 0;
  }

  void Transceiver::termPort()
  {
#ifdef _COM_DEBUG
	std::cout << "Transceiver: termPort() invoked" << std::endl;
#endif
    //This will wait to join.
    stopReceiving();
    close(_s_fd);
    clearByteStream();
    clearPDUStream();
    clearSyncTimeStream();
    _fs_raw_log.close();
    _fs_packet_log.close();
  }
  
  int Transceiver::startReceiving()
  {
#ifdef _COM_DEBUG
	std::cout << "Transceiver: startReceiving() invoked" << std::endl;
#endif
    
    //Listener thread not created
    if(_listen_stop)
      {
	_listen_stop = false;
	if(pthread_create(&_listen_thread_t, NULL, _listener_work, this) != 0)
	  return -1;
      }
#ifdef _COM_DEBUG
    else
	std::cout << "Transceiver: Listener thread already created." << std::endl;
#endif

    //Divider thread not created
    if(_divide_stop)
      {
	_divide_stop = false;
	if(pthread_create(&_divide_thread_t, NULL, _divider_work, this) != 0)
	  return -2;
      }
#ifdef _COM_DEBUG
    else
	std::cout << "Transceiver: Divider thread already created." << std::endl;
    
    std::cout << "Transceiver: Listener and divider threads are up and running." << std::endl;
#endif 
    
    return 0;
  }

  void Transceiver::stopReceiving()
  {
#ifdef _COM_DEBUG
	std::cout << "Transceiver: stopReceiving() invoked" << std::endl;
#endif
      
    if(_listen_stop)
      {
#ifdef _COM_DEBUG
	std::cout << "Transceiver: Listener thread not running" << std::endl;
#endif	
      }
    else
      {
	_listen_stop = true;
#ifdef _COM_DEBUG
	std::cout << "Transceiver: Waiting for listener thread to terminate" << std::endl;
#endif	
	pthread_join(_listen_thread_t, NULL);
#ifdef _COM_DEBUG
	std::cout << "Transceiver: Listener thread terminated" << std::endl;
#endif	
      }

    if(_divide_stop)
      {
#ifdef _COM_DEBUG
	std::cout << "Transceiver: Divider thread not running" << std::endl;
#endif	
      }
    else
      {
	_divide_stop = true;
#ifdef _COM_DEBUG
	std::cout << "Transceiver: Waiting for divider thread to terminate" << std::endl;
#endif	
	pthread_join(_divide_thread_t, NULL);
#ifdef _COM_DEBUG
	std::cout << "Transceiver: Divider thread terminated" << std::endl;
#endif	
      }
  }
  
  void Transceiver::clearByteStream()
  {
    LOCK_BYTE(this);
    while(!_byte_stream.empty())
      _byte_stream.pop();
    UNLOCK_BYTE(this);
  }
  
  void Transceiver::clearPDUStream()
  {
    LOCK_PDU(this);
    while(!_pdu_stream.empty())
      {
	delete _pdu_stream.front();
	_pdu_stream.pop();
      }
    UNLOCK_PDU(this);
  }

  void Transceiver::clearSyncTimeStream()
  {
    LOCK_SYNCTIME(this);
    while(!_synctime_stream.empty())
      _synctime_stream.pop();
    UNLOCK_SYNCTIME(this);
  }
  
  void* Transceiver::_listener_work(void* arg)
  {
    Transceiver* obj_ptr = static_cast<Transceiver*>(arg);
    std::queue<byte1_t>& byte_stream_ref = obj_ptr->_byte_stream;
    std::queue<timeval>& synctime_stream_ref = obj_ptr->_synctime_stream;
    //Set poll in policy
    pollfd pfds[1];
    pfds[0].fd = obj_ptr->_s_fd;
    pfds[0].events = POLLIN;

    timeval t_buf;
    byte1_t b_buf;
    while(!obj_ptr->_listen_stop)
      {
	//If the serial port is accidentally disconnected 
	if(access(obj_ptr->_port_name.c_str(), F_OK) < 0)
	  {
	    obj_ptr->_port_connected = false;
	    continue;
	  }

	//Just been reconnected
	if(obj_ptr->_port_connected == false)
	  {
	    //Try to init port but fail
	    if(obj_ptr->initPort(obj_ptr->_port_name, obj_ptr->_baud_rate) == -1)
	      continue;
	    //init successful
	    else
	      pfds[0].fd = obj_ptr->_s_fd;
	  }

	//Only one file, pollin, 500ms block timeout.
	//A higher block timeout will result in less loops
	//But main thread will spend more time waiting to join.
	if(poll(pfds, 1, 500) > 0)
	  {
	    //Fail to read a byte. Put it in the log, but do not push it into byte stream.
	    if(read(obj_ptr->_s_fd, &b_buf, sizeof(byte1_t)) < 0)
	      {
		obj_ptr->_fs_raw_log << "--\t" << std::flush;
		continue;
	      }

	    //Acquire time stamp.
	    gettimeofday(&t_buf, NULL);
	    
	    obj_ptr->_fs_raw_log << std::hex << int(b_buf) << "\t" << std::flush;

	    //Push byte stream and sync time stream.
	    //Make sure both of them are pushed before any other threads could access these streams.
	    LOCK_SYNCTIME(obj_ptr);
	    LOCK_BYTE(obj_ptr);
	    synctime_stream_ref.push(t_buf);
	    byte_stream_ref.push(b_buf);
	    UNLOCK_BYTE(obj_ptr);
	    UNLOCK_SYNCTIME(obj_ptr);
	  }
      }
    
#ifdef _COM_DEBUG
    std::cout << "Listener: Request to terminate..." << std::endl;
#endif
    pthread_exit(NULL);
  }

  
  void* Transceiver::_divider_work(void* arg)
  {
    Transceiver* obj_ptr = static_cast<Transceiver*>(arg);
    std::queue<byte1_t>& byte_stream_ref = obj_ptr->_byte_stream;
    std::queue<timeval>& synctime_stream_ref = obj_ptr->_synctime_stream;
    std::queue<Packet*>& pdu_stream_ref = obj_ptr->_pdu_stream;
    fsm::FSM<char, bool, bool> packet_detector_ref = obj_ptr->_packet_detector;
      
    byte1_t b_buf;
    timeval t_buf;
    
    Packet* p_buf = new Packet;
    byte1_t* byte_pos = (byte1_t*)p_buf;
    memset(byte_pos, 0, sizeof(Packet));

    LOCK_BYTE(obj_ptr);
    //Even when _divide_stop is set, still will work until byte stream is empty
    while(!obj_ptr->_divide_stop || !byte_stream_ref.empty())
      {
	if(byte_stream_ref.empty())
	  {
	    UNLOCK_BYTE(obj_ptr);
	    usleep(1000);
	    continue;
	  }

	b_buf = byte_stream_ref.front();
	byte_stream_ref.pop();
	UNLOCK_BYTE(obj_ptr);

	//Copy this byte into p_buf
	*byte_pos = b_buf;
	
	//FSM transition
	packet_detector_ref.transit(bool(b_buf));
	
	//If FSM is at state a, take down time stamp
	if(packet_detector_ref.getCurrState() == 'a')
	  {
	    LOCK_SYNCTIME(obj_ptr);
	    t_buf = synctime_stream_ref.front();
	    UNLOCK_SYNCTIME(obj_ptr);
	  }
	//Pop time stamp.
	LOCK_SYNCTIME(obj_ptr);
	synctime_stream_ref.pop();
	UNLOCK_SYNCTIME(obj_ptr);
	
	//FSM is at x state, A complete packet detected.
	if(packet_detector_ref.getCurrOutput())
	  {
	    //push pdu stream
	    LOCK_PDU(obj_ptr);
	    pdu_stream_ref.push(p_buf);
	    UNLOCK_PDU(obj_ptr);

	    //New packet log entry
	    obj_ptr->_new_log_entry(t_buf, (byte1_t*)p_buf, sizeof(Packet), LOG_RX);

	    //New packet buffer
	    p_buf = new Packet;
	    byte_pos = (byte1_t*)p_buf;
	    memset(byte_pos, 0, sizeof(Packet));
	  }
	//FSM is at a-w states, Packet not complete.
	else
	  ++byte_pos;

	//Lock it for next round 'while' statement
	LOCK_BYTE(obj_ptr);
      }

    //If there is still some leftover bytes in p_buf, take it as a whole packet.
    if(packet_detector_ref.getCurrState() != 'x')
      {
	//push pdu stream
	LOCK_PDU(obj_ptr);
	pdu_stream_ref.push(p_buf);
	UNLOCK_PDU(obj_ptr);

	//New packet log entry
	obj_ptr->_new_log_entry(t_buf, (byte1_t*)p_buf, sizeof(Packet), LOG_RX);
      }
    else
      delete p_buf;
    
#ifdef _COM_DEBUG
    std::cout << "Divider: Request to terminate..." << std::endl;
#endif
    pthread_exit(NULL);
  }
  
  void Transceiver::_new_log_entry(const timeval& t, const byte1_t* buf, size_t len, int entry_type)
  {
    if(entry_type == LOG_RX)
      _fs_packet_log << "RX : ";
    
    if(entry_type == LOG_SS)
      _fs_packet_log << "SS : ";

    if(entry_type == LOG_SF)
      _fs_packet_log << "SF : ";
    
    _fs_packet_log << _get_time_str(t);
    _fs_packet_log << "\t";
    for(size_t index = 0; index < len; ++index)
      _fs_packet_log << std::hex << int(buf[index]) << " ";
    _fs_packet_log << std::endl;
  }

  std::string Transceiver::_get_time_str(const timeval& t)
  {
    int milli = t.tv_usec / 1000;

    char buffer [80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&t.tv_sec));

    std::ostringstream oss;
    oss << buffer;
    oss << ":" << milli;
      
    char currentTime[84] = "";
    sprintf(currentTime, "%s:%d", buffer, milli);
    return oss.str();
  }
 
  int Transceiver::tryPopUnpack(byte1_t& id, byte2_t& index, byte1_t* p_data)
  {
    pthread_mutex_lock(&_pdu_stream_lock);
    //PDU stream empty
    if(_pdu_stream.empty())
      {
	pthread_mutex_unlock(&_pdu_stream_lock);
	return -1;
      }
    
    Packet p;
    memcpy(&p, _pdu_stream.front(), sizeof(p));
    pthread_mutex_unlock(&_pdu_stream_lock);
    
    //COBS decode failure
    if(!Protocol::cobsDecode(&(p.ohb), 23, p.sync))
      return -2;

    //CRC mismatch
    if(Protocol::crc16Gen(&(p.ID), 19, _crc_gen) != p.checksum)
      return -3;

    id = p.ID;
    index = p.index;
    memcpy(p_data, p.data, sizeof(p.data));


    LOCK_PDU(this);
    delete _pdu_stream.front();
    _pdu_stream.pop();
    UNLOCK_PDU(this);
    
    return 0;
  }

  bool Transceiver::extractNext(Packet& p)
  {
    LOCK_PDU(this);
    if(_pdu_stream.empty())
      {
	UNLOCK_PDU(this);
	return false;
      }

    p = *_pdu_stream.front();
    delete _pdu_stream.front();
    _pdu_stream.pop();
    
    UNLOCK_PDU(this);
    return true;
  }
  
  
  int Transceiver::packSend(byte1_t id, const byte1_t* p_data)
  {
    size_t actual_len;
    //id invalid
    if(!(actual_len = lengthByID(id)))
      return -2;
  
    Packet p;
    p.sync = 0x00;
    p.ID = id;
    p.index = _send_index++;

    memcpy(p.data, p_data, actual_len);
    //Fill the unused memory in p.data with '\0'
    memset(p.data + actual_len, '\0', sizeof(p.data) - actual_len);

    //CRC
    p.checksum = Protocol::crc16Gen(&(p.ID), 19, _crc_gen);
    //COBS
    Protocol::cobsEncode(&(p.ohb), 23, p.sync);
    
    return sendRaw((byte1_t*)(&p), sizeof(Packet));
  }

  int Transceiver::sendRaw(const byte1_t* buf, size_t len)
  {
    timeval s_time;  //packet send time
    int len_sent = write(_s_fd, buf, len);
    gettimeofday(&s_time, NULL);  //Take a snapshot of surrent time
    
#ifdef _COM_DEBUG
    std::cout << "Tranceiver: " << std::dec << len_sent << " characters sent" << std::endl;    
#endif

    if(len_sent < 0)
      {
	_new_log_entry(s_time, buf, len, LOG_SF);
	return -1;
      }

    _new_log_entry(s_time, buf, len, LOG_SS);
    return 0;
  }
}

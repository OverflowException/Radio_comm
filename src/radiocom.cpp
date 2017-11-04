#include <cstring>
#include <termios.h>
#include <fcntl.h>
#include <sys/poll.h>
#include "../include/radiocom.h"

namespace rfcom
{
  Transceiver::~Transceiver()
  {
    termPort();
    while(!_pdu_queue.empty())
      {
	delete _pdu_queue.front();
	_pdu_queue.pop();
      }
  }
  
  int Transceiver::initPort(const std::string& p_name)
  {
    //Fail to open
    if((_s_fd = open(p_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
      return _s_fd;

    struct termios ttyS_io;
    bzero(&ttyS_io, sizeof(ttyS_io));
    cfsetispeed(&ttyS_io, B115200);
    cfsetospeed(&ttyS_io, B115200);
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
    stopListener();
    close(_s_fd);
  }
  
  void* Transceiver::_listener_work(void* arg)
  {
    Transceiver* obj_ptr = static_cast<Transceiver*>(arg);
    //Set poll in policy
    pollfd pfds[1];
    pfds[0].fd = obj_ptr->_s_fd;
    pfds[0].events = POLLIN;

    int len;
    Packet* p_buf = NULL;
    while(!obj_ptr->_listen_stop)
      {
	//Only one file, pollin, 500ms block timeout.
	//A higher block timeout will result in less loops
	//But main thread will spend more time waiting for join.
	if(poll(pfds, 1, 500) > 0)
	  {
	    p_buf = new Packet;
	    //memset(&p_buf, 0, sizeof(Packet));
	    len = read(obj_ptr->_s_fd, (byte1_t*)p_buf, sizeof(Packet));
#ifdef _COM_DEBUG
	    std::cout << "Listener: " << std:: dec << len << " characters read" << std::endl;
	    for(int offset = 0; offset < len; ++offset)
	      std::cout << std::hex << int(*((byte1_t*)p_buf + offset)) << " ";
	    std::cout << std::endl;
#endif
	    
	    pthread_mutex_lock(&obj_ptr->_pdu_lock);
	    obj_ptr->_pdu_queue.push(p_buf);
	    pthread_mutex_unlock(&obj_ptr->_pdu_lock);
	  }
      }
#ifdef _COM_DEBUG
    std::cout << "Listener: Request to terminate..." << std::endl;
#endif
    pthread_exit(NULL);
  }
  
  int Transceiver::startListener()
  {
    //Already started;
    if(!_listen_stop)
      {
#ifdef _COM_DEBUG
	std::cout << "Parent of listener: Listener already created..." << std::endl;
#endif
	return 0;
      }
    
    _listen_stop = false;
    pthread_mutex_init(&_pdu_lock, NULL);
    //Create thread
    int ret;
    if((ret = pthread_create(&_listen_thread_t, NULL, _listener_work, this)) != 0)
      return ret;
    
#ifdef _COM_DEBUG
    std::cout << "Parent of listener: Listener created..." << std::endl;
#endif
    
    return 0;
  }

  void Transceiver::stopListener()
  {
    //Already stopped;
    if(_listen_stop)
      {
#ifdef _COM_DEBUG
	std::cout << "Parent of listener: Listener not running" << std::endl;
#endif
	return;
      }
    
    _listen_stop = true;
#ifdef _COM_DEBUG
    std::cout << "Parent of listener: Waiting for listener to terminate..." << std::endl;
#endif
    
    pthread_join(_listen_thread_t, NULL);

#ifdef _COM_DEBUG
    std::cout << "Parent of listener: Listener terminated " << std::endl;
#endif
 
  }
  
  int Transceiver::tryPopUnpack(byte1_t& id, byte2_t& index, byte1_t* p_data)
  {
    pthread_mutex_lock(&_pdu_lock);
    //PDU queue empty
    if(_pdu_queue.empty())
      {
	pthread_mutex_unlock(&_pdu_lock);
	return -1;
      }

    
    Packet p;
    memcpy(&p, _pdu_queue.front(), sizeof(p));
    pthread_mutex_unlock(&_pdu_lock);
    
    //COBS decode failure
    if(!Protocol::cobsDecode(&(p.ohb), 23, p.sync))
      return -2;

    //CRC mismatch
    if(Protocol::crc16Gen(&(p.ID), 19, _crc_gen) != p.checksum)
      return -3;

    id = p.ID;
    index = p.index;
    memcpy(p_data, p.data, sizeof(p.data));


    pthread_mutex_lock(&_pdu_lock);
    delete _pdu_queue.front();
    _pdu_queue.pop();
    pthread_mutex_unlock(&_pdu_lock);
    
    return 0;
  }

  bool Transceiver::retrieveNext(Packet& p)
  {
    
    pthread_mutex_lock(&_pdu_lock);
    if(_pdu_queue.empty())
      {
	pthread_mutex_unlock(&_pdu_lock);
	return false;
      }
    
    memcpy(&p, _pdu_queue.front(), sizeof(Packet));
    pthread_mutex_unlock(&_pdu_lock);
    return true;
  }
  
  
  int Transceiver::packSend(byte1_t id, byte2_t index, const byte1_t* p_data)
  {
    size_t actual_len;
    //id invalid
    if(!(actual_len = lengthByID(id)))
      return -1;
  
    Packet p;
    p.sync = 0x00;
    p.ID = id;
    p.index = index;

    memcpy(p.data, p_data, actual_len);
    memset(p.data + actual_len, '\0', sizeof(p.data) - actual_len);

    //CRC
    p.checksum = Protocol::crc16Gen(&(p.ID), 19, _crc_gen);
    //COBS
    Protocol::cobsEncode(&(p.ohb), 23, p.sync);

    int len_sent = write(_s_fd, (byte1_t*)(&p), sizeof(p));
    
#ifdef _COM_DEBUG
    std::cout << std::dec << len_sent << " characters sent " << std::endl;    
#endif
    
    return 0;
  }
}

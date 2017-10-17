#ifndef _RADIOCOM
#define _RADIOCOM

#include "protocol.h"
#include <queue>
#include <iostream>
#include <iterator>
#include <algorithm>
#include "pipes/pipes.h"


namespace rfcom {

	/*Radio communication module base class*/
	class ComModule {
	protected:
		byte2_t _crc_gen; //CRC16 generator polynomial
		std::queue<Packet*> _pdu_send_queue; //A queue of protocal data units
		std::queue<Packet*> _pdu_recv_queue;

	public:

		ComModule(byte2_t gen) {
			_crc_gen = gen;
		}

		virtual ~ComModule() {
			while (!_pdu_send_queue.empty()) {
				delete _pdu_send_queue.front();
				_pdu_send_queue.pop();
			}
			while (!_pdu_recv_queue.empty()) {
				delete _pdu_recv_queue.front();
				_pdu_recv_queue.pop();
			}
		}

		void setCRCGen(byte2_t gen) {
			_crc_gen = gen;
		}

		byte2_t getCRCGen() const {
			return _crc_gen;
		}
	};

	class Transceiver : public ComModule {
	public:

		Transceiver(byte2_t gen = CRC16_GEN_BUYPASS, int recv_fd, int send_fd) : ComModule(gen) {
			_fd_send = send_fd;
			_fd_recv = recv_fd;
		}

		Transeiver(byte2_t gen = CRC16_GEN_BUYPASS, Pipe pipes) : ComModule(gen) {
			_fd_send = pipes.getWritefd();
			_fd_recv = pipes.getReadfd();
		}

		~Transceiver() = default;

		/**
		   Try to pop the next packet from queue and unpack it.
		   @params
		   id: the reference of ID.
		   index: the reference packet index.
		   p_data: starting position of data buffer.
		   len: the actual length of buffer. Depends on ID
		   @return
		   0: Success. Delete the packet from queue.
		   -1: No more packets in PDU queue.
		   -2: COBS decode failure. Keep packet in queue.
		   -3: CRC mismatch. Keep packet in queue
		 */
		int unpack(byte1_t& id, byte2_t& index, byte1_t* p_data);

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
		int pack(byte1_t id, byte2_t index, byte1_t* p_pata);

		/**
		   Send the next packet in the queue.
		 */
		void sendNext();

		/**
		 * Receive any data in the buffer and push into the queue
		 */
		void recvNext();

	private:
		int _fd_send = 0;
		int _fd_recv = 0;
	};
}
#endif

#Makefile for CRC and COBS tests
CRC_TARGET = bin/crc_test
COBS_TARGET = bin/cobs_test
SEND_TARGET = bin/send_test
RECEIVE_TARGET = bin/receive_test

CXX = g++
CFLAGS = -Wall -O2 -std=c++11 -pthread
C_COMMAND = $(CXX) $(CFLAGS) $^ -o $@

all: $(CRC_TARGET) $(COBS_TARGET) $(RECEIVE_TARGET)

$(CRC_TARGET): tests/crc.cpp src/protocol.cpp
	$(C_COMMAND)

$(COBS_TARGET): tests/cobs.cpp src/protocol.cpp
	$(C_COMMAND)

$(RECEIVE_TARGET): tests/receive.cpp src/protocol.cpp src/radiocom.cpp
	$(C_COMMAND) -D _COM_DEBUG	#debug multithread communication
$(shell   mkdir -p bin)

clean:
	rm -fvr bin/*


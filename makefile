#Makefile for CRC and COBS tests
CRC_TARGET = bin/crc_test
COBS_TARGET = bin/cobs_test
SEND_TARGET = bin/send_test

CXX = g++
CFLAGS = -Wall -O2 -std=c++11
C_COMMAND = $(CXX) $(CFLAGS) $^ -o $@

all: $(CRC_TARGET) $(COBS_TARGET) $(SEND_TARGET)

$(CRC_TARGET): tests/crc.cpp src/protocol.cpp
	$(C_COMMAND)

$(COBS_TARGET): tests/cobs.cpp src/protocol.cpp
	$(C_COMMAND)

$(SEND_TARGET): tests/send.cpp src/protocol.cpp src/radiocom.cpp
	$(C_COMMAND)
$(shell   mkdir -p bin)

clean:
	rm -fvr bin/*


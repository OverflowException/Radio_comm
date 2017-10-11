#Makefile for CRC and COBS tests
CRC_TARGET = bin/crc_test
COBS_TARGET = bin/cobs_test

CXX = g++
CFLAGS = -Wall -O2 -std=c++11
C_COMMAND = $(CXX) $(CFLAGS) $^ -o $@

all: $(CRC_TARGET) $(COBS_TARGET)

$(CRC_TARGET): tests/crc.cpp src/protocol.cpp
	$(C_COMMAND)

$(COBS_TARGET): tests/cobs.cpp src/protocol.cpp
	$(C_COMMAND)

$(shell   mkdir -p bin)

clean:
	rm -fvr bin/*


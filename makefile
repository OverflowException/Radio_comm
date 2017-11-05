#Makefile for CRC and COBS tests
CRC_TARGET = bin/crc
COBS_TARGET = bin/cobs
SEND_TARGET = bin/send
RAW_TRANS_TARGET = bin/raw_tranceiver
REPEATED_RAW_TRANS_TARGET = bin/repeated_raw_tranceiver

CXX = g++
CFLAGS = -Wall -c -O2 -std=c++11
LFLAGS = -Wall -pthread
COMPILE = $(CXX) $(CFLAGS) $^ -o $@
LINK = $(CXX) $(LFLAGS) $^ -o $@

all: $(CRC_TARGET) $(COBS_TARGET) $(RAW_TRANS_TARGET) $(REPEATED_RAW_TRANS_TARGET)


$(CRC_TARGET): build/crc.o build/protocol.o
	$(LINK)
build/crc.o: tests/crc.cpp
	$(COMPILE)
build/protocol.o: src/protocol.cpp
	$(COMPILE)

$(COBS_TARGET): build/cobs.o build/protocol.o
	$(LINK)
build/cobs.o: tests/cobs.cpp
	$(COMPILE)

$(RAW_TRANS_TARGET): build/raw_transceiver.o build/protocol.o build/radiocom.o
	$(LINK)
build/raw_transceiver.o: tests/raw_transceiver.cpp
	$(COMPILE)
build/radiocom.o: src/radiocom.cpp
	$(COMPILE) -D _COM_DEBUG	#debug multithread communication

$(REPEATED_RAW_TRANS_TARGET): build/repeated_raw_transceiver.o build/protocol.o build/radiocom.o
	$(LINK)
build/repeated_raw_transceiver.o: tests/repeated_raw_transceiver.cpp
	$(COMPILE)


$(shell   mkdir -p bin)
$(shell	  mkdir -p build)

clean:
	rm -fvr bin/*
	rm -fvr build/*


CC = g++
CPPFLAGS = -std=c++11
LIBRAIES = -lpthread

TARGET = main.cpp tx/tx.cpp tx/tx_rpc_handler.cpp rpc/rpc.cpp mappings/mappings.cpp datastore/ds.cpp datastore/ds_req.cpp

all: main

main:
	$(CC) $(CPPFLAGS) $(TARGET) -o main $(LIBRAIES)

clean:
	rm main

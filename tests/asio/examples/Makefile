CC = g++
CFLAGS = -std=c++11 -c -O0 -ggdb -pthread -Wall

all: chat_client chat_server

chat_client: chat_client.o
	$(CC) chat_client.o -lboost_thread -lboost_system -o chat_client
	
chat_client.o: chat_client.cpp chat_message.hpp
	$(CC) $(CFLAGS) chat_client.cpp
	
chat_server: chat_server.o
	$(CC) chat_server.o -lboost_system -lboost_thread -o chat_server
	
chat_server.o: chat_server.cpp chat_message.hpp
	$(CC) $(CFLAGS) chat_server.cpp

clean:
	rm -fr *.o chat_server chat_client

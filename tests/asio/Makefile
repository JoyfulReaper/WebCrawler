CC = g++
CFLAGS = -std=c++11 -c -O0 -ggdb -Wall

async_http: async_http.o
	$(CC) async_http.o -lboost_system -o async_http

async_http.o: async_http.cpp
	$(CC) $(CFLAGS) async_http.cpp

timer: timer.o
	$(CC) timer.o -lboost_system -o timer

timer.o: timer.cpp
	$(CC) $(CFLAGS) timer.cpp

astimer: astimer.o
	$(CC) astimer.o -lboost_system -o astimer

astimer.o: astimer.cpp
	$(CC) $(CFLAGS) astimer.cpp

astimer2: astimer2.o
	$(CC) astimer2.o -lboost_system -o astimer2

astimer2.o: astimer2.cpp
	$(CC) $(CFLAGS) astimer2.cpp
	
strandTimer: strandTimer.o
	$(CC) strandTimer.o -lboost_system -lboost_thread -o strandTimer

strandTimer.o: strandTimer.cpp
	$(CC) $(CFLAGS) -pthread strandTimer.cpp

clean:
	rm -fr *.o timer astimer astimer2 strandTimer async_http

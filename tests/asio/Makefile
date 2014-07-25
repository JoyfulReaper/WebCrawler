CC = g++
CFLAGS = -std=c++11 -c -O0 -ggdb -Wall

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

clean:
	rm *.o timer astimer astimer2

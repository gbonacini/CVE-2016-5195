all: dcow

dcow: dcow.cpp
	g++ -Wall -pedantic -O2 -pthread -o dcow dcow.cpp -lutil

strip:  dcow
	strip ./dcow

clean:
	rm -f dcow

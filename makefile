all: dcow

dcow: dcow.cpp
	g++ -Wall -pedantic -std=c++11 -pthread -o dcow dcow.cpp 

clean:
	rm -f dcow

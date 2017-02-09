# PortScanner
Port Scanner, written in C++.

## **COMPILE**
	g++ -O0 -g3 -Wall -c -o ArgumentsManager.o ArgumentsManager.cpp
	
	g++ -O0 -g3 -Wall -c -o checksum.o checksum.cpp
	
	g++ -O0 -g3 -Wall -c -o communication.o communication.cpp
	
	g++ -O0 -g3 -Wall -c -o scan_functions.o scan_functions.cpp
	
	g++ -O0 -g3 -Wall -c -o main.o main.cpp
	
	g++ -static-libgcc -static-libstdc++ -o PortScanner ArgumentsManager.o scan_functions.o communication.o checksum.o main.o

## **RUN**
	./PortScanner

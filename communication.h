#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

const char* send_packet(unsigned char* packet, const char* dst, const char* port, int size_ip, int size_tcp);

#endif /* COMMUNICATION_H_ */

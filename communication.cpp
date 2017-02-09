#include "communication.h"

const char* send_packet(unsigned char* packet, const char* dst, const char* port, int size_ip,
		int size_tcp) {
	struct sockaddr_in sockaddr;
	int sd;
	const int on = 1;

	if ((sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		std::cout << "Couldn't create raw socket" << std::endl;
		exit(1);
	}

	if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
		std::cout << "setsockopt() failed" << std::endl;
		exit(1);
	}

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = atoi(port);
	sockaddr.sin_addr.s_addr = inet_addr(dst);

	if (sendto(sd, packet, size_ip + size_tcp, 0, (struct sockaddr *) &sockaddr,
			sizeof(struct sockaddr)) < 0) {
		std::cout << "Packet couldn't be sent" << std::endl;
		exit(1);
	}

	struct timeval timeout;      
    	timeout.tv_sec = 3;
    	timeout.tv_usec = 0;
	setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	char recvPacket[4096];
	recv(sd, recvPacket, sizeof(recvPacket), 0);
	
	std::string s = recvPacket;
	return s.c_str();
}

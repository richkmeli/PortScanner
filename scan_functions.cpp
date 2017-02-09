#include "scan_functions.h"

void SYN_scan(const char* src, const char* dst, const char* port) {
	unsigned char* packet;
	packet = (u_char *) malloc(60);

	struct ip ip;

	ip.ip_hl = 0x5;
	ip.ip_v = 0x4;
	ip.ip_tos = 0x0;
	ip.ip_len = htons(sizeof(ip));
	ip.ip_id = htons(12830);
	ip.ip_off = 0x0;
	ip.ip_ttl = 64;
	ip.ip_p = IPPROTO_TCP;
	ip.ip_src.s_addr = inet_addr(src);
	ip.ip_dst.s_addr = inet_addr(dst);
	ip.ip_sum = 0x0;
	ip.ip_sum = checksum((u_short *) &ip, sizeof(ip));

	// copy the IP header
	memcpy(packet, &ip, sizeof(ip));

	struct tcphdr tcp;

	tcp.source = htons(60000);
	tcp.dest = htons(80);
	tcp.seq = htonl(rand() % 100000);
	tcp.ack_seq = 0;

	tcp.doff = sizeof(tcp) / 4;

	tcp.fin = 0;
	tcp.syn = 1;
	tcp.rst = 0;
	tcp.psh = 0;
	tcp.ack = 0;
	tcp.urg = 0;

	tcp.res1 = 0; // nonce
	tcp.res2 = 0; // ECN

	tcp.window = htons(2048);
	tcp.check = checksum((unsigned short *) &tcp, sizeof(tcp));
	tcp.urg_ptr = 0;

	memcpy(packet + sizeof(ip), &tcp, sizeof(tcp));

	const char* response = send_packet(packet,dst,port,sizeof(ip),sizeof(tcp));

	std::cout << response;

}

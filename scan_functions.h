#ifndef SCAN_FUNCTIONS_H_
#define SCAN_FUNCTIONS_H_

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <cstring>

#include "checksum.h"
#include "communication.h"

void SYN_scan(const char* src, const char* dst, const char* port);

#endif

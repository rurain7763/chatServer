#pragma once

#define RECV_BUFF 2048

#include "iClient.h"
#include "iServer.h"

int createSocket(const char* servIp, uint16 servPort);
bool setSockBlockingStatus(long long socket, bool blocking);
void closeSocket(uint64 socket);
bool isend(uint64 socket, const char* msg);
char* irecv(uint64 socket);




#pragma once

#define INCLUDE_INETWORK_HEADER

#define RECV_BUFF 2048

#include "iServer.h"
#include "iConnectionManager.h"

extern pthread_mutex_t selectMutex;

void loadNetwork();
void endNetwork();

int createSocket(const char* servIp, uint16 servPort);
bool setSockBlockingStatus(long long socket, bool blocking);
void closeSocket(uint64 socket);

char* getIpByDomainName(const char* domain);

bool isend(uint64 socket, const char* msg);
char* irecv(uint64 socket);




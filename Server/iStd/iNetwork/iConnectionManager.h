#pragma once

#include "iType.h"

#ifdef _WIN32

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#endif

#define CONNECTION_LOOSE_MSG(msg)  !strcmp(msg, "__conloose__")

#define iCLIENT_BUFF_SIZE				      512  

#define iCONNECTIONMG_MAX_SOCKET	   FD_SETSIZE
#define iCONNECTIONMG_ALIVE_SIGN				0
#define iCONNECTIONMG_READY_TO_DEAD_SIGN		1
#define iCONNECTIONMG_DEAD_SIGN					2

#include "iHashTable.h"
#include "iString.h"
#include "iThreadPool.h"
#include "iQueue.h"

struct iConnectionManager;
class iConnection;

typedef void (*ResponseMethod)(iConnection* conn, const char* msg, int len);

class iConnectionManager
{
private:
	iConnectionManager();
	static iConnectionManager* S;

public:
	virtual ~iConnectionManager();

	iConnection* connectByIp(const char* ip, uint16 port, ResponseMethod method);
	iConnection* connectByDomain(const char* domain, uint16 port, ResponseMethod method);

	void disConnect(const char* ip);

	void shutDown();

	static iConnectionManager* share();

private:
	static void* active(void* iconMg);

public:
	uint8 sign;

	FD_SET recvSet;
	FD_SET sendSet;

	pthread_mutex_t mutex;

	iQueue con;
	iQueue disCon;

	iHashTable clients;
};

class iConnection
{
public:
	iConnection(uint64& socket, sockaddr_in& addr);
	virtual ~iConnection();

	bool sendMsg(const char* msg, int size);

public:
	uint64 socket;
	sockaddr_in addr;
	pthread_mutex_t mutex;

	char buff[iCLIENT_BUFF_SIZE];

	iString recvBuff;
	uint16 recvChunkLen;

	iString sendBuff;

	ResponseMethod method;
};





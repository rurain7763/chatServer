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

#include "iList.h"
#include "iString.h"
#include "iThreadPool.h"

#define iSERVERUSER_BUFF_SIZE 512
#define iUSERMANAGER_BUFF_SIZE 512

#define iUSERMANGER_WORKING			1
#define iUSERMANGER_READY_TO_DIE	2

#define iSERVER_WORKING_SIGN	0
#define iSERVER_EXIT_SIGN		1
#define iSERVER_READY_TO_DIE	2

enum iServErrCode
{
	iServErrCodeNoErr = 0,
	iServErrCodeListen,
	iServErrCodeSockStatusChanging,
	iServErrCodeAccept,
	iServErrCodeSelect,
};

struct iServerUser;
struct iUserManager;

class iServer
{
public:
	iServer(const char* ip, uint16 port);
	virtual ~iServer();

	static void* run(void* server);

	virtual void eventServStart();
	virtual void eventUserIn(iServerUser* user);
	virtual void eventUserWait(iServerUser* user);
	virtual void eventUserOut(iServerUser* user);
	virtual void eventUserRequest(iServerUser* user, const char* msg, int len);
	virtual void eventSendMsgToUser(iServerUser* user, int len);
	virtual void eventServExit();
	virtual void eventError(iServErrCode code);

	void sendMsgToUser(iServerUser* user, const char* msg);

private:
	static void* activeiUserManager(void* userMg);

public:
	int err;

	uint8 sign;
	pthread_mutex_t mutex;

	iUserManager* currManager;
	iList manager;

	uint64 servSock;
};

struct iServerUser
{
	uint64 socket;
	sockaddr_in addr;

	iString recvMsg;
	uint16 recvMsgChunkLen;

	iString sendMsg;
	uint16 sendMsgChunkLen;
};

struct iUserManager
{
	FD_SET recvSet;
	FD_SET sendSet;

	iServer* server;

	uint8 flag;
	char buff[iUSERMANAGER_BUFF_SIZE];
	iList users;
};






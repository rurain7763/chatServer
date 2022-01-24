#include "iServer.h"
#include "iStd.h"

iServer::iServer(const char* ip, uint16 port)
{
	servSock = createSocket(ip, port);

	err = listen(servSock, SOMAXCONN);
	if (err < 0)
	{
		eventError(iServErrCodeListen);
		closeSocket(servSock);
	}

	currManager = new iUserManager;
	currManager->server = this;
	currManager->flag = iUSERMANGER_WORKING;

	sign = iSERVER_WORKING_SIGN;

	pthread_mutex_init(&mutex, NULL);
	iThreadPool::share()->addJob(activeiUserManager, currManager);
	manager.push_back(currManager);
}

iServer::~iServer()
{
	pthread_mutex_lock(&mutex);
	sign = iSERVER_EXIT_SIGN;
	closeSocket(servSock);
	pthread_mutex_unlock(&mutex);

	while (sign != iSERVER_READY_TO_DIE) {}

	for (int i = 0; i < manager.num; i++)
	{
		iUserManager* um = (iUserManager*)manager[i];

		for (int j = 0; j < um->users.num; j++)
		{
			iServerUser* user = (iServerUser*)um->users[j];

			closeSocket(user->socket);
			delete user;
		}

		delete um;
	}

	pthread_mutex_destroy(&mutex);
}

void* iServer::run(void* server)
{
	iServer* serv = (iServer*)server;

	int result = 0;
	sockaddr_in userAddr;
	int addrLen = sizeof(sockaddr_in);

	serv->eventServStart();

	while (serv->sign == iSERVER_WORKING_SIGN)
	{
		result = accept(serv->servSock, (sockaddr*)&userAddr, &addrLen);

		if (result < 0)
		{
			serv->eventError(iServErrCodeAccept);
			continue;
		}

		iServerUser* user = new iServerUser;
		user->socket = result;
		user->addr = userAddr;
		user->recvMsg.resize(iSERVERUSER_BUFF_SIZE);
		user->recvMsgChunkLen = 0;
		user->sendMsg.resize(iSERVERUSER_BUFF_SIZE);
		user->sendMsgChunkLen = 0;

		pthread_mutex_lock(&serv->mutex);
		serv->eventUserWait(user);
		pthread_mutex_unlock(&serv->mutex);

		if (!setSockBlockingStatus(result, false))
		{
			pthread_mutex_lock(&serv->mutex);
			serv->eventError(iServErrCodeSockStatusChanging);
			pthread_mutex_unlock(&serv->mutex);
			continue;
		}

		if (serv->currManager->users.num == FD_SETSIZE)
		{
			iUserManager* um = new iUserManager;
			um->server = serv;
			um->flag = iUSERMANGER_WORKING;
			um->users.push_back(user);

			iThreadPool::share()->addJob(activeiUserManager, um);

			serv->currManager = um;
			serv->manager.push_back(um);
		}
		else
		{
			pthread_mutex_lock(&serv->mutex);
			serv->currManager->users.push_back(user);
			pthread_mutex_unlock(&serv->mutex);
		}

		pthread_mutex_lock(&serv->mutex);
		serv->eventUserIn(user);
		pthread_mutex_unlock(&serv->mutex);
	}

	serv->eventServExit();

	for (int i = 0; i < serv->manager.num; i++)
	{
		iUserManager* um = (iUserManager*)serv->manager[i];

		if (um->flag != iUSERMANGER_READY_TO_DIE) i--;
	}

	serv->sign = iSERVER_READY_TO_DIE;

	return NULL;
}

void iServer::eventServStart()
{
	printf("hello world\n");
}

void iServer::eventUserIn(iServerUser* user)
{
	printf("hello %s\n", inet_ntoa(user->addr.sin_addr));
}

void iServer::eventUserWait(iServerUser* user)
{
	printf("%s waiting\n", inet_ntoa(user->addr.sin_addr));
}

void iServer::eventUserOut(iServerUser* user)
{
	printf("goodbye %s\n", inet_ntoa(user->addr.sin_addr));
}

void iServer::eventUserRequest(iServerUser* user, const char* msg, int len)
{
	printf("%s : %s\n", inet_ntoa(user->addr.sin_addr), msg);
}

void iServer::eventSendMsgToUser(iServerUser* user, int len)
{
	printf("send %d bytes to %s\n", len, inet_ntoa(user->addr.sin_addr));
}

void iServer::eventServExit()
{
	printf("goodbye world\n");
}

void iServer::eventError(iServErrCode code)
{
	printf("%d error occured\n", code);
}

void iServer::sendMsgToUser(iServerUser* user, const char* m)
{
	uint16 msgLen = strlen(m);
	uint32 totalLen = msgLen + 2;

	char* msg = new char[totalLen];

	for (int i = 1; i > -1; i--)
	{
		uint8 c = ((uint8*)&msgLen)[i];
		msg[1 - i] = c;
	}

	memcpy(&msg[2], m, sizeof(char) * msgLen);

	user->sendMsg.forcingAppend(msg, totalLen);
	user->sendMsgChunkLen = totalLen;

	delete[] msg;
}

void* iServer::activeiUserManager(void* userMg)
{
	iUserManager* um = (iUserManager*)userMg;
	
	uint64 maxSock = 0;
	timeval tv = { 0, 0 };
	int result;

	while (um->server->sign == iSERVER_WORKING_SIGN)
	{
		if (um->users.num == 0) continue;

		FD_ZERO(&um->recvSet);
		FD_ZERO(&um->sendSet);

		pthread_mutex_lock(&um->server->mutex);
		iList::iIterator begin = um->users.begin();
		iList::iIterator end = um->users.end();
		pthread_mutex_unlock(&um->server->mutex);

		for (iList::iIterator itr = begin; itr != end; itr++)
		{
			iServerUser* user = (iServerUser*)itr->data;

			FD_SET(user->socket, &um->recvSet);
			FD_SET(user->socket, &um->sendSet);

#if __unix__
			if (maxSock < user->socket) maxSock = user->socket + 1ll;
#endif
		}

		pthread_mutex_lock(&um->server->mutex);
		result = select(maxSock, &um->recvSet, &um->sendSet, NULL, &tv);
		if (result < 0)
		{
			um->server->eventError(iServErrCodeSelect);
			continue;
		}
		pthread_mutex_unlock(&um->server->mutex);

		if (!result) continue;

		for (iList::iIterator itr = begin; itr != end; )
		{
			iServerUser* user = (iServerUser*)itr->data;

			if (FD_ISSET(user->socket, &um->recvSet))
			{
				result = recv(user->socket, um->buff, iUSERMANAGER_BUFF_SIZE, 0);

				if (result > 0)
				{
					user->recvMsg.forcingAppend(um->buff, result);
				}
#ifdef __unix__
				else if (result == 0 || errno != EAGAIN)
#elif _WIN32
				else if (result == 0 ||
						 WSAGetLastError() == WSAECONNRESET)
#endif
				{
					pthread_mutex_lock(&um->server->mutex);
					um->server->eventUserOut(user);
					um->users.erase(itr);
					closeSocket(user->socket);
					delete user;
					pthread_mutex_unlock(&um->server->mutex);
					continue;
				}

				if (user->recvMsg.len != 0)
				{
					if (user->recvMsg.len > 2 &&
						user->recvMsgChunkLen == 0)
					{
						uint8* ml = (uint8*)&user->recvMsgChunkLen;

						for (int i = 0; i < 2; i++)
						{
							ml[1 - i] = user->recvMsg[i];
						}

						user->recvMsg.erase(0, 2);
					}

					if (user->recvMsgChunkLen != 0 &&
						user->recvMsg.len >= user->recvMsgChunkLen)
					{
						int len = user->recvMsgChunkLen;

						char* msg = new char[len + 1];
						memcpy(msg, user->recvMsg.str, sizeof(char) * len);
						msg[len] = 0;

						pthread_mutex_lock(&um->server->mutex);
						um->server->eventUserRequest(user, msg, len);
						pthread_mutex_unlock(&um->server->mutex);

						delete[] msg;

						user->recvMsg.erase(0, len);
						user->recvMsgChunkLen = 0;
					}
				}				
			}

			if (user->sendMsgChunkLen != 0)
			{
				if (FD_ISSET(user->socket, &um->sendSet))
				{
					result = send(user->socket, user->sendMsg.str,
								  user->sendMsgChunkLen, 0);

					if (result > 0)
					{
						pthread_mutex_lock(&um->server->mutex);
						um->server->eventSendMsgToUser(user, result);
						user->sendMsgChunkLen -= result;
						user->sendMsg.erase(0, result);
						pthread_mutex_unlock(&um->server->mutex);
					}
					else
					{
						pthread_mutex_lock(&um->server->mutex);
						um->server->eventUserOut(user);
						um->users.erase(itr);
						closeSocket(user->socket);
						delete user;
						pthread_mutex_unlock(&um->server->mutex);
						continue;
					}
				}
			}

			itr++;
		}
	}

	um->flag = iUSERMANGER_READY_TO_DIE;

	return NULL;
}



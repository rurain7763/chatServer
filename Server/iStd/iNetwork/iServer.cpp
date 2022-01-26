#include "iServer.h"
#include "iStd.h"

iServer::iServer(const char* ip, uint16 port)
{
	servSock = createSocket(ip, port);
	pthread_mutex_init(&mutex, NULL);
	err = 0;
	currManager = NULL;
	sign = iSERVER_EXIT_SIGN;
}

iServer::~iServer()
{
	if (sign == iSERVER_WORKING_SIGN)
	{
		pthread_mutex_lock(&mutex);
		sign = iSERVER_EXIT_SIGN;
		closeSocket(servSock);
		pthread_mutex_unlock(&mutex);

		while (sign != iSERVER_READY_TO_DIE) {}

		for (int i = 0; i < manager.num; i++)
		{
			iUserManager* um = (iUserManager*)manager[i];
			delete um;
		}
	}

	closeSocket(servSock);
	pthread_mutex_destroy(&mutex);
}

void* iServer::run(void* server)
{
	iServer* serv = (iServer*)server;

	serv->err = listen(serv->servSock, SOMAXCONN);
	if (serv->err < 0)
	{
		serv->eventError(iServErrCodeListen);
		closeSocket(serv->servSock);

		return NULL;
	}

	serv->currManager = new iUserManager(serv);

	serv->sign = iSERVER_WORKING_SIGN;
	serv->manager.push_back(serv->currManager);
	iThreadPool::share()->addJob(activeiUserManager, serv->currManager);

	int result = 0;
	sockaddr_in userAddr;
	int addrLen = sizeof(sockaddr_in);

	serv->eventServStart();

	while (serv->sign != iSERVER_EXIT_SIGN)
	{
		pthread_mutex_unlock(&serv->mutex);
		result = accept(serv->servSock, (sockaddr*)&userAddr, &addrLen);
		pthread_mutex_lock(&serv->mutex);

		if (result < 0)
		{
			serv->eventError(iServErrCodeAccept);
			continue;
		}

		if (!setSockBlockingStatus(result, false))
		{
			serv->eventError(iServErrCodeSockStatusChanging);
			continue;
		}

		iServerUser* user = new iServerUser(result, userAddr);

		if (serv->currManager->users.num == FD_SETSIZE)
		{
			iUserManager* um = new iUserManager(serv);

			user->userMg = um;
			um->request.push(user);

			serv->currManager = um;
			serv->manager.push_back(um);

			iThreadPool::share()->addJob(activeiUserManager, um);
		}
		else
		{
			pthread_mutex_lock(&serv->currManager->mutex);
			user->userMg = serv->currManager;
			serv->currManager->request.push(user);
			pthread_mutex_unlock(&serv->currManager->mutex);
		}
	}

	serv->eventServExit();

	for (int i = 0; i < serv->manager.num; i++)
	{
		iUserManager* um = (iUserManager*)serv->manager[i];

		if (um->sign != iUSERMANGER_READY_TO_DIE) i--;
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

void iServer::sendMsgToUser(iServerUser* user, const char* m, int size)
{
	uint16 msgLen = size;
	uint32 totalLen = msgLen + 2;

	char* msg = new char[totalLen];

	for (int i = 1; i > -1; i--)
	{
		uint8 c = ((uint8*)&msgLen)[i];
		msg[1 - i] = c;
	}

	memcpy(&msg[2], m, sizeof(char) * msgLen);

	pthread_mutex_lock(&user->userMg->mutex);
	user->sendMsg.forcingAppend(msg, totalLen);
	pthread_mutex_unlock(&user->userMg->mutex);

	delete[] msg;
}

void iServer::shutDownUser(iServerUser* user)
{
	pthread_mutex_lock(&user->userMg->mutex);
	user->sign = iSERVERUSER_DEAD_SIGN;
	pthread_mutex_unlock(&user->userMg->mutex);
}

void iServer::shutDown()
{
	if (sign == iSERVER_WORKING_SIGN)
	{
		pthread_mutex_lock(&mutex);
		sign = iSERVER_EXIT_SIGN;
		closeSocket(servSock);
		pthread_mutex_unlock(&mutex);

		while (sign != iSERVER_READY_TO_DIE) {}

		for (int i = 0; i < manager.num; i++)
		{
			iUserManager* um = (iUserManager*)manager[i];
			delete um;
		}

		err = 0;
		currManager = NULL;
	}
}

void* iServer::activeiUserManager(void* userMg)
{
	iUserManager* um = (iUserManager*)userMg;

	um->sign = iUSERMANGER_WORKING;

	uint64 maxSock = 0;
	timeval tv = { 0, 0 };
	int result;

	while (um->server->sign != iSERVER_EXIT_SIGN)
	{
		FD_ZERO(&um->recvSet);
		FD_ZERO(&um->sendSet);

		pthread_mutex_lock(&um->mutex);
		while (!um->request.empty())
		{
			iServerUser* user = (iServerUser*)um->request.pop();

			if (um->users.num >= FD_SETSIZE)
			{
				pthread_mutex_unlock(&um->mutex);
				um->server->eventUserWait(user);
				pthread_mutex_lock(&um->mutex);

				um->request.push(user);
			}
			else
			{
				pthread_mutex_unlock(&um->mutex);
				um->server->eventUserIn(user);
				pthread_mutex_lock(&um->mutex);

				user->sign = ISERVERUSER_ALIVE_SIGN;
				um->users.push_back(user);
			}
		}

		iList::iIterator begin = um->users.begin();
		iList::iIterator end = um->users.end();

		for (iList::iIterator itr = begin; itr != end; itr++)
		{
			iServerUser* user = (iServerUser*)itr->data;

			if (user->sign == iSERVERUSER_DEAD_SIGN)
			{
				pthread_mutex_unlock(&um->mutex);
				um->server->eventUserOut(user);
				pthread_mutex_lock(&um->mutex);

				um->users.erase(itr);
				begin = um->users.begin();
				end = um->users.end();
				closeSocket(user->socket);
				delete user;
				continue;
			}

			FD_SET(user->socket, &um->recvSet);
			if (user->sendMsg.len != 0) FD_SET(user->socket, &um->sendSet);

#if __unix__
			if (maxSock < user->socket) maxSock = user->socket + 1ll;
#endif
		}
		pthread_mutex_unlock(&um->mutex);

		if (um->users.num == 0) continue;

		pthread_mutex_lock(&selectMutex);
		result = select(maxSock, &um->recvSet, &um->sendSet, NULL, &tv);
		if (result < 0)
		{
			um->server->eventError(iServErrCodeSelect);
			pthread_mutex_unlock(&selectMutex);
			continue;
		}
		pthread_mutex_unlock(&selectMutex);

		if (!result) continue;

		for (iList::iIterator itr = begin; itr != end; itr++)
		{
			iServerUser* user = (iServerUser*)itr->data;

			if (FD_ISSET(user->socket, &um->recvSet))
			{
				result = recv(user->socket, um->buff, ISERVER_RECV_BUFF_SIZE, 0);

				if (result > 0)
				{
					user->recvMsg.forcingAppend(um->buff, result);
				}
				else
				{
					um->server->eventError(iServErrCodeConnectionOut);
					um->server->shutDownUser(user);
					continue;
				}
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

					um->server->eventUserRequest(user, msg, len);
					delete[] msg;

					user->recvMsg.erase(0, len);
					user->recvMsgChunkLen = 0;
				}
			}

			pthread_mutex_lock(&um->mutex);
			if (user->sendMsg.len != 0)
			{
				if (FD_ISSET(user->socket, &um->sendSet))
				{
					result = send(user->socket, user->sendMsg.str,
								  user->sendMsg.len, 0);

					if (result > 0)
					{
						um->server->eventSendMsgToUser(user, result);
						user->sendMsg.erase(0, result);
					}
					else
					{
						pthread_mutex_unlock(&um->mutex);
						um->server->eventError(iServErrCodeSend);
						um->server->shutDownUser(user);
						continue;
					}
				}
			}
			pthread_mutex_unlock(&um->mutex);
		}
	}

	um->sign = iUSERMANGER_READY_TO_DIE;

	return NULL;
}

iUserManager::iUserManager(iServer* serv)
	:request(ISERVER_QUEUE_SIZE)
{
	server = serv;
	sign = iUSERMANGER_READY_TO_DIE;

	FD_ZERO(&recvSet);
	FD_ZERO(&sendSet);

	pthread_mutex_init(&mutex, NULL);
}

iUserManager::~iUserManager()
{
	iList::iIterator begin = users.begin();
	iList::iIterator end = users.end();

	for (iList::iIterator itr = begin; itr != end; itr++)
	{
		iServerUser* user = (iServerUser*)itr->data;
		delete user;
	}

	while (!request.empty())
	{
		iServerUser* user = (iServerUser*)request.pop();
		delete user;
	}

	pthread_mutex_destroy(&mutex);
}

iServerUser::iServerUser(uint64 s, sockaddr_in a)
{
	sign = ISERVERUSER_WAIT_SIGN;

	socket = s;
	addr = a;

	recvMsg.resize(ISERVER_RECV_BUFF_SIZE);
	recvMsgChunkLen = 0;

	sendMsg.resize(ISERVER_RECV_BUFF_SIZE);
}

iServerUser::~iServerUser()
{
	closeSocket(socket);
}

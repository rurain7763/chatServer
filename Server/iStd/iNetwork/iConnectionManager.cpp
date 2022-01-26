#include "iConnectionManager.h"
#include "iStd.h"

iConnectionManager* iConnectionManager::S = NULL;

iConnectionManager::iConnectionManager()
	:con(FD_SETSIZE), disCon(FD_SETSIZE)
{
	sign = iCONNECTIONMG_DEAD_SIGN;

	FD_ZERO(&recvSet);
	FD_ZERO(&sendSet);

	pthread_mutex_init(&mutex, NULL);
}

iConnectionManager::~iConnectionManager()
{
	if (sign == iCONNECTIONMG_DEAD_SIGN)
	{
		pthread_mutex_destroy(&mutex);
	}
	else
	{
		sign = iCONNECTIONMG_DEAD_SIGN;

		while (sign != iCONNECTIONMG_READY_TO_DEAD_SIGN) {}

		while (!con.empty())
		{
			iConnection* cl = (iConnection*)con.pop();
			delete cl;
		}

		while (!disCon.empty())
		{
			char* ip = (char*)disCon.pop();
			delete[] ip;
		}

		pthread_mutex_destroy(&mutex);
	}
}

iConnection* iConnectionManager::connectByIp(const char* ip, uint16 port, ResponseMethod method)
{
	if (clients.num >= iCONNECTIONMG_MAX_SOCKET) return NULL;

	if (sign == iCONNECTIONMG_DEAD_SIGN)
	{
		sign = iCONNECTIONMG_ALIVE_SIGN;
		iThreadPool::share()->addJob(iConnectionManager::active, this);
	}

	uint64 sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) return NULL;

	if (!setSockBlockingStatus(sock, false))
	{
		closeSocket(sock);
		return NULL;
	}

	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &servAddr.sin_addr);

	if (!connect(sock, (sockaddr*)&servAddr, sizeof(servAddr)))
	{
		closeSocket(sock);
		return NULL;
	}

	iConnection* cl = new iConnection(sock, servAddr);
	cl->method = method;

	pthread_mutex_lock(&mutex);
	con.push(cl);
	pthread_mutex_unlock(&mutex);

	return cl;
}

iConnection* iConnectionManager::connectByDomain(const char* domain, uint16 port, ResponseMethod method)
{
	if (clients.num >= iCONNECTIONMG_MAX_SOCKET) return NULL;

	if (sign == iCONNECTIONMG_DEAD_SIGN)
	{
		sign = iCONNECTIONMG_ALIVE_SIGN;
		iThreadPool::share()->addJob(iConnectionManager::active, this);
	}

	uint64 sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) return NULL;

	if (!setSockBlockingStatus(sock, false))
	{
		closeSocket(sock);
		return NULL;
	}

	char* ip = getIpByDomainName(domain);

	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &servAddr.sin_addr);

	if (!connect(sock, (sockaddr*)&servAddr, sizeof(servAddr)))
	{
		closeSocket(sock);
		delete[] ip;
		return NULL;
	}

	iConnection* cl = new iConnection(sock, servAddr);
	cl->method = method;

	pthread_mutex_lock(&mutex);
	con.push(cl);
	pthread_mutex_unlock(&mutex);

	delete[] ip;

	return cl;
}

void iConnectionManager::disConnect(const char* ip)
{
	int len = strlen(ip);

	char* str = new char[len + 1];
	memcpy(str, ip, sizeof(char) * len);
	str[len] = 0;

	pthread_mutex_lock(&mutex);
	disCon.push(str);
	pthread_mutex_unlock(&mutex);
}

void iConnectionManager::shutDown()
{
	sign = iCONNECTIONMG_DEAD_SIGN;

	while (sign != iCONNECTIONMG_READY_TO_DEAD_SIGN) {}

	while (!con.empty())
	{
		iConnection* cl = (iConnection*)con.pop();
		delete cl;
	}

	while (!disCon.empty())
	{
		char* ip = (char*)disCon.pop();
		delete[] ip;
	}

	sign = iCONNECTIONMG_DEAD_SIGN;
}

iConnectionManager* iConnectionManager::share()
{
	if (!S) S = new iConnectionManager();
	return S;
}

void* iConnectionManager::active(void* iconMg)
{
	iConnectionManager* connMg = (iConnectionManager*)iconMg;

	uint64 maxSock = 0;
	timeval tv = { 0, 0 };
	int result = 0;

	while (connMg->sign == iCONNECTIONMG_ALIVE_SIGN)
	{
		pthread_mutex_lock(&connMg->mutex);
		while (!connMg->con.empty())
		{
			iConnection* cl = (iConnection*)connMg->con.pop();
			connMg->clients.insert(inet_ntoa(cl->addr.sin_addr), cl);
		}

		while (!connMg->disCon.empty())
		{
			char* ip = (char*)connMg->disCon.pop();

			iConnection* cl = (iConnection*)connMg->clients[ip];
			delete cl;

			connMg->clients.remove(ip);

			delete[] ip;
		}
		pthread_mutex_unlock(&connMg->mutex);

		FD_ZERO(&connMg->recvSet);
		FD_ZERO(&connMg->sendSet);

		iHashTable::iIterator begin = connMg->clients.begin();
		iHashTable::iIterator end = connMg->clients.end();

		for (iHashTable::iIterator itr = begin; itr != end; itr++)
		{
			iConnection* cl = (iConnection*)itr->data;

			FD_SET(cl->socket, &connMg->recvSet);
			if (cl->sendBuff.len != 0) FD_SET(cl->socket, &connMg->sendSet);

#if __unix__
			if (maxSock < conn->socket) maxSock = conn->socket + 1ll;
#endif
		}

		if (connMg->clients.num == 0) continue;

		pthread_mutex_lock(&selectMutex);
		result = select(maxSock, &connMg->recvSet, &connMg->sendSet, NULL, &tv);
		if (result < 0)
		{
			pthread_mutex_unlock(&selectMutex);
			continue;
		}
		pthread_mutex_unlock(&selectMutex);

		if (result == 0) continue;

		for (iHashTable::iIterator itr = begin; itr != end; itr++)
		{
			iConnection* cl = (iConnection*)itr->data;

			if (FD_ISSET(cl->socket, &connMg->recvSet))
			{
				result = recv(cl->socket, cl->buff, iCLIENT_BUFF_SIZE, 0);

				if (result > 0)
				{
					cl->recvBuff.forcingAppend(cl->buff, result);
				}
				else
				{
					char* ip = inet_ntoa(cl->addr.sin_addr);

					cl->method(cl, "__conloose__", 20);

					delete cl;

					connMg->clients.remove(ip);
					continue;
				}
			}

			if (cl->recvBuff.len != 0)
			{
				if (cl->recvBuff.len > 2 &&
					cl->recvChunkLen == 0)
				{
					uint8* ml = (uint8*)&cl->recvChunkLen;

					for (int i = 0; i < 2; i++)
					{
						ml[1 - i] = cl->recvBuff[i];
					}

					cl->recvBuff.erase(0, 2);
				}

				if (cl->recvChunkLen != 0 &&
					cl->recvBuff.len >= cl->recvChunkLen)
				{
					int len = cl->recvChunkLen;

					char* msg = new char[len + 1];
					memcpy(msg, cl->recvBuff.str, sizeof(char) * len);
					msg[len] = 0;

					cl->method(cl, msg, len);

					delete[] msg;

					cl->recvBuff.erase(0, len);
					cl->recvChunkLen = 0;
				}
			}

			pthread_mutex_lock(&cl->mutex);
			if (cl->sendBuff.len != 0)
			{
				if (FD_ISSET(cl->socket, &connMg->sendSet))
				{
					result = send(cl->socket, cl->sendBuff.str,
						cl->sendBuff.len, 0);

					if (result > 0)
					{
						cl->sendBuff.erase(0, result);
					}
					else
					{
						char* ip = inet_ntoa(cl->addr.sin_addr);

						pthread_mutex_unlock(&cl->mutex);
						cl->method(cl, "__conloose__", 20);
						pthread_mutex_lock(&cl->mutex);

						delete cl;

						connMg->clients.remove(ip);
					}
				}
			}
			pthread_mutex_unlock(&cl->mutex);
		}
	}

	iHashTable::iIterator begin = connMg->clients.begin();
	iHashTable::iIterator end = connMg->clients.end();

	for (iHashTable::iIterator itr = begin; itr != end; itr++)
	{
		iConnection* cl = (iConnection*)itr->data;
		delete cl;
	}

	connMg->sign = iCONNECTIONMG_READY_TO_DEAD_SIGN;

	return NULL;
}

iConnection::iConnection(uint64& sock, sockaddr_in& ad)
{
	socket = sock;
	addr = ad;
	pthread_mutex_init(&mutex, NULL);
	recvBuff.resize(iCLIENT_BUFF_SIZE);
	recvChunkLen = 0;
	sendBuff.resize(iCLIENT_BUFF_SIZE);
	method = NULL;
}

iConnection::~iConnection()
{
	closeSocket(socket);
	pthread_mutex_destroy(&mutex);
}

bool iConnection::sendMsg(const char* m, int size)
{
	uint16 msgLen = size;

	if (sendBuff.len + msgLen >= iCLIENT_BUFF_SIZE) return false;

	uint32 totalLen = msgLen + 2;

	char* msg = new char[totalLen];

	for (int i = 1; i > -1; i--)
	{
		uint8 c = ((uint8*)&msgLen)[i];
		msg[1 - i] = c;
	}

	memcpy(&msg[2], m, sizeof(char) * msgLen);

	pthread_mutex_lock(&mutex);
	sendBuff.forcingAppend(msg, totalLen);
	pthread_mutex_unlock(&mutex);

	delete[] msg;

	return true;
}

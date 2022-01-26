#include "iNetwork.h"
#include "iStd.h"

pthread_mutex_t selectMutex = NULL;

void loadNetwork()
{
#ifdef _WIN32
	loadWSA();
#endif

	pthread_mutex_init(&selectMutex, NULL);
}

void endNetwork()
{
#ifdef _WIN32
	endWSA();
#endif

	pthread_mutex_destroy(&selectMutex);
}

int createSocket(const char* si, uint16 sp)
{
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock < 0)
	{
		return -1;
	}

	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(si);
	addr.sin_port = htons(sp);

	int funcResult = bind(sock, (sockaddr*)&addr, sizeof(addr));

	if (funcResult != 0)
	{
		closeSocket(sock);
		return -1;
	}

	return sock;
}

bool setSockBlockingStatus(long long socket, bool blocking)
{
	if (socket < 0) return false;

#ifdef __unix__
	int flags = fcntl(socket, F_GETFL, 0);
	flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
	return !fcntl(fd, F_SETFL, flags);

#elif _WIN32
	unsigned long mode = blocking ? 0 : 1;
	return !ioctlsocket(socket, FIONBIO, &mode);
#endif
}

void closeSocket(uint64 socket)
{
#ifdef __unix__
	close(servSock);
#elif _WIN32
	closesocket(socket);
#endif
}

char* getIpByDomainName(const char* domain)
{
	in_addr ia;
	hostent* info = gethostbyname(domain);
	char* addr = info->h_addr_list[0];
	memcpy(&ia, addr, sizeof(char) * info->h_length);

	char* ip = inet_ntoa(ia);

	int len = strlen(ip);
	char* r = new char[len + 1];
	memcpy(r, ip, sizeof(char) * len);
	r[len] = 0;

	return r;
}

bool isend(uint64 socket, const char* m)
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

	int off = 0;

	while (off < totalLen)
	{
		int funcResult = send(socket, &msg[off], totalLen - off, 0);

		if (funcResult < 0)
		{
			delete[] msg;
			return false;
		}

		off += funcResult;
	}

	delete[] msg;

	return true;
}

char* irecv(uint64 socket)
{
	int off = 0;
	char* result = NULL;

	int recvResult = -1;
	char buff[RECV_BUFF];
	uint16 msgLen = 0;

	while (!result)
	{
		recvResult = recv(socket, buff, sizeof(buff), 0);

		if (recvResult > 0)
		{
			for (int i = 0; i < recvResult; i++)
			{
				uint8* ml = (uint8*)&msgLen;
				ml[1 - off] = buff[i];
				off++;

				if (off >= 2)
				{
					off = 0;
					result = new char[msgLen + 1];

					for (int j = i + 1; j < recvResult; j++)
					{
						result[off++] = buff[j];
					}

					break;
				}
			}
		}
		else
		{
			if (result) delete[] result;
			return result;
		}
	}

	while (off != msgLen)
	{
		recvResult = recv(socket, buff, sizeof(buff), 0);

		if (recvResult > 0)
		{
			for (int i = 0; i < recvResult; i++)
			{
				result[off++] = buff[i];
			}
		}
		else
		{
			if (result) delete[] result;
			return result;
		}
	}

	result[off] = 0;

	return result;
}






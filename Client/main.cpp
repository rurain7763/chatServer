#include "iStd.h"

#define IP ""
#define PORT 9600

#ifndef IP 
#error define IP as char[]
#endif

#ifndef PORT
#error define PORT as int
#endif

bool run = true;
iConnection* echoServer;
pthread_mutex_t mutex;

void* input(void* v);
void serverRespon(iConnection* con, const char* msg, int len);

int main()
{
	loadNetwork();
	pthread_mutex_init(&mutex, NULL);

	iThreadPool* pool = iThreadPool::share();
	iConnectionManager* cm = iConnectionManager::share();

	echoServer = cm->connectByIp(IP, PORT, serverRespon);
	pool->addJob(input, NULL);
	echoServer->sendMsg("hello!", strlen("hello"));

	while (run)
	{
		pool->update();
	}

	pthread_mutex_destroy(&mutex);
	delete cm;
	delete pool;
	endNetwork();

	system("pause");
}

void serverRespon(iConnection* con, const char* msg, int len)
{
	if (CONNECTION_LOOSE_MSG(msg))
	{
		pthread_mutex_lock(&mutex);
		echoServer = NULL;
		pthread_mutex_unlock(&mutex);

		printf("connection loose :(\n");
	}
	else
	{
		printf("%s\n", msg);
	}
}

void* input(void* v)
{
	char re[100];
	memset(re, 0, sizeof(char) * 100);

	while (re[0] != 'q')
	{
		char buff[100];

		scanf("%s", buff);

		int len = strlen(buff);
		memcpy(re, buff, sizeof(char) * len);
		re[len] = 0;

		pthread_mutex_lock(&mutex);
		if (echoServer)
		{
			echoServer->sendMsg(re, len);
		}
		pthread_mutex_unlock(&mutex);
	}
		
	run = false;

	return NULL;
}




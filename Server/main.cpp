#include "iStd.h"

#define IP ""
#define PORT 9600

#ifndef IP 
#error define IP as char[]
#endif

#ifndef PORT
#error define PORT as int
#endif

class iEchoServer : public iServer
{
public:
	iEchoServer(const char* ip, uint16 port) : iServer(ip, port) {}

	virtual void eventUserRequest(iServerUser* user, const char* msg, int len)
	{
		iString sm = "From Server : ";
		sm += msg;

		sendMsgToUser(user, sm.str, sm.len);
	}

	virtual void eventError(iServErrCode code)
	{

	}
};

bool run = true;
uint32 count = 0;

void* input(void* pool);

int main()
{
	loadNetwork();

	iThreadPool* pool = iThreadPool::share();

	iEchoServer serv(IP, PORT);
	pool->addJob(iServer::run, &serv);
	pool->addJob(input, NULL);

	while (run)
	{
		pool->update();
	}

	serv.shutDown();
	delete pool;

	endNetwork();

	system("pause");
}

void* input(void* v)
{
	char stop[2];
	scanf("%s", stop);

	run = false;

	return NULL;
}


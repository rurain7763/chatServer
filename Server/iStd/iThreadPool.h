#pragma once

#ifdef _WIN32
#define HAVE_STRUCT_TIMESPEC
#endif
#include "pthread.h"
#pragma comment(lib, "pthreadVC2.lib")

#include "iType.h"
#include "iQueue.h"
#include "iArray.h"

#define MAX_THREAD_NUM 10 //_POSIX_THREAD_THREADS_MAX
#define JOB_QUEUE_SIZE 50

#define WORKER_STATUS_CREATING -1
#define WORKER_STATUS_WAIT		0
#define WORKER_STATUS_WORKING	1

typedef void* (*ThreadJobMethodNoArg)();
typedef void* (*ThreadJobMethodOneArg)(void*);
typedef void* (*ThreadJobMethodTwoArg)(void*, void*);

typedef void (*ThreadJobMethodCallBack)(void* result);

struct iThreadInfo;

class iThreadPool
{
private:
	static iThreadPool* S;
	iThreadPool();

public:
	~iThreadPool();
	static iThreadPool* share();

	void addJob(ThreadJobMethodNoArg method, 
				ThreadJobMethodCallBack callBack = NULL);
	void addJob(ThreadJobMethodOneArg method, void* arg, 
				ThreadJobMethodCallBack callBack = NULL);
	void addJob(ThreadJobMethodTwoArg method, void* arg1, void* arg2, 
				ThreadJobMethodCallBack callBack = NULL);

	bool isAllWorkDone();

	void update();

private:
	iThreadInfo* worker;
	int workerNum;

	iQueue job;
	pthread_mutex_t mutex;
};

enum iThreadJobFlag
{
	iThreadJobFlagNoArg = 0,
	iThreadJobFlagOneArg,
	iThreadJobFlagTwoArg,
	iThreadJobFlagMax
};

struct iJobInfo
{
	iThreadJobFlag flag;
	void* job;
	void* callBack;
	iArray* args;
	bool completeJob;
	void* jobResult;

	iJobInfo();
	~iJobInfo();
};

struct iThreadInfo
{
	uint32 status;

	uint32 threadID;
	iJobInfo* jobInfo;

	pthread_t thread;

	pthread_mutex_t condMutex;
	pthread_cond_t cond;
};





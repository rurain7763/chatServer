#include"iThreadPool.h"
#include"iStd.h"

iThreadPool* iThreadPool::S = NULL;

void* work(void* info)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	iThreadInfo* threadInfo = (iThreadInfo*)info;

	while (1)
	{
		pthread_mutex_lock(&threadInfo->condMutex);
		threadInfo->status = WORKER_STATUS_WAIT;
		pthread_cond_wait(&threadInfo->cond, &threadInfo->condMutex);
		pthread_mutex_unlock(&threadInfo->condMutex);
		
		iJobInfo* jobInfo = threadInfo->jobInfo;

		void* result = NULL;

		if (jobInfo->flag == iThreadJobFlagNoArg)
		{
			result = ((ThreadJobMethodNoArg)jobInfo->job)();
		}
		else if (jobInfo->flag == iThreadJobFlagOneArg)
		{
			result = ((ThreadJobMethodOneArg)jobInfo->job)(jobInfo->args->at(0));
		}
		else if (jobInfo->flag == iThreadJobFlagTwoArg)
		{
			result = ((ThreadJobMethodTwoArg)jobInfo->job)(jobInfo->args->at(0), jobInfo->args->at(1));
		}

		jobInfo->completeJob = true;
		jobInfo->jobResult = result;
	}

	return NULL;
}

iThreadPool::iThreadPool()
	:job(JOB_QUEUE_SIZE)
{
	workerNum = MAX_THREAD_NUM;
	worker = new iThreadInfo[workerNum];

	for (int i = 0; i < workerNum; i++)
	{
		iThreadInfo* info = &worker[i];

		info->status = WORKER_STATUS_CREATING;
		info->threadID = i;
		info->jobInfo = NULL;
		
		pthread_mutex_init(&info->condMutex, NULL);
		pthread_cond_init(&info->cond, NULL);

		pthread_create(&info->thread, NULL, work, (void*)info);

		while (info->status != WORKER_STATUS_WAIT);
	}

	pthread_mutex_init(&mutex, NULL);
}

iThreadPool::~iThreadPool()
{
	for (int i = 0; i < workerNum; i++)
	{
		iThreadInfo* info = &worker[i];

		int err = -1;
		while (err)
		{
			printf("thread %d will be destroy\n", info->threadID);
			err = pthread_cancel(info->thread);
		}

		printf("thread %d was destroyed\n", info->threadID);

		pthread_mutex_destroy(&info->condMutex);
		pthread_cond_destroy(&info->cond);

		if (info->jobInfo) delete info->jobInfo;
	}

	delete[] worker;

	while(!job.empty())
	{
		iJobInfo* info = (iJobInfo*)job.pop();
		delete info->args;
		delete info;
	}

	pthread_mutex_destroy(&mutex);
}

iThreadPool* iThreadPool::share()
{
	if (!S) S = new iThreadPool();
	return S;
}

void iThreadPool::addJob(ThreadJobMethodNoArg method, ThreadJobMethodCallBack callBack)
{
	if (job.num == job.size)
	{
		printf("your order was skipped \
				because job queue size is small than your orders. \
				increase your queue size.\n");

		return;
	}

	iJobInfo* info = new iJobInfo;
	info->flag = iThreadJobFlagNoArg;
	info->args = NULL;
	info->job = method;
	info->callBack = callBack;
	info->completeJob = false;
	info->jobResult = NULL;

	pthread_mutex_lock(&mutex);
	job.push(info);
	pthread_mutex_unlock(&mutex);
}

void iThreadPool::addJob(ThreadJobMethodOneArg method, void* arg,
						 ThreadJobMethodCallBack callBack)
{
	if (job.num == job.size)
	{
		printf("your order was skipped \
				because job queue size is small than your orders. \
				increase your queue size.\n");

		return;
	}

	iJobInfo* info = new iJobInfo;
	info->flag = iThreadJobFlagOneArg;
	info->job = method;
	info->args = new iArray(1);
	info->args->push_back(arg);
	info->callBack = callBack;
	info->completeJob = false;
	info->jobResult = NULL;

	pthread_mutex_lock(&mutex);
	job.push(info);
	pthread_mutex_unlock(&mutex);
}

void iThreadPool::addJob(ThreadJobMethodTwoArg method, void* arg1, void* arg2,
						 ThreadJobMethodCallBack callBack)
{

	if (job.num == job.size)
	{
		printf("your order was skipped \
				because job queue size is small than your orders. \
				increase your queue size.\n");

		return;
	}

	iJobInfo* info = new iJobInfo;
	info->flag = iThreadJobFlagTwoArg;
	info->job = method;
	info->args = new iArray(2);
	info->args->push_back(arg1);
	info->args->push_back(arg2);
	info->callBack = callBack;
	info->completeJob = false;
	info->jobResult = NULL;

	pthread_mutex_lock(&mutex);
	job.push(info);
	pthread_mutex_unlock(&mutex);
}

bool iThreadPool::isAllWorkDone()
{
	for (int i = 0; i < workerNum; i++)
	{
		iThreadInfo* info = &worker[i];

		if (info->status == WORKER_STATUS_WORKING) return false;
	}

	return true;
}

void iThreadPool::update()
{
	for (int i = 0; i < workerNum; i++)
	{
		iThreadInfo* info = &worker[i];
		iJobInfo* jobInfo = info->jobInfo;

		if (jobInfo && jobInfo->completeJob)
		{
			if(jobInfo->callBack)
			{
				((ThreadJobMethodCallBack)jobInfo->callBack)(jobInfo->jobResult);
			}

			delete info->jobInfo;
			info->jobInfo = NULL;
		}

		if (!job.empty() && info->status == WORKER_STATUS_WAIT)
		{
			info->status = WORKER_STATUS_WORKING;
			info->jobInfo = (iJobInfo*)job.pop();

			pthread_mutex_lock(&info->condMutex);
			pthread_cond_signal(&info->cond);
			pthread_mutex_unlock(&info->condMutex);
		}
	}
}

iJobInfo::iJobInfo()
{
	flag = iThreadJobFlagMax;
	job = NULL;
	callBack = NULL;
	args = NULL;
	completeJob = false;
	jobResult = NULL;
}

iJobInfo::~iJobInfo()
{
	if (args) delete args;
}


#pragma once

struct iQueueData;

class iQueue
{
public:
	iQueue(int size);
	virtual ~iQueue();

	void push(void* v);
	void* pop();
	bool empty();

private:
	iQueueData* data;
	int front;
	int back;

public:
	int num;
	int size;
};

struct iQueueData
{
	void* data;
};
#include "iQueue.h"
#include "iStd.h"

iQueue::iQueue(int s)
{
	data = new iQueueData[s];
	size = s;
	front = 0;
	back = 0;
	num = 0;
}

iQueue::~iQueue()
{
	delete[] data;
}

void iQueue::push(void* v)
{
	if (num == size) return;

	data[back].data = v;
	back = (back + 1) % size;
	num++;
}

void* iQueue::pop()
{
	if (num == 0) return NULL;

	void* r = data[front].data;
	front = (front + 1) % size;
	num--;

	return r;
}

bool iQueue::empty()
{
	return num == 0;
}

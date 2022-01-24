#include "iStack.h"
#include "iStd.h"

iStack::iStack(int s)
{
	data = new iStackData[s];
	top = 0;
	size = s;
}

iStack::~iStack()
{
	delete[] data;
}

void iStack::push(void* v)
{
	if (top >= size) return;

	data[top].data = v;
	top++;
}

void* iStack::topData()
{
	if (top == 0) return NULL;

	return data[top - 1].data;
}

void* iStack::pop()
{
	if (top == 0) return NULL;

	void* r = data[top - 1].data;
	top--;

	return r;
}

bool iStack::empty()
{
	return top == 0;
}

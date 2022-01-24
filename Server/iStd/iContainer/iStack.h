#pragma once

struct iStackData;

class iStack
{
public:
	iStack(int size);
	virtual ~iStack();

	void push(void* v);
	void* topData();
	void* pop();
	bool empty();

private:
	iStackData* data;
	int top;

public:
	int size;
};

struct iStackData
{
	void* data;
};
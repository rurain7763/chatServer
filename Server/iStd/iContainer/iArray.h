#pragma once

#define DEFAULT_ARRAY_SIZE			    20
#define DUMY_DATA			0xffffffff / 2

struct iArrayData;

class iArray
{
public:
	iArray();
	iArray(int size);
	iArray(const iArray& arr);
	virtual ~iArray();

	iArray& operator=(const iArray& arr);

	void insert(int idx, void* data);
	void erase(int idx);
	void push_back(void* data);
	void* pop_back();
	void resize(int size);
	void*& at(int idx);

	void*& operator[](int idx);

private:
	unsigned int dumy;

public:
	int size;
	iArrayData* data;
	int dataNum;
};

struct iArrayData
{
	void* data;
};




#include"iArray.h"
#include"iStd.h"

iArray::iArray()
{
	data = new iArrayData[DEFAULT_ARRAY_SIZE];
	size = DEFAULT_ARRAY_SIZE;
	dataNum = 0;
	dumy = DUMY_DATA;
}

iArray::iArray(int s)
{
	data = new iArrayData[s];
	size = s;
	dataNum = 0;
	dumy = DUMY_DATA;
}

iArray::iArray(const iArray& arr)
{
	size = arr.size;
	data = new iArrayData[size];
	dataNum = arr.dataNum;
	memcpy(data, arr.data, sizeof(iArrayData) * dataNum);
	dumy = DUMY_DATA;
}

iArray::~iArray()
{
	delete[] data;
}

iArray& iArray::operator=(const iArray& arr)
{
	delete[] data;

	size = arr.size;
	data = new iArrayData[size];
	dataNum = arr.dataNum;
	memcpy(data, arr.data, sizeof(iArrayData) * dataNum);
	dumy = DUMY_DATA;

	return *this;
}

void iArray::insert(int idx, void* d)
{
	if (dataNum == idx)
	{
		push_back(d);
	}
	else if(dataNum < idx)
	{
		if (idx >= size) resize(idx * 2);

		for (int i = dataNum; i <= idx; i++)
			data[i].data = &dumy;
		
		data[idx].data = d;
		dataNum += idx - dataNum + 1;
	}
	else
	{
		if (dataNum + 1 > size) resize(size * 2);

		for (int i = dataNum - 1; i >= idx; i--)
			data[i + 1] = data[i];
		
		data[idx].data = d;
		dataNum++;
	}
}

void iArray::erase(int idx)
{
	if (dataNum == idx)
	{
		pop_back();
	}
	else if(dataNum > idx)
	{
		dataNum--;
		for (int i = idx; i < dataNum; i++)
			data[i] = data[i + 1];
	}
}

void iArray::push_back(void* d)
{
	if (dataNum >= size) resize(size * 2);

	data[dataNum].data = d;
	dataNum++;
}

void* iArray::pop_back()
{
	if (dataNum < 1) return NULL;

	dataNum--;
	return data[dataNum].data;
}

void iArray::resize(int s)
{
	iArrayData* copy = new iArrayData[s];

	int num = min(dataNum, s);
	memcpy(copy, data, sizeof(iArrayData) * num);
	delete[] data;
	data = copy;

	size = s;
	dataNum = num;
}

void*& iArray::at(int idx)
{
	if (idx >= size)
	{
		resize(idx * 2);

		for (int i = dataNum; i <= idx; i++)
			data[i].data = &dumy;

		dataNum += idx - dataNum + 1;
	}

	return data[idx].data;
}

void*& iArray::operator[](int idx)
{
	if (idx >= size)
	{
		resize(idx * 2);

		for (int i = dataNum; i <= idx; i++)
			data[i].data = &dumy;

		dataNum += idx - dataNum + 1;
	}

	return data[idx].data;
}

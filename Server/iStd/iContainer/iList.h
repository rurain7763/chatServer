#pragma once

#include "iType.h"

struct iListNode;

class iList
{
public:
	class iIterator
	{
	public:
		iIterator();
		iListNode operator*() const;
		iListNode* operator->();
		iList::iIterator& operator++(int n);
		iList::iIterator& operator=(iList::iIterator itr);

	public:
		uint32 currPos;
		iListNode* prev;
		iListNode* ptr;
	};

public:
	iList();
	virtual ~iList();

	void clear();
	void insert(int idx, void* data);
	void erase(int idx);
	void erase(iList::iIterator& itr);
	void push_back(void* data);
	void push_front(void* data);

	void*& operator[](int idx);

	iList::iIterator begin();
	iList::iIterator end();

private:
	iListNode* head;

public:
	int num;
};

struct iListNode
{
	void* data;
	iListNode* next;
};

bool operator!=(const iList::iIterator& itr1, const iList::iIterator& itr2);
bool operator==(const iList::iIterator& itr1, const iList::iIterator& itr2);

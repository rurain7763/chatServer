#include"iList.h"
#include"iStd.h"

iList::iList()
{
	head = NULL;
	num = 0;
}

iList::~iList()
{
	iListNode* node = head;

	while (node)
	{
		iListNode* next = node->next;
		delete node;
		node = next;
	}
}

void iList::clear()
{
	iListNode* node = head;

	while (node)
	{
		iListNode* next = node->next;
		delete node;
		node = next;
	}

	head = NULL;
	num = 0;
}

void iList::insert(int idx, void* data)
{
	if (idx > num - 1)
	{
		push_back(data);
	}
	else
	{
		iListNode* prev = head;
		int prevIdx = idx - 1;

		for (int i = 0; i < prevIdx; i++)
		{
			prev = prev->next;
		}

		if (prevIdx == -1)
		{
			iListNode* newNode = new iListNode;
			newNode->data = data;
			newNode->next = head;
			head = newNode;
		}
		else
		{
			iListNode* newNode = new iListNode;
			newNode->data = data;
			newNode->next = prev->next;
			prev->next = newNode;
		}	
	}

	num++;
}

void iList::erase(int idx)
{
	if (idx > num - 1) idx = num - 1;

	iListNode* prev = head;
	int prevIdx = idx - 1;

	for (int i = 0; i < prevIdx; i++)
	{
		prev = prev->next;
	}

	if (prevIdx == -1)
	{
		iListNode* newHead = head->next;
		delete head;
		head = newHead;
	}
	else
	{
		iListNode* targetNode = prev->next;
		prev->next = targetNode->next;
		delete targetNode;
	}

	num--;
}

void iList::erase(iList::iIterator& itr)
{
	if (itr.currPos == -1) return;

	iListNode* node = itr->next;

	erase(itr.currPos);

	itr.ptr = node;

	if (itr.currPos >= num) itr.currPos = -1;
}

void iList::push_back(void* data)
{
	if (head)
	{
		iListNode* node = head;

		while (node->next)
		{
			node = node->next;
		}

		iListNode* newNode = new iListNode;
		newNode->data = data;
		newNode->next = NULL;
		node->next = newNode;
	}
	else
	{
		iListNode* newNode = new iListNode;
		newNode->data = data;
		newNode->next = NULL;
		head = newNode;
	}

	num++;
}

void iList::push_front(void* data)
{
	iListNode* newNode = new iListNode;
	newNode->data = data;
	newNode->next = head;
	head = newNode;

	num++;
}

void*& iList::operator[](int idx)
{
	void* dummy = NULL;

	if (idx > num - 1) return dummy;

	iListNode* node = head;

	for (int i = 0; i < idx; i++)
	{
		node = node->next;
	}

	return node->data;
}

iList::iIterator::iIterator()
{
	prev = NULL;
	currPos = -1;
	ptr = NULL;
}

iListNode iList::iIterator::operator*() const
{
	return *ptr;
}

iListNode* iList::iIterator::operator->()
{
	return ptr;
}

iList::iIterator& iList::iIterator::operator++(int n)
{
	if (!ptr) return *this;

	if (!ptr->next)
	{
		prev = ptr;
		currPos = -1;
		ptr = NULL;
	}
	else
	{
		prev = ptr;
		currPos++;
		ptr = ptr->next;
	}
	
	return *this;
}

iList::iIterator& iList::iIterator::operator=(iList::iIterator itr)
{
	prev = itr.prev;
	currPos = itr.currPos;
	ptr = itr.ptr;

	return *this;
}

iList::iIterator iList::begin()
{
	iList::iIterator r;

	if (num != 0)
	{
		r.prev = NULL;
		r.ptr = head;
		r.currPos = 0;
	}
	
	return r;
}

iList::iIterator iList::end()
{
	iList::iIterator r;

	if (num != 0)
	{
		iListNode* node = head;

		while (node->next)
		{
			node = node->next;
		}

		r.prev = node;
		r.ptr = NULL;
		r.currPos = -1;
	}
	
	return r;
}

bool operator!=(const iList::iIterator& itr1, const iList::iIterator& itr2)
{
	return itr1.ptr != itr2.ptr;
}

bool operator==(const iList::iIterator& itr1, const iList::iIterator& itr2)
{
	return itr1.ptr == itr2.ptr;
}
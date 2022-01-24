#pragma once

#define DEFAULT_HASHTABLE_SIZE	200

#define LINEAR_PROBING		0
#define QUADRATIC_PROBING	1
#define DOUBLE_PROBING		2
#define HASHTABLE_PROBING	QUADRATIC_PROBING

#include "iType.h"

struct iBucket;

class iHashTable
{
public:
	iHashTable();
	iHashTable(int size);
	virtual ~iHashTable();

	void insert(const char* key, void* data);
	void* at(const char* key);
	void remove(const char* key);

	void*& operator[](const char* key);

	class iIterator
	{
	public:
		iIterator();
		iBucket operator*() const;
		iBucket* operator->();
		iHashTable::iIterator& operator++(int n);
		iHashTable::iIterator& operator=(iHashTable::iIterator itr);

	public:
		uint32 currPos;
		uint32 size;
		iBucket* ptr;
	};

	iHashTable::iIterator begin();
	iHashTable::iIterator end();

private:
	void resize();

	uint64 hashFunc1(const char* key);
	uint64 hashFunc2(const char* key);

	iBucket* linearProbing(uint64 hash);
	iBucket* quadraticProbing(uint64 hash);
	iBucket* doubleHashing(uint64 hash);

private:
	static char dummy[6];
	iBucket* data;
	int dummyNum;

public:
	int size;
	int num;
};

struct iBucket
{
	uint64 hash;
	void* data;
};

bool operator!=(const iHashTable::iIterator& itr1, const iHashTable::iIterator& itr2);
bool operator==(const iHashTable::iIterator& itr1, const iHashTable::iIterator& itr2);
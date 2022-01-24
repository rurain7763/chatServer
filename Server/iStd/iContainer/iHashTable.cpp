#include "iHashTable.h"
#include "iStd.h"

char iHashTable::dummy[6];

iHashTable::iHashTable()
{
	uint32 prime = nextPrime(DEFAULT_HASHTABLE_SIZE);

	strcpy(dummy, "DUMMY");
	data = new iBucket[prime];
	dummyNum = 0;
	memset(data, 0, sizeof(iBucket) * prime);
	size = prime;
	num = 0;
}

iHashTable::iHashTable(int s)
{
	uint32 prime = nextPrime(s);

	strcpy(dummy, "DUMMY");
	data = new iBucket[prime];
	dummyNum = 0;
	memset(data, 0, sizeof(iBucket) * prime);
	size = prime;
	num = 0;
}

iHashTable::~iHashTable()
{
	delete[] data;
}

void iHashTable::insert(const char* key, void* d)
{
	uint64 hash = hashFunc1(key);

#if HASHTABLE_PROBING == LINEAR_PROBING
	iBucket* b = linearProbing(hash);
	b->hash = hash;
	b->data = d;
#elif HASHTABLE_PROBING == QUADRATIC_PROBING
	iBucket* b = quadraticProbing(hash);
	b->hash = hash;
	b->data = d;
#elif HASHTABLE_PROBING == DOUBLE_PROBING
	iBucket* b = doubleHashing(hash);
	b->hash = hash;
	b->data = d;
#endif

	num++;
	if (num + dummyNum >= size * .7f) resize();
}

void* iHashTable::at(const char* key)
{
	if (num == 0) return NULL;

	uint64 hash = hashFunc1(key);

#if HASHTABLE_PROBING == LINEAR_PROBING
	iBucket* b = linearProbing(hash);
	return b->data;
#elif HASHTABLE_PROBING == QUADRATIC_PROBING
	iBucket* b = quadraticProbing(hash);
	return b->data;
#elif HASHTABLE_PROBING == DOUBLE_PROBING
	iBucket* b = doubleSearch(hash);
	return b->data;
#endif
}

void iHashTable::remove(const char* key)
{
	if (num == 0) return;

	uint64 hash = hashFunc1(key);

#if HASHTABLE_PROBING == LINEAR_PROBING
	iBucket* b = linearProbing(hash);
#elif HASHTABLE_PROBING == QUADRATIC_PROBING
	iBucket* b = quadraticProbing(hash);
#elif HASHTABLE_PROBING == DOUBLE_PROBING
	iBucket* b = doubleSearch(key);
#endif

	b->data = dummy;
	dummyNum++;
	num--;

	if ( num + dummyNum >= size * .7f ) resize();
}

void*& iHashTable::operator[](const char* key)
{
	uint64 hash = hashFunc1(key);

#if HASHTABLE_PROBING == LINEAR_PROBING
	iBucket* b = linearProbing(hash);
	return b->data;
#elif HASHTABLE_PROBING == QUADRATIC_PROBING
	iBucket* b = quadraticProbing(hash);
	return b->data;
#elif HASHTABLE_PROBING == DOUBLE_PROBING
	iBucket* b = doubleHashing(hash);
	return b->data;
#endif
}

iHashTable::iIterator::iIterator()
{
	size = 0;
	currPos = -1;
	ptr = NULL;
}

iBucket iHashTable::iIterator::operator*() const
{
	return ptr[currPos];
}

iBucket* iHashTable::iIterator::operator->()
{
	return &ptr[currPos];
}

iHashTable::iIterator& iHashTable::iIterator::operator++(int n)
{
	uint32 start = currPos + 1;
	currPos = -1;

	for (uint32 i = start; i < size; i++)
	{
		if (ptr[i].data && ptr[i].data != dummy)
		{
			currPos = i;
			break;
		}		
	}

	return *this;
}

iHashTable::iIterator& iHashTable::iIterator::operator=(iHashTable::iIterator itr)
{
	size = itr.size;
	currPos = itr.currPos;
	ptr = itr.ptr;

	return *this;
}

iHashTable::iIterator iHashTable::begin()
{
	iHashTable::iIterator r;
	r.size = size;
	r.ptr = data;
	r.currPos = -1;

	for (int i = 0; i < size; i++)
	{
		if (data[i].data && data[i].data != dummy)
		{
			r.currPos = i;
			break;
		}
	}

	return r;
}

iHashTable::iIterator iHashTable::end()
{
	iHashTable::iIterator r;
	r.size = size;
	r.ptr = data;
	r.currPos = -1;

	return r;
}

void iHashTable::resize()
{
	uint32 prime = nextPrime(size);
	iBucket* copy = new iBucket[prime];
	memset(copy, 0, sizeof(iBucket) * prime);
	num = 0;

	for (int i = 0; i < size; i++)
	{
		iBucket* from = &data[i];

		if (from->hash == 0 || from->data == dummy) continue;

#if HASHTABLE_PROBING == QUADRATIC_PROBING
		int idx = from->hash % prime;
		
		iBucket* to = &copy[idx];
		int step = 1;

		while (to->data != NULL)
		{
			idx = (idx + step * step) % size;
			step++;
			to = &copy[idx];
		}

		to->hash = from->hash;
		to->data = from->data;
		num++;
#endif
	}

	delete[] data;
	data = copy;
	size = prime;
	dummyNum = 0;
}

uint64 iHashTable::hashFunc1(const char* key)
{
	//horner's method => when x = 31
	uint64 h = 0;
	int len = strlen(key);

	for (int i = 0; i < len; i++)
		h = ((h << 5) - h) + key[i];
	
	return h;
}

uint64 iHashTable::hashFunc2(const char* key)
{
	//TODO : make double hashing
	int v = 0;

	return v;
}

iBucket* iHashTable::linearProbing(uint64 hash)
{
	uint64 idx = hash % size;
	
	iBucket* b = &data[idx];

	while(b->data != NULL)
	{
		if (b->hash == hash && b->data != dummy) break;

		idx = (idx + 1) % size;
		b = &data[idx];
	}

	return b;
}

iBucket* iHashTable::quadraticProbing(uint64 hash)
{
	uint32 idx = hash % size;

	iBucket* b = &data[idx];
	uint32 step = 1;

	while (b->data != NULL && step <= size)
	{
		if (b->hash == hash && b->data != dummy) break;

		idx = (idx + step * step) % size;
		step++;

		b = &data[idx];
	}

	return b;
}

iBucket* iHashTable::doubleHashing(uint64 hash)
{
	//TODO : make double hashing
	return NULL;
}

bool operator!=(const iHashTable::iIterator& itr1, 
				const iHashTable::iIterator& itr2)
{
	iBucket* left = itr1.ptr;
	int leftIdx = itr1.currPos;

	iBucket* right = itr2.ptr;
	int rightIdx = itr2.currPos;

	if (leftIdx == -1 || rightIdx == -1)
	{
		return (left != right) || (leftIdx != rightIdx);
	}

	return &left[leftIdx] != &right[rightIdx];
}

bool operator==(const iHashTable::iIterator& itr1, 
				const iHashTable::iIterator& itr2)
{
	iBucket* left = itr1.ptr;
	int leftIdx = itr1.currPos;

	iBucket* right = itr2.ptr;
	int rightIdx = itr2.currPos;

	if (leftIdx == -1 || rightIdx == -1)
	{
		return (left == right) && (leftIdx == rightIdx);
	}

	return &left[leftIdx] == &right[rightIdx];
}
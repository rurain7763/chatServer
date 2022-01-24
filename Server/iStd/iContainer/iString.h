#pragma once

#define DEFAULT_STR_SIZE 20

#include "iType.h"

class iString
{
public:
	iString();
	iString(uint32 size);
	iString(const iString& istr);
	iString(const char* str);
	iString(char c);
	virtual ~iString();

	iString& operator=(const iString& istr);
	iString& operator=(const char* str);
	iString& operator=(char c);

	iString& operator+=(const iString& istr);
	iString& operator+=(const char* str);
	iString& operator+=(char c);

	char& operator[](int idx);

	void resize(uint32 size);
	void clear();
	bool empty();
	void shrink_to_fit();
	char at(int idx) const;

	iString& append(const iString& istr, uint32 subPos = 0, uint32 subLen = 0);
	iString& append(const char* str, uint32 num = 1);
	iString& append(char c, uint32 num = 1);
	iString& forcingAppend(const char* str, uint32 len);

	iString& insert(uint32 pos, const iString& istr, uint32 subPos = 0, uint32 subLen = 0);
	iString& insert(uint32 pos, const char* str, uint32 num = 1);
	iString& insert(uint32 pos, char c, uint32 num = 1);

	iString& erase(uint32 pos = 0, uint32 len = 0);

	iString& replace(uint32 pos, uint32 len, const iString& istr, uint32 subPos = 0, uint32 subLen = 0);
	iString& replace(uint32 pos, uint32 len, const char* str, uint32 strLen = 0);
	iString& replace(uint32 pos, uint32 len, char c, uint32 num = 1);

	uint32 find(const iString& istr, uint32 pos = 0) const;
	uint32 find(const char* str, uint32 pos = 0) const;
	uint32 find(const char* str, uint32 pos, uint32 strLen) const;

	iString substr(uint32 pos = 0, uint32 len = 0) const;

public:
	char* str;
	int size;
	int len;
};

iString operator+(const iString& str1, const iString& str2);

bool operator==(const iString& str1, const iString& str2);
bool operator!=(const iString& str1, const iString& str2);

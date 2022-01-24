#include"iString.h"
#include"iStd.h"

iString::iString()
{
	str = new char[DEFAULT_STR_SIZE];
	str[0] = 0;
	len = 0;
	size = DEFAULT_STR_SIZE;
}

iString::iString(uint32 s)
{
	if (s == 0) s = DEFAULT_STR_SIZE;

	str = new char[s];
	str[0] = 0;
	len = 0;
	size = s;
}

iString::iString(const iString& istr)
{
	len = istr.len;
	size = len * 2 + 1;

	str = new char[size];
	memcpy(str, istr.str, sizeof(char) * len);
	str[len] = 0;
}

iString::iString(const char* s)
{
	if (!s)
	{
		len = 0;
		size = DEFAULT_STR_SIZE;
	}
	else
	{
		len = strlen(s);
		size = len * 2 + 1;
	}

	str = new char[size];
	memcpy(str, s, sizeof(char) * len);
	str[len] = 0;
}

iString::iString(char c)
{
	len = 1;
	size = 2;
	str = new char[size];
	str[0] = c;
	str[1] = 0;
}

iString::~iString()
{
	delete[] str;
}

iString& iString::operator=(const iString& istr)
{
	len = istr.len;

	if (len >= size)
	{
		delete[] str;
		size = len * 2 + 1;
		str = new char[size];
	}

	memcpy(str, istr.str, sizeof(char) * len);
	str[len] = 0;

	return *this;
}

iString& iString::operator=(const char* s)
{
	len = strlen(s);

	if (len >= size)
	{
		delete[] str;
		size = len * 2 + 1;
		str = new char[size];
	}

	memcpy(str, s, sizeof(char) * len);
	str[len] = 0;

	return *this;
}

iString& iString::operator=(char c)
{
	if (size < 2)
	{
		size = 2 * 2 + 1;
		delete[] str;
		str = new char[size];
	}

	str[0] = c;
	str[1] = 0;
	len = 1;
	
	return *this;
}

iString& iString::operator+=(const iString& istr)
{
	int newLen = len + istr.len;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	memcpy(&str[len], istr.str, sizeof(char) * istr.len);
	str[newLen] = 0;
	len = newLen;

	return *this;
}

iString& iString::operator+=(const char* s)
{
	int sLen = strlen(s);
	int newLen = len + sLen;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	memcpy(&str[len], s, sizeof(char) * sLen);
	str[newLen] = 0;
	len = newLen;

	return *this;
}

iString& iString::operator+=(char c)
{
	int newLen = len + 1;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	str[len] = c;
	str[newLen] = 0;
	len = newLen;

	return *this;
}

char& iString::operator[](int idx)
{
	if (idx >= len)
	{
		char* dummy = NULL;
		return *dummy;
	}

	return str[idx];
}

void iString::resize(uint32 s)
{
	if (s)
	{
		if (len >= s) len = s - 1;

		char* copy = new char[s];
		memcpy(copy, str, sizeof(char) * len);
		copy[len] = 0;
		delete[] str;
		str = copy;

		size = s;
	}
}

void iString::clear()
{
	memset(str, 0, sizeof(char) * len);
	len = 0;
}

bool iString::empty()
{
	return !len;
}

void iString::shrink_to_fit()
{
	size = len + 1;
	char* copy = new char[size];
	memcpy(copy, str, sizeof(char) * len);
	copy[len] = 0;
	delete[] str;
	str = copy;
}

char iString::at(int idx) const
{
	if (idx >= len)
	{
		return 0;
	}
	
	return str[idx];
}

iString& iString::append(const iString& istr, uint32 subPos, uint32 subLen)
{
	if (subPos >= istr.len || 
		subPos + subLen > istr.len) 
		return *this;

	int appendLen = subLen ? subLen : istr.len - subPos;
	int newLen = len + appendLen;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	memcpy(&str[len], &istr.str[subPos], sizeof(char) * appendLen);
	str[newLen] = 0;
	len = newLen;

	return *this;
}

iString& iString::append(const char* s, uint32 num)
{
	int sLen = strlen(s);
	int appendLen = sLen * num;
	int newLen = len + appendLen;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	for(int i = 0 ; i < num; i++) 
		memcpy(&str[len + sLen * i], s, sizeof(char) * sLen);
	str[newLen] = 0;
	len = newLen;

	return *this;
}

iString& iString::append(char c, uint32 num)
{
	int newLen = len + num;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	for (int i = 0; i < num; i++) str[len + i] = c;
	str[newLen] = 0;
	len = newLen;

	return *this;
}

iString& iString::forcingAppend(const char* s, uint32 l)
{
	int newLen = len + l;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	memcpy(&str[len], s, sizeof(char) * l);
	str[newLen] = 0;
	len = newLen;

	return *this;
}

iString& iString::insert(uint32 pos, const iString& istr, uint32 subPos, uint32 subLen)
{
	if (pos >= len ||
		subPos >= istr.len ||
		subPos + subLen > istr.len)
		return *this;

	int insertLen = subLen ? subLen : istr.len - subPos;
	int newLen = len + insertLen;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}
	
	int backBufLen = len - pos;
	char* backBuf = new char[backBufLen];
	memcpy(backBuf, &str[pos], sizeof(char) * backBufLen);

	memcpy(&str[pos], &istr.str[subPos], sizeof(char) * insertLen);
	memcpy(&str[pos + insertLen], backBuf, sizeof(char) * backBufLen);
	str[newLen] = 0;
	len = newLen;

	delete[] backBuf;

	return *this;
}

iString& iString::insert(uint32 pos, const char* s, uint32 num)
{
	if (pos >= len) return *this;

	int sLen = strlen(s);
	int insertLen = sLen * num;
	int newLen = len + insertLen;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	int backBufLen = len - pos;
	char* backBuf = new char[backBufLen];
	memcpy(backBuf, &str[pos], sizeof(char) * backBufLen);

	for (int i = 0; i < num; i++)
		memcpy(&str[pos + sLen * i], s, sizeof(char) * sLen);
	memcpy(&str[pos + insertLen], backBuf, sizeof(char) * backBufLen);
	str[newLen] = 0;
	len = newLen;

	delete[] backBuf;

	return *this;
}

iString& iString::insert(uint32 pos, char c, uint32 num)
{
	if (pos >= len) return *this;

	int newLen = len + num;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	int backBufLen = len - pos;
	char* backBuf = new char[backBufLen];
	memcpy(backBuf, &str[pos], sizeof(char) * backBufLen);

	for (int i = 0; i < num; i++) str[pos + i] = c;
	memcpy(&str[pos + num], backBuf, sizeof(char) * backBufLen);
	str[newLen] = 0;
	len = newLen;

	delete[] backBuf;

	return *this;
}

iString& iString::erase(uint32 pos, uint32 l)
{
	l = l ? l : len;

	if (pos >= len || pos + l > len) return *this;

	int newLen = len - l;
	int copyPos = pos + l;
	memcpy(&str[pos], &str[copyPos], sizeof(char) * (len - copyPos));
	str[newLen] = 0;
	len = newLen;

	return *this;
}

iString& iString::replace(uint32 pos, uint32 l, const iString& istr, uint32 subPos, uint32 subLen)
{
	if (pos >= len ||
		subPos >= istr.len ||
		subPos + subLen > istr.len)
		return *this;

	subLen = subLen ? subLen : istr.len - subPos;
	int newLen = (len - l) + subLen;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	int copyPos = pos + l;
	int backBufLen = len - copyPos;
	char* backBuf = new char[backBufLen];
	memcpy(backBuf, &str[copyPos], sizeof(char) * backBufLen);

	memcpy(&str[pos], &istr.str[subPos], sizeof(char) * subLen);
	copyPos = pos + subLen;
	memcpy(&str[copyPos], backBuf, sizeof(char) * backBufLen);
	str[newLen] = 0;
	len = newLen;

	delete[] backBuf;

	return *this;
}

iString& iString::replace(uint32 pos, uint32 l, const char* s, uint32 strLen)
{
	if (pos >= len) return *this;

	strLen = strLen ? strLen : strlen(s);
	int newLen = (len - l) + strLen;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	int copyPos = pos + l;
	int backBufLen = len - copyPos;
	char* backBuf = new char[backBufLen];
	memcpy(backBuf, &str[copyPos], sizeof(char) * backBufLen);

	memcpy(&str[pos], s, sizeof(char) * strLen);
	copyPos = pos + strLen;
	memcpy(&str[copyPos], backBuf, sizeof(char) * backBufLen);
	str[newLen] = 0;
	len = newLen;

	delete[] backBuf;

	return *this;
}

iString& iString::replace(uint32 pos, uint32 l, char c, uint32 num)
{
	if (pos >= len) return *this;

	int newLen = (len - l) + num;

	if (newLen >= size)
	{
		size = newLen * 2 + 1;
		char* copy = new char[size];
		memcpy(copy, str, sizeof(char) * len);
		delete[] str;
		str = copy;
	}

	int copyPos = pos + l;
	int backBufLen = len - copyPos;
	char* backBuf = new char[backBufLen];
	memcpy(backBuf, &str[copyPos], sizeof(char) * backBufLen);

	for (int i = 0; i < num; i++) str[pos + i] = c;
	copyPos = pos + num;
	memcpy(&str[copyPos], backBuf, sizeof(char) * backBufLen);
	str[newLen] = 0;
	len = newLen;

	delete[] backBuf;

	return *this;
}

uint32 iString::find(const iString& istr, uint32 pos) const
{
	if (pos >= istr.len ||
		istr.len == 0)
		return -1;

	for (int i = pos; i + istr.len <= len; i++)
	{
		if (str[i] == istr.str[0])
		{
			if (!memcmp(&str[i], istr.str, sizeof(char) * istr.len)) return i;
		}
	}

	return -1;
}

uint32 iString::find(const char* s, uint32 pos) const
{
	int l = strlen(s);

	if (pos >= len) return -1;

	for (int i = pos; i + l <= len; i++)
	{
		if (str[i] == s[0])
		{
			if (!memcmp(&str[i], s, sizeof(char) * l)) return i;
		}
	}

	return -1;
}

uint32 iString::find(const char* s, uint32 pos, uint32 strLen) const
{
	if (pos >= len) return -1;

	for (int i = pos; i + strLen <= len; i++)
	{
		if (str[i] == s[0])
		{
			if (!memcmp(&str[i], s, sizeof(char) * strLen)) return i;
		}
	}

	return -1;
}

iString iString::substr(uint32 pos, uint32 l) const
{
	if (pos >= len ||
		pos + l > len) 
		return iString();

	l = l ? l : len - pos;

	iString r;

	char* subStr = new char[l + 1];
	int end = pos + l;

	for (int i = pos; i < end; i++)
	{
		subStr[i - pos] = str[i];
	}

	subStr[l] = 0;
	r = subStr;
	delete[] subStr;

	return r;
}

iString operator+(const iString& str1, const iString& str2)
{
	iString r;

	int len = str1.len + str2.len;
	char* str = new char[len + 1];
	memcpy(str, str1.str, sizeof(char) * str1.len);
	memcpy(&str[str1.len], str2.str, sizeof(char) * str2.len);
	str[len] = 0;

	r = str;
	delete[] str;

	return r;
}

bool operator==(const iString& str1, const iString& str2)
{
	if (str1.len != str2.len) return false;
	
	for (int i = 0; i < str1.len; i++)
	{
		if (str1.at(i) != str2.at(i)) return false;
	}

	return true;
}

bool operator!=(const iString& str1, const iString& str2)
{
	if (str1.len != str2.len) return true;

	for (int i = 0; i < str1.len; i++)
	{
		if (str1.at(i) != str2.at(i)) return true;
	}

	return false;
}


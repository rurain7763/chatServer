#include"iStd.h"

float isin(float degree)
{
	return sin(ToRadian(degree));
}

float icos(float degree)
{
	if (degree == 90) return 0;
	return cos(ToRadian(degree));
}

float itan(float degree)
{
	return tan(ToRadian(degree));
}

int random()
{
	return rand();
}

unsigned int nextPow2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 16;
	v++;

	return v;
}

unsigned int nextPrime(unsigned int v)
{
	while (v <= 0xffffffff)
	{
		if (isPrime(++v)) return v;
	}

	return 0;
}

bool isPrime(unsigned int v)
{
	if (v == 1) return false;

	for (int i = 2 ; i * i <= v; i++)
	{
		if (!(v % i)) return false;
	}

	return true;
}

unsigned int iabs(int v)
{
	return v > 0 ? v : v * -1;
}

float iabs(float v)
{
	return v > 0.f ? v : v * -1;
}

unsigned long long iabs(long long v)
{
	return v > 0ll ? v : v * -1ll;
}

int imin(int v1, int v2)
{
	return v1 < v2 ? v1 : v2;
}

char* readFile(const char* path)
{
	FILE* file = fopen(path, "rb");

	if (!file) return NULL;

	fseek(file, 0, SEEK_END);
	int len = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* buffer = new char[len + 1];
	fread(buffer, sizeof(char), len, file);
	buffer[len] = 0;

	fclose(file);

	return buffer;
}

char* getDirectoryInPath(const char* path)
{
	int len = strlen(path);
	int endPos = len;
	while (endPos > 0)
	{
		endPos--;
		if (path[endPos] == '/') break;
	}

	char* r = new char[endPos + 1];
	memcpy(r, path, sizeof(char) * endPos);
	r[endPos] = 0;

	return r;
}

char* getFileNameInPath(const char* path)
{
	int len = strlen(path);
	int startPos = len;
	while (startPos > 0)
	{
		startPos--;
		if (path[startPos] == '\\' || path[startPos] == '/') break;
	}

	int rLen = len - startPos;
	char* r = new char[rLen + 1];
	memcpy(r, &path[startPos], sizeof(char) * rLen);
	r[rLen] = 0;

	return r;
}

void bubbleSort(CompareMethod m, void* data, int elementSize, int num)
{
	char* d = (char*)data;

	for (int i = num - 1; i > 0; i--)
	{
		for (int j = 0; j < i; j++)
		{
			int leftIdx = j * elementSize;
			int rightIdx = (j + 1) * elementSize;

			if (m(&d[leftIdx], &d[rightIdx]))
			{
				swap(&d[leftIdx], &d[rightIdx], elementSize);
			}
		}
	}
}

void swap(void* left, void* right, int argSize)
{
	char* copy = new char[argSize];
	memcpy(copy, left, argSize);
	memcpy(left, right, argSize);
	memcpy(right, copy, argSize);

	delete[] copy;
}

void printBit(unsigned int v)
{
	unsigned int b = 0x80000000;

	for (int i = 0; i < 32; i++)
	{
		int r = v & b;
		if (r) printf("1");
		else printf("0");
		b >>= 1;

		if (i % 4 == 3) printf(" ");
	}

	printf("\n");
}

char* toString(int v)
{
	if (v == 0)
	{
		char* r = new char[2];
		r[0] = '0';
		r[1] = 0;

		return r;
	}

	int negative = v < 0 ? 1 : 0;

	v = iabs(v);
	int digit = log10(v) + 1;
	char* r = new char[digit + negative + 1];
	int place = pow(10, digit - 1);

	digit += negative;
	for (int i = 0 + negative; i < digit; i++)
	{
		char c = (v / place) + '0';
		r[i] = c;

		v %= place;
		place /= 10;
	}

	if (negative) r[0] = '-';
	r[digit] = 0;

	return r;
}

char* toString(long long v)
{
	if (v == 0)
	{
		char* r = new char[2];
		r[0] = '0';
		r[1] = 0;

		return r;
	}

	int negative = v < 0ll ? 1 : 0;

	v = iabs(v);
	int digit = log10(v) + 1;
	char* r = new char[digit + negative + 1];
	long long place = pow(10, digit - 1);

	digit += negative;
	for (int i = 0 + negative; i < digit; i++)
	{
		char c = (v / place) + '0';
		r[i] = c;

		v %= place;
		place /= 10;
	}

	if (negative) r[0] = '-';
	r[digit] = 0;

	return r;
}

char* toString(float v)
{
	int off = v < 0 ? 1 : 0;

	v = iabs(v);
	int decimalPart = (int)v;
	int floatPart = (v - decimalPart) * pow(10, 9);

	char* decimalStr = toString(decimalPart);
	char* floatStr = toString(floatPart);

	int decimalStrLen = strlen(decimalStr);
	int floatStrLen = strlen(floatStr);
	int totalLen = decimalStrLen + floatStrLen;

	char* r = new char[totalLen + off + 2];
	if (off) r[0] = '-';
	memcpy(&r[off], decimalStr, sizeof(char) * decimalStrLen);
	r[off += decimalStrLen] = '.';
	memcpy(&r[++off], floatStr, sizeof(char) * floatStrLen);
	r[off + floatStrLen] = 0;

	delete[] decimalStr;
	delete[] floatStr;

	return r;
}

void ivsprintf(char* buff, const char* s, va_list ap)
{
	int idx = 0;
	int len = strlen(s);

	for (int i = 1; i < len; i++)
	{
		if (s[i - 1] == '%')
		{
			switch (s[i])
			{
			case 'd':
			{
				int v = va_arg(ap, int);
				char* vStr = toString(v);
				int l = strlen(vStr);
				memcpy(&buff[idx], vStr, sizeof(char) * l);
				delete[] vStr;
				idx += l;
				i++;
				break;
			}
			case 's':
			{
				char* v = va_arg(ap, char*);
				int l = strlen(v);
				memcpy(&buff[idx], v, sizeof(char) * l);
				idx += l;
				i++;
				break;
			}
			case 'f':
			{
				double v = va_arg(ap, double);
				char* vStr = toString((float)v);
				int l = strlen(vStr);
				memcpy(&buff[idx], vStr, sizeof(char) * l);
				delete[] vStr;
				idx += l;
				i++;
				break;
			}
			case '%':
			{
				//TODO
				int exception = 0;
				break;
			}
			default:
				//TODO
				int exception = 0;
				break;
			}
		}
		else
		{
			buff[idx] = s[i - 1];
			idx++;
		}
	}

	buff[idx] = 0;
}

bool isSystemLittleEndian()
{
	int testValue = 0x12345678;
	char* byte = (char*)&testValue;
	int checkValue = 0x00000000;

	for (int i = 0; i < 4; i++)
	{
		checkValue |= byte[i] << (i * 8);
	}

	return checkValue == testValue;
}

void IEEE754(float v, int& exponent, float& fraction)
{
	int* casting = reinterpret_cast<int*>(&v);
	uint8 bias = 127;
	uint32 exponentMask = 0xff << 23;
	uint32 fractionMask = 0x7fffff;
	exponent = ((*casting) & exponentMask) >> 23;
	fraction = 1.f + (float)((*casting) & fractionMask) / (float)(1 << 23);
	
	float result = fraction * (float)(1 << (exponent - bias));

	printf("%f\n", result);
}

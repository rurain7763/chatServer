#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "iType.h"
#include "iArray.h"
#include "iHashTable.h"
#include "iQueue.h"
#include "iStack.h"
#include "iString.h"
#include "iList.h"

#include "iThreadPool.h"
#include "iNetwork.h"

#ifdef _WIN32
#include "iWindows.h"
#elif __unix__
#endif

#define ToRadian(degree) degree * M_PI / 180
#define ToDegree(radian) radian * 180 / M_PI
#define formattedText(buffer, formmat) va_list ap; \
									   va_start(ap, formmat); \
									   ivsprintf(buffer, formmat, ap); \
									   va_end(ap)

float isin(float degree);
float icos(float degree);
float itan(float degree);
int random();
unsigned int nextPow2(unsigned int v);
unsigned int nextPrime(unsigned int v);
bool isPrime(unsigned int v);
unsigned int iabs(int v);
float iabs(float v);
unsigned long long iabs(long long v);
int imin(int v1, int v2);

char* readFile(const char* path);
char* getDirectoryInPath(const char* path);
char* getFileNameInPath(const char* path);

void bubbleSort(CompareMethod method, void* data, int elementSize, int num);

void swap(void* left, void* right, int argSize);

void printBit(unsigned int v);
char* toString(int v);
char* toString(float v);
char* toString(long long v);
void ivsprintf(char* buffer, const char* str, va_list ap);

bool isSystemLittleEndian();
void IEEE754(float v, int& exponent, float& fraction);

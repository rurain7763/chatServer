#pragma once

#ifdef _WIN32

typedef bool Flag;
typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned int		uint32;
typedef unsigned long long	uint64;

#elif _WIN64

typedef bool Flag;
typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned int		uint32;
typedef unsigned long		uint64;

#endif

#ifndef NULL
#define NULL 0
#endif

typedef bool (*CompareMethod)(void* left, void* right);
typedef void* (*MinMethod)(void* left, void* right);

struct iSize
{
	float width, height;
};

struct iRect
{
	float x, y;
	float width, height;
};
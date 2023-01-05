//  ***************************************************************
//  Nanoshell.hpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements the standard C functions that may
//    or may not be needed, such as memcpy, memset, strlen etc.
//
//  ***************************************************************
#include <Nanoshell.hpp>

extern "C" {

void* memcpy(void* dst, const void* src, size_t n)
{
	__asm__("rep movsb"::"c"(n),"D"(dst),"S"(src));
	return dst;
}

void* memset(void* dst, int c, size_t n)
{
	__asm__("rep stosb"::"c"(n),"D"(dst),"a"(c));
	return dst;
}

size_t strlen(const char * s)
{
	size_t sz = 0;
	while (*s++)
		sz++;
	return sz;
}

char* strcpy(char* s, const char * d)
{
	return (char*)memcpy(s, d, strlen(d) + 1);
}

char* strcat(char* s, const char * src)
{
	return strcpy(s + strlen(s), src);
}

};

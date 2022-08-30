//  ***************************************************************
//  _string.h - Creation date: 30/08/2022
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#ifndef _STRING_H
#define _STRING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

int memcmp(const void* ap, const void* bp, size_t size);
void* memcpy (void* dstptr, const void* srcptr, size_t size);
void* memmove(void* dstptr, const void* srcptr, size_t size);
void* memset(void* bufptr, uint8_t val, size_t size);

size_t strgetlento(const char* str, char chr);
size_t strlen(const char* str);
void* strcpy(const char* ds, const char* ss);
int strcmp(const char* as, const char* bs);
void strcat(char* dest, const char* after);
char* strchr (char* stringToSearch, const char characterToSearchFor);
char* strrchr(char* stringToSearch, const char characterToSearchFor);
void strtolower(char* as);
void strtoupper(char* as);
void memtolower(char* as, int w);
void memtoupper(char* as, int w);

int atoi(const char* str);
int atoihex(const char* str);

bool EndsWith(const char* pText, const char* pCheck);
bool StartsWith(const char* pText, const char* pCheck);

#ifdef __cplusplus
};
#endif

#endif//_STRING_H
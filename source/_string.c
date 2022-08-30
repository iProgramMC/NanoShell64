//  ***************************************************************
//  _string.c - Creation date: 30/08/2022
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include "_string.h"

bool EndsWith(const char* pText, const char* pCheck)
{
	int slt = strlen (pText), slc = strlen (pCheck);
	if (slt < slc) return false; //obviously, it can't.
	
	const char* pTextEnd = pText + slt;
	pTextEnd -= slc;
	return (strcmp (pTextEnd, pCheck) == 0);
}

bool StartsWith(const char* pText, const char* pCheck)
{
	int slt = strlen (pText), slc = strlen (pCheck);
	if (slt < slc) return false; //obviously, it can't.
	
	char text[slc+1];
	memcpy (text, pText, slc);
	text[slc] = 0;
	
	return (strcmp (text, pCheck) == 0);
}

int memcmp(const void* ap, const void* bp, size_t size)
{
	const uint8_t* a = (const uint8_t*) ap;
	const uint8_t* b = (const uint8_t*) bp;
	for(size_t i = 0; i < size; i++)
	{
		if(a[i] < b[i])
			return -1;
		else if(b[i] < a[i])
			return 1;
	}
	return 0;
}

void* memcpy(void* dstptr, const void* srcptr, size_t size)
{
	uint8_t* dst = (uint8_t*) dstptr;
	const uint8_t* src = (const uint8_t*) srcptr;
	for(size_t i = 0; i < size; i++)
	{
		dst[i] = src[i];
	}
	return dstptr;
}

void* memmove(void* dstptr, const void* srcptr, size_t size)
{
	uint8_t* dst = (uint8_t*) dstptr;
	const uint8_t* src = (const uint8_t*) srcptr;
	if (dst < src) {
		for (size_t i = 0; i < size; i++)
			dst[i] = src[i];
	} else {
		for (size_t i = size; i > 0; i--)
			dst[i-1] = src[i-1];
	}
	return dstptr;
}

void* memset(void* bufptr, uint8_t val, size_t size)
{
	uint8_t* buf = (uint8_t*) bufptr;
	for(size_t i = 0; i < size; i++)
	{
		buf[i] = val;
	}
	return bufptr;
}

size_t strgetlento(const char* str, char chr)
{
	size_t len = 0;
	while (str[len] != chr)
	{
		len++;
	}
	return len;
}

int atoi(const char* str) 
{
	int f = 0;
	int s = 1;
	int i = 0;
	if (str[0] == '-')
	{
		i++;
		s = -1;
	}
	for (; str[i] != '\0'; i++)
		f = f * 10 + str[i] - '0';
	
	return s * f;
}

int atoihex(const char* str)
{
	int f = 0;
	int s = 1;
	int i = 0;
	if (str[0] == '-')
	{
		i++;
		s = -1;
	}
	for (; str[i] != '\0'; i++)
	{
		f = f * 16;
		char c = str[i];
		
		if (c >= '0' && c <= '9')
			f += c - '0';
		else if (c >= 'A' && c <= 'F')
			f += c - 'A' + 0xA;
		else if (c >= 'a' && c <= 'f')
			f += c - 'a' + 0xA;
	}
	
	return s * f;
}

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
	{
		len++;
	}
	return len;
}

void* strcpy(const char* ds, const char* ss)
{
	return memcpy((void*)ds, (void*)ss, strlen(ss) + 1);
}

void strtolower(char* as)
{
	while(*as != 0)
	{
		if(*as >= 'A' && *as <= 'Z')
			*as += ('a' - 'A');
		as++;
	}
}

void strtoupper(char* as)
{
	while(*as != 0)
	{
		if(*as >= 'a' && *as <= 'z')
			*as -= ('a' - 'A');
		as++;
	}
}

void memtolower(char* as, int w)
{
	int a = 0;
	while(a <= w)
	{
		if(*as >= 'A' && *as <= 'Z')
			*as += ('a' - 'A');
		as++;
		a++;
	}
}
void memtoupper(char* as, int w)
{
	int a = 0;
	while(a <= w)
	{
		if(*as >= 'a' && *as <= 'z')
			*as -= ('a' - 'A');
		as++;
		a++;
	}
}

int strcmp(const char* as, const char* bs)
{
	size_t al = strlen(as);
	size_t bl = strlen(bs);
	if(al < bl)
	{
		return -1;
	}
	else if(al > bl)
	{
		return 1;
	}
	else if(al == bl)
	{
		return memcmp((void*)as, (void*)bs, al + 1);//Also compare null term
	}
	return 0;
}

void strcat(char* dest, const char* after)
{
	char* end = strlen(dest) + dest;
	memcpy(end, after, strlen(after) + 1);
}

char* strchr (char* stringToSearch, const char characterToSearchFor)
{
	while (*stringToSearch)
	{
		if (*stringToSearch == characterToSearchFor)
		{
			return stringToSearch;
		}
		stringToSearch++;
	}
	return NULL;
}

char* strrchr (char* stringToSearch, const char characterToSearchFor)
{
	int sl = strlen (stringToSearch);
	
	char*  p1 = stringToSearch + sl - 1;
	while (p1 >= stringToSearch)
	{
		if (*p1 == characterToSearchFor)
			return p1;
		p1--;
	}
	return NULL;
}

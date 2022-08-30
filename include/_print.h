//  ***************************************************************
//  _print.h - Creation date: 30/08/2022
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#ifndef _PRINT_H
#define _PRINT_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

size_t sprintf(char*a, const char*c, ...);
size_t vsprintf(char* memory, const char* format, va_list list);

#ifdef __cplusplus
};
#endif

#endif//_PRINT_H
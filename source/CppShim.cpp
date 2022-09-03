//  ***************************************************************
//  CppShim.cpp - Creation date: 03/09/2022
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include "System.hpp"

typedef void(*Constructor)();
typedef void(*Destructor)();

extern Constructor g_init_array_start[], g_init_array_end[];
extern Destructor  g_fini_array_start[], g_fini_array_end[];

void System::RunAllConstructors()
{
	for (auto func = g_init_array_start; func != g_init_array_end; func++)
		(*func)();
}

void System::RunAllDestructors()
{
	for (auto func = g_fini_array_start; func != g_fini_array_end; func++)
		(*func)();
}

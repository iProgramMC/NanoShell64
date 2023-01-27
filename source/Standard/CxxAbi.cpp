//  ***************************************************************
//  CxxAbi.cpp - Creation date: 07/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//
//  Module description:
//    This file contains the global constructor executor.
//  ***************************************************************
#include <Nanoshell.hpp>

typedef void(*Constructor)();
typedef void(*Destructor)();

extern Constructor g_init_array_start[], g_init_array_end[];
extern Destructor  g_fini_array_start[], g_fini_array_end[];

void RunAllConstructors()
{
	for (auto func = g_init_array_start; func != g_init_array_end; func++)
		(*func)();
}

void RunAllDestructors()
{
	for (auto func = g_fini_array_start; func != g_fini_array_end; func++)
		(*func)();
}

// cxa calls
extern "C"
{

void __cxa_pure_virtual()
{
    // Do nothing or print an error message.
	LogMsg("Pure virtual function!");
}

};

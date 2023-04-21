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
#include <NanoShell.hpp>
#include <MemoryManager.hpp>

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

static void* OperatorNew(size_t size)
{
	void* pMem = VMM::KernelHeap::Allocate(size);
	
	if (!pMem)
		KernelPanic("ERROR: cannot new[](%ll), kernel heap gave us NULL.");
	
	return pMem;
}

static void OperatorFree(void* ptr)
{
	VMM::KernelHeap::Free(ptr);
}

void* operator new(size_t size)
{
	return OperatorNew(size);
}

void* operator new[](size_t size)
{
	return OperatorNew(size);
}

void* operator new(size_t size, const nopanic_t&)
{
	return VMM::KernelHeap::Allocate(size);
}

void* operator new[](size_t size, const nopanic_t&)
{
	return VMM::KernelHeap::Allocate(size);
}

void operator delete(void* ptr)
{
	OperatorFree(ptr);
}

void operator delete(void* ptr, UNUSED size_t size)
{
	OperatorFree(ptr);
}

void operator delete[](void* ptr)
{
	OperatorFree(ptr);
}

void operator delete[](void* ptr, UNUSED size_t size)
{
	OperatorFree(ptr);
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

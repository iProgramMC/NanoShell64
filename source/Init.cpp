//  ***************************************************************
//  Init.cpp - Creation date: 04/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include <stdint.h>
#include <stddef.h>
#include <_limine.h>

#include <Arch.hpp>
#include <Atomic.hpp>
#include <Spinlock.hpp>
#include <EternalHeap.hpp>

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

volatile limine_terminal_request g_TerminalRequest =
{
	.id = LIMINE_TERMINAL_REQUEST,
	.revision = 0,
	.response = NULL,
	.callback = NULL,
};

volatile limine_bootloader_info_request g_BootloaderInfoRequest =
{
	.id = LIMINE_BOOTLOADER_INFO_REQUEST,
	.revision = 0,
	.response = NULL,
};

volatile limine_hhdm_request g_HHDMRequest =
{
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0,
	.response = NULL,
};

// note: these definitions are purely temporary

Spinlock g_E9Spinlock;
Spinlock g_TermSpinlock;

extern "C" void _e9_puts(const char* str)
{
	//LockGuard lg(g_E9Spinlock);
	while (*str)
	{
		Arch::WriteByte(0xE9, *str);
		str++;
	}
}

extern "C" size_t _strlen(const char* str)
{
	size_t len = 0;
	while (*str)
	{
		len++;
		str++;
	}
	return len;
}

extern "C" void _term_puts(const char* str)
{
	LockGuard lg(g_TermSpinlock);
	g_TerminalRequest.response->write(g_TerminalRequest.response->terminals[0], str, _strlen(str));
}

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

// The following will be our kernel's entry point.
extern "C" void _start(void)
{
	// Ensure we have our responses.
	if (g_TerminalRequest.response == NULL || g_TerminalRequest.response->terminal_count < 1)
		Arch::IdleLoop();
	
	RunAllConstructors();
	
	limine_smp_response* pSMP = Arch::CPU::GetSMPResponse();
	if (!pSMP)
		Arch::IdleLoop();
	
	_term_puts("NanoShell64 (TM), January 2023 - V0.001\n");
	
	// Since this is an SMP system, we should bootstrap the CPUs.
	if (pSMP->cpu_count == 1)
	{
		_term_puts("Found uniprocessor system\n");
	}
	else
	{
		char foundprocs[] = "Found X-processor system\n";
		foundprocs[6] = '0' + pSMP->cpu_count;
		_term_puts(foundprocs);
	}
	
	Arch::CPU::InitAsBSP();
	
	char procs[] = "All X processors have been bootstrapped.\n";
	procs[4] = '0' + Arch::CPU::GetCount();
	
	_term_puts(procs);
	
	Arch::IdleLoop();
}

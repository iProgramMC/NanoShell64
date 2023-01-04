//  ***************************************************************
//  main.cpp - Creation date: 04/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include <stdint.h>
#include <stddef.h>
#include <_limine.h>

#include <Atomic.hpp>
#include <Spinlock.hpp>

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

volatile limine_smp_request g_SMPRequest =
{
	.id = LIMINE_SMP_REQUEST,
	.revision = 0,
	.response = NULL,
	.flags = 0,
};

// note: these definitions are purely temporary

Spinlock g_E9Spinlock;
Spinlock g_TermSpinlock;

extern "C" void _hang(void)
{
	__asm__("cli");
	for (;;) __asm__("hlt");
}

extern "C" void _outb(uint16_t port, uint8_t data)
{
	__asm__("outb %0, %1"::"a"((uint8_t)data),"Nd"((uint16_t)port));
}

extern "C" void _e9_puts(const char* str)
{
	LockGuard lg(g_E9Spinlock);
	while (*str)
	{
		_outb(0xE9, *str);
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

Atomic<int> g_CPUsInitialized;

// TODO: a sprintf() like function, and other string functions.

// Since the pointer to the structure is passed into RDI, assuming
// the x86_64 System V ABI, the first argument corresponds to RDI.
void CPUBootstrap(limine_smp_info* pInfo)
{
	// The X will be replaced.
	char hello_text[] = "Hello from processor #X!\n";
	hello_text[22] = '0' + pInfo->processor_id;
	_term_puts(hello_text);
	
	g_CPUsInitialized.FetchAdd(1);
	
	if (pInfo->processor_id == 0)
	{
		// wait a bit
		for (int i = 0; i < 100000000; i++)
		{
			__asm__("":::"memory");
		}
		
		int loaded = g_CPUsInitialized.Load();
		
		char procs[] = "X processors have been bootstrapped.\n";
		procs[0] = '0' + loaded;
		
		_term_puts(procs);
	}
	
	_hang();
}


// The following will be our kernel's entry point.
extern "C" void _start(void)
{
	// Ensure we have our responses.
	if (g_TerminalRequest.response == NULL || g_TerminalRequest.response->terminal_count < 1 || 
	    g_SMPRequest.response == NULL)
		_hang();
		
	limine_smp_response* pSMP = g_SMPRequest.response;
	
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
	
	// Boot strap each of the processors.
	uint64_t thisProcessorIndex = -1; // we haven't found our processor yet
	
	for (uint64_t i = 0; i < pSMP->cpu_count; i++)
	{
		if (pSMP->bsp_lapic_id == pSMP->cpus[i]->lapic_id)
		{
			// we will bootstrap this processor last
			thisProcessorIndex = i;
		}
		
		// writing to the goto_address of the BSP actually doesn't do anything
		pSMP->cpus[i]->extra_argument = i;
		pSMP->cpus[i]->goto_address = CPUBootstrap;
	}
	
	if (thisProcessorIndex == (uint64_t)-1)
	{
		_term_puts("ERROR: Did not find this processor's index");
		_hang();
	}
	
	CPUBootstrap(pSMP->cpus[thisProcessorIndex]);
}

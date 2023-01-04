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
	while (*str)
	{
		_outb(0xE9, *str);
		str++;
	}
}

Atomic<int> g_CPUsInitialized;

// Since the pointer to the structure is passed into RDI, assuming
// the x86_64 System V ABI, the first argument corresponds to RDI.
void CPUBootstrap(limine_smp_info* pInfo)
{
	char pid[2];
	pid[1] = 0;
	pid[0] = '0' + pInfo->processor_id;
	_e9_puts("Hello world from ");
	_e9_puts(pid);
	_e9_puts("!\n");
	
	g_CPUsInitialized.FetchAdd(1);
	
	if (pInfo->processor_id == 0)
	{
		// wait a bit
		for (int i = 0; i < 100000000; i++)
		{
			__asm__("":::"memory");
		}
		
		int loaded = 0;
		g_CPUsInitialized.Load(&loaded);
		
		char start[2];
		start[1] = 0;
		start[0] = '0' + loaded;
		
		_e9_puts("\n\n\n");
		_e9_puts(start);
		_e9_puts(" processors have been bootstrapped.\n");
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
	
	// Since this is an SMP system, we should bootstrap the CPUs.
	if (pSMP->cpu_count == 1)
	{
		_e9_puts("Found uniprocessor system\n");
	}
	else
	{
		char chr[2];
		chr[1] = 0;
		chr[0] = '0' + pSMP->cpu_count;
		_e9_puts("Found ");
		_e9_puts(chr);
		_e9_puts("-processor system\n");
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
		_e9_puts("ERROR: Did not find this processor's index");
		_hang();
	}
	
	CPUBootstrap(pSMP->cpus[thisProcessorIndex]);
}

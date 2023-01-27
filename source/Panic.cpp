//  ***************************************************************
//  Panic.cpp - Creation date: 27/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements the kernel panic functionality.
//
//  ***************************************************************
#include <stdarg.h>
#include <_limine.h>
#include <Arch.hpp>
#include <Spinlock.hpp>

static Spinlock g_PanicLock;

extern Atomic<int> g_panickedCpus;
extern Spinlock g_E9Spinlock;
extern Spinlock g_TermSpinlock;

void KernelPanicLogMsg(const char* fmt, va_list lst)
{
	char chr[8192];
	vsnprintf(chr, sizeof chr, fmt, lst);
	LogMsg("\nMessage: %s\n", chr);
}

extern "C" void KernelPanic(const char* fmt, ...)
{
	using namespace Arch;
	
	if (!g_PanicLock.TryLock())
	{
		// well, what a coincidence! We panicked here too. Simply wait forever at this pointer
		// Eventually an IPI will come, putting us out of our misery.
		while (true)
			Halt();
	}
	
	// send all other processors an IPI
	limine_smp_response* pResp = CPU::GetSMPResponse();
	
	CPU* pThisCpu = CPU::GetCurrent();
	
	g_panickedCpus.FetchAdd(1);
	
	for (uint32_t pid = 0; pid < pResp->cpu_count; pid++)
	{
		CPU* pCpu = CPU::GetCPU(pid);
		if (pCpu == pThisCpu) continue;
		
		pCpu->SendIPI(CPU::eIpiType::PANIC);
	}
	
	while (g_panickedCpus.Load() < (int)pResp->cpu_count)
	{
		Spinlock::SpinHint();
	}
	
	// now that all CPUs are halted, forcefully unlock the LogMsg mutex, just in case we crashed within LogMsg functions
	g_E9Spinlock.Unlock();
	g_TermSpinlock.Unlock();
	
	va_list lst;
	va_start(lst, fmt);
	
	LogMsg("KERNEL PANIC! (CPU %u)", pThisCpu->ID());
	KernelPanicLogMsg(fmt, lst);
	LogMsg("Note: Last return address: %p", __builtin_return_address(0));
	// TODO: A stack frame unwinder. NanoShell32 can do this, why not 64?
	
	va_end(lst);
}



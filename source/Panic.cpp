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

extern "C" void KernelPanic(const char* fmt, ...)
{
	using namespace Arch;
	CPU* pThisCpu = CPU::GetCurrent();
	
	char panic_formatted[8192];
	
	va_list lst;
	va_start(lst, fmt);
	vsnprintf(panic_formatted, sizeof panic_formatted, fmt, lst);
	
	// log the panic to the console, in case the code to force panic the other CPUs fails to send the IPIs for some reason
	SLogMsg("CPU %d - PANIC: %s (RA:%p)", pThisCpu->ID(), panic_formatted, __builtin_return_address(0));
	
	// if the panic lock is already locked
	if (!g_PanicLock.TryLock())
	{
		// well, what a coincidence! We panicked here too. Simply wait forever at this pointer
		// Eventually an IPI will come, putting us out of our misery.
		while (true)
			Halt();
	}
	
	// really, we should ::Store(1), however we can only panic once during the lifetime of the kernel, so this and Store(1) are equivalent
	g_panickedCpus.FetchAdd(1);
	
	// send all other processors an IPI
	limine_smp_response* pResp = CPU::GetSMPResponse();
	
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
	
	LogMsg("KERNEL PANIC! (CPU %u)", pThisCpu->ID());
	LogMsg("\nMessage: %s\n", panic_formatted);
	LogMsg("Note: Last return address: %p", __builtin_return_address(0));
	// TODO: A stack frame unwinder. NanoShell32 can do this, why not 64?
	
	va_end(lst);
	
	Arch::IdleLoop();
}



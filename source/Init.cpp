//  ***************************************************************
//  Init.cpp - Creation date: 04/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include <Nanoshell.hpp>
#include <_limine.h>

#include <Arch.hpp>
#include <Atomic.hpp>
#include <Spinlock.hpp>
#include <Terminal.hpp>
#include <MemoryManager.hpp>

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

volatile limine_bootloader_info_request g_BootloaderInfoRequest =
{
	.id = LIMINE_BOOTLOADER_INFO_REQUEST,
	.revision = 0,
	.response = NULL,
};

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
	// Ensure Limine has set up these features.
	if (!Terminal::CheckResponse() || !Arch::CPU::GetSMPResponse() || !Arch::CPU::GetHHDMResponse())
		Arch::IdleLoop();
	
	RunAllConstructors();
	
	Terminal::Setup();
	
	LogMsg("NanoShell64 (TM), January 2023 - V0.001");
	
	PMM::Init();
	
#ifdef TARGET_X86_64
	Arch::APIC::EnsureOn();
#endif
	
	uint32_t processorCount = Arch::CPU::GetCount();
	LogMsg("%d System Processor%s [%llu Kb Memory] MultiProcessor Kernel", processorCount, processorCount == 1 ? "" : "s", PMM::GetTotalPages() * 4);
	
	// Initialize the other CPUs. This should not return.
	Arch::CPU::InitAsBSP();
}

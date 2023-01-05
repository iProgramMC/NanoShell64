//  ***************************************************************
//  CPU.hpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#include <Arch.hpp>
#include <Atomic.hpp>

extern Atomic<int> g_CPUsInitialized; // Arch.cpp

// TODO: a sprintf() like function, and other string functions.
extern "C" void _term_puts(const char* str);

void Arch::CPU::Go()
{
	// Write the GS base MSR.
	Arch::WriteMSR(Arch::eMSR::KERNEL_GS_BASE, uint64_t(this));
	
	// The X will be replaced.
	char hello_text[] = "Hello from processor #X!\n";
	hello_text[22] = '0' + m_processorID;
	_term_puts(hello_text);
	
	g_CPUsInitialized.FetchAdd(1);
	
	if (!m_bIsBSP)
		Arch::IdleLoop();
}




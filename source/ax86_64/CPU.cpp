//  ***************************************************************
//  CPU.cpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements the CPU object specific functions.
//
//  ***************************************************************
#include <Arch.hpp>
#include <Atomic.hpp>
#include <Terminal.hpp>

extern Atomic<int> g_CPUsInitialized; // Arch.cpp

// TODO: a sprintf() like function, and other string functions.
extern "C" void _term_puts(const char* str);

void Arch::CPU::Init()
{
	// Write the GS base MSR.
	Arch::WriteMSR(Arch::eMSR::KERNEL_GS_BASE, uint64_t(this));
	
	// Re-load the GDT.
	LoadGDT();
	
	// The X will be replaced.
	char hello_text[] = "Hello from processor #X!\n";
	hello_text[22] = '0' + m_processorID;
	
	Terminal::Write(hello_text);
	
	g_CPUsInitialized.FetchAdd(1);
}

void Arch::CPU::Go()
{
	// The X will be replaced.
	char hello_text[] = "processor #X is going!\n";
	hello_text[11] = '0' + m_processorID;
	
	Terminal::E9Write(hello_text);
	
	Arch::IdleLoop();
}

Arch::CPU* Arch::CPU::GetCurrent()
{
	return (CPU*)ReadMSR(Arch::eMSR::KERNEL_GS_BASE);
}


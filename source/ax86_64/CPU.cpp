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
	WriteMSR(Arch::eMSR::KERNEL_GS_BASE, uint64_t(this));
	
	// Re-load the GDT.
	LoadGDT();
	
	// Initialize the APIC on this CPU.
	APIC::Init();
	
	// The X will be replaced.
	LogMsg("Hello from processor #%d", m_processorID);
	
	g_CPUsInitialized.FetchAdd(1);
}

void Arch::CPU::Go()
{
	// TODO: If the bootstrap processor, do some other stuff, like spawn an initial thread
	if (m_bIsBSP)
	{
		LogMsg("I am the bootstrap processor, and I will soon spawn an initial task instead of printing this!");
		
		// Since all other processors are running, try sending an IPI to processor 1.
		CPU* p1 = GetCPU(1);
		p1->SendTestIPI();
	}
	
	Arch::IdleLoop();
}

Arch::CPU* Arch::CPU::GetCurrent()
{
	return (CPU*)ReadMSR(Arch::eMSR::KERNEL_GS_BASE);
}

// Set the CPU's interrupt gate to the following handler.
void Arch::CPU::SetInterruptGate(uint8_t intNum, uintptr_t fnHandler, uint8_t ist, uint8_t dpl)
{
	m_idt.SetEntry(intNum, IDT::Entry(fnHandler, ist, dpl));
}

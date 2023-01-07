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
	using namespace VMM;
	
	// Write the GS base MSR.
	WriteMSR(Arch::eMSR::KERNEL_GS_BASE, uint64_t(this));
	
	// Re-load the GDT.
	LoadGDT();
	
	// Clone the page mapping and assign it to this CPU. This will
	// ditch the lower half mapping that the bootloader has provided us.
	m_pPageMap = PageMapping::GetFromCR3()->Clone(false);
	
	// We shall at least try to map a new page in. Use the new fangled tech.
	uintptr_t addr = 0x100000;
	
	bool result = m_pPageMap->MapPage(addr);
	
	if (!result)
	{
		LogMsg("Couldn't MapPage");
	}
	else
	{
		// try out our new fangled mapping!
		*((uint64_t*)addr) = 0xFEDCBA9876543210 + m_processorID;
		
		LogMsg("Got back: %p", *((uint64_t*)addr));
	}
	
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

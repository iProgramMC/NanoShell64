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
#include <EternalHeap.hpp>

extern Atomic<int> g_CPUsInitialized; // Arch.cpp

extern "C" void CPU_OnPageFault_Asm();
extern "C" void Arch_APIC_OnInterrupt_Asm();
extern "C" void CPU_OnPageFault(Registers* pRegs)
{
	Arch::CPU::GetCurrent()->OnPageFault(pRegs);
}

void Arch::CPU::SetupGDTAndIDT()
{
	// Re-load the GDT.
	LoadGDT();
	
	// Setup the IDT....
	SetInterruptGate(IDT::INT_PAGE_FAULT, uintptr_t(CPU_OnPageFault_Asm));
	SetInterruptGate(IDT::INT_IPI,        uintptr_t(Arch_APIC_OnInterrupt_Asm));
	
	// Load the IDT.
	LoadIDT();
}

static Atomic<bool> g_bIsBSPInitted(false);

void Arch::CPU::OnBSPInitialized()
{
	g_bIsBSPInitted.Store(true);
}

void Arch::CPU::WaitForBSP()
{
	while (!g_bIsBSPInitted.Load())
		Spinlock::SpinHint();
}

bool Arch::CPU::SetInterruptsEnabled(bool b)
{
	if (GetCurrent() != this)
	{
		SLogMsg("Error: Arch::CPU::SetInterruptsEnabled can only be called on the same CPU it's modifying");
	}
	
	bool x = m_InterruptsEnabled;
	
	if (m_InterruptsEnabled != b)
	{
		m_InterruptsEnabled = b;
		
		if (b)
			ASM("sti":::"memory");
		else
			ASM("cli":::"memory");
	}
	
	return x;
}

void Arch::CPU::Init()
{
	bool bIsBSP = GetSMPResponse()->bsp_lapic_id == m_pSMPInfo->lapic_id;
	
	using namespace VMM;
	
	if (!bIsBSP)
	{
		WaitForBSP();
	}
	
	// Allocate a small stack
	m_pIsrStack = EternalHeap::Allocate(C_INTERRUPT_STACK_SIZE);
	
	// Set it in the TSS
	m_gdt.m_tss.m_rsp[0] = m_gdt.m_tss.m_rsp[1] = m_gdt.m_tss.m_rsp[2] = uint64_t(m_pIsrStack);
	
	// Write the GS base MSR.
	WriteMSR(Arch::eMSR::KERNEL_GS_BASE, uint64_t(this));
	
	SetupGDTAndIDT();
	
	if (bIsBSP)
	{
		KernelHeap::Init();
	}
	
	// Clone the page mapping and assign it to this CPU. This will
	// ditch the lower half mapping that the bootloader has provided us.
	m_pPageMap = PageMapping::GetFromCR3()->Clone(false);
	m_pPageMap->SwitchTo();
	
	/*
	LogMsg("Cloned, gonna map...");
	
	// We shall at least try to map a new page in. Use the new fangled tech.
	uintptr_t addr = 0x100000;
	
	bool result = m_pPageMap->MapPage(addr);
	
	LogMsg("Mapped?");
	
	if (!result)
	{
		LogMsg("Couldn't MapPage");
	}
	else
	{
		// try out our new fangled mapping!
		*((uint64_t*)addr) = 0xFEDCBA9876543210 + m_processorID;
		ASM("":::"memory");
		LogMsg("Got back: %p", *((uint64_t*)addr));
	}
	*/
	
	/*
	// Try to allocate and test some memory.
	LogMsg("Allocating from the kernel heap....");
	uint8_t * p1 = new(nopanic) uint8_t[4286];
	uint8_t * p2 = new(nopanic) uint8_t[5768];
	
	LogMsg("p1 = %p, p2 = %p", p1, p2);
	
	delete[] p1;
	delete[] p2;
	*/
	
	// Initialize the APIC on this CPU.
	APIC::Init();
	
	// The X will be replaced.
	LogMsg("Hello from processor #%d", m_processorID);
	
	g_CPUsInitialized.FetchAdd(1);
	
	// Enable interrupts.
	SetInterruptsEnabled(true);
	
	// Initialize our scheduler.
	m_Scheduler.Init();
	
	if (bIsBSP)
	{
		OnBSPInitialized();
	}
}

void Arch::CPU::Go()
{
	// TODO: If the bootstrap processor, do some other stuff, like spawn an initial thread
	if (m_bIsBSP)
	{
		LogMsg("I am the bootstrap processor, and I will soon spawn an initial task instead of printing this!");
		
		// Since all other processors are running, try sending an IPI to processor 1.
		CPU* p1 = GetCPU(1);
		
		if (p1)
			p1->SendIPI(eIpiType::HELLO);
	
		//KernelPanic("Hello there %d", 1337);
	}
	
	Thread::Yield();
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

Atomic<int> g_panickedCpus { 0 };

void Arch::CPU::OnIPI()
{
	eIpiType type = m_ipiType;
	
	switch (type)
	{
		case eIpiType::NONE: break;
		case eIpiType::HELLO:
		{
			LogMsg("Got IPI! (I am processor %u)", m_processorID);
			break;
		}
		case eIpiType::PANIC:
		{
			SLogMsg("Processor %u got panic IPI from someone.", m_processorID);
			
			g_panickedCpus.FetchAdd(1);
			
			SetInterruptsEnabled(false);
			Arch::IdleLoop();
		}
	}
}

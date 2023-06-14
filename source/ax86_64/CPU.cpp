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

Spinlock g_CalibrateSpinlock;

extern "C" void CPU_OnPageFault_Asm();
extern "C" void Arch_APIC_OnIPInterrupt_Asm();
extern "C" void Arch_APIC_OnTimerInterrupt_Asm();
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
	SetInterruptGate(IDT::INT_IPI,        uintptr_t(Arch_APIC_OnIPInterrupt_Asm));
	SetInterruptGate(IDT::INT_APIC_TIMER, uintptr_t(Arch_APIC_OnTimerInterrupt_Asm));
	//SetInterruptGate(0, uintptr_t(Arch_APIC_OnTimerInterrupt_Asm));
	
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
		return false;
	}
	
	bool x = m_InterruptsEnabled;
	
	m_InterruptsEnabled = b;
	
	if (b)
		ASM("sti":::"memory");
	else
		ASM("cli":::"memory");
	
	return x;
}

void Arch::CPU::CalibrateTimer()
{
	// Since the calibration operation needs to be performed carefully, we can't have
	// interrupts enabled, or anyone else messing with the PIT. Take ownership of the
	// PIT now.
	LockGuard lg(g_CalibrateSpinlock);
	
	APIC::CalibrateTimer(m_LapicTicksPerMS, m_TscTicksPerMS);
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
	
	if (bIsBSP)
	{
		RSD::Load();
	}
	
	// Initialize the APIC on this CPU.
	APIC::Init();
	
	// Calibrate its timer.
	CalibrateTimer();
	
	// The X will be replaced.
	LogMsg("Processor #%d has come online.", m_processorID);
	
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

Atomic<int> g_CPUsReady;

void Arch::CPU::Go()
{
	auto* pResponse = GetSMPResponse();
	
	int cpuCount = int(pResponse->cpu_count);
	
	// TODO: If the bootstrap processor, do some other stuff, like spawn an initial thread
	if (m_bIsBSP)
	{
		// for each CPU, check if the lapic ticks per MS is very close between CPUs
		int64_t LapicTicksPerMS_Avg = 0;
		int64_t TscTicksPerMS_Avg   = 0;
		
		for (int i = 0; i < cpuCount; i++)
		{
			CPU *pCpu = (CPU*)(pResponse->cpus[i]->extra_argument);
			
			LapicTicksPerMS_Avg += pCpu->m_LapicTicksPerMS;
			TscTicksPerMS_Avg   += pCpu->m_TscTicksPerMS;
		}
		
		LapicTicksPerMS_Avg /= cpuCount;
		TscTicksPerMS_Avg   /= cpuCount;
		
		int64_t diffApic = LapicTicksPerMS_Avg - m_LapicTicksPerMS;
		int64_t diffTsc  = LapicTicksPerMS_Avg - m_LapicTicksPerMS;
		
		// if it's within 100 ticks of tolerance...
		if (diffApic <= 1000 && diffApic >= -1000)
		{
			// just set the rest of the CPUs' LAPIC timer to the average;
			for (int i = 0; i < cpuCount; i++)
			{
				CPU *pCpu = (CPU*)(pResponse->cpus[i]->extra_argument);
				
				pCpu->m_LapicTicksPerMS = LapicTicksPerMS_Avg;
			}
		}
		// if it's within 100 ticks of tolerance...
		if (diffTsc <= 1000 && diffTsc >= -1000)
		{
			// just set the rest of the CPUs' LAPIC timer to the average;
			for (int i = 0; i < cpuCount; i++)
			{
				CPU *pCpu = (CPU*)(pResponse->cpus[i]->extra_argument);
				
				pCpu->m_TscTicksPerMS = TscTicksPerMS_Avg;
			}
		}
		
		for (int i = 0; i < cpuCount; i++)
		{
			CPU *pCpu = (CPU*)(pResponse->cpus[i]->extra_argument);
			
			LogMsg("CPU %d has APIC tick rate %lld, TSC tick rate %lld", i, pCpu->m_LapicTicksPerMS, pCpu->m_TscTicksPerMS);
		}
		
		LogMsg("I am the bootstrap processor, and I will soon spawn an initial task instead of printing this!");
		
		// Since all other processors are running, try sending an IPI to processor 1.
		CPU* p1 = GetCPU(1);
		
		if (p1)
			p1->SendIPI(eIpiType::HELLO);
	
		//KernelPanic("Hello there %d", 1337);
	}
	
	g_CPUsReady.FetchAdd(1);
	
	while (g_CPUsReady.Load(ATOMIC_MEMORD_RELAXED) < cpuCount)
		Spinlock::SpinHint();
	
	m_StartingTSC = TSC::Read();
	
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

void Arch::CPU::OnTimerIRQ(Registers* pRegs)
{
	m_Scheduler.OnTimerIRQ(pRegs);
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

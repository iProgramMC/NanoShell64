//  ***************************************************************
//  APIC.cpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements a manager for the APIC for each CPU.
//
//  ***************************************************************
#include <Arch.hpp>
#include <Terminal.hpp>
#include <Spinlock.hpp>

#define C_SPURIOUS_INTERRUPT_VECTOR (0xFF)
#define C_APIC_TIMER_DIVIDE_BY_128  (0b1010) // Intel SDM Vol.3A Ch.11 "11.5.4 APIC Timer". Bit 2 is reserved.
#define C_APIC_TIMER_DIVIDE_BY_16   (0b0011) // Intel SDM Vol.3A Ch.11 "11.5.4 APIC Timer". Bit 2 is reserved.
#define C_APIC_TIMER_MODE_ONESHOT   (0b00 << 17)
#define C_APIC_TIMER_MODE_PERIODIC  (0b01 << 17) // not used right now, but may be needed.
#define C_APIC_TIMER_MODE_TSCDEADLN (0b10 << 17)

#define APIC_LVT_INT_MASKED (0x10000)

#define IA32_APIC_BASE_MSR (0x1B)

using namespace Arch;

enum
{
	APIC_REG_ID            = 0x20,
	APIC_REG_VER           = 0x30,
	APIC_REG_TASK_PRIORITY = 0x80,
	APIC_REG_ARB_PRIORITY  = 0x90,
	APIC_REG_PROC_PRIORITY = 0xA0,
	APIC_REG_EOI           = 0xB0,
	APIC_REG_REMOTE_READ   = 0xC0,
	APIC_REG_LOGICAL_DEST  = 0xD0,
	APIC_REG_DEST_FORMAT   = 0xE0,
	APIC_REG_SPURIOUS      = 0xF0,
	APIC_REG_ISR_START     = 0x100, // 0x100 - 0x170
	APIC_REG_TRIG_MODE     = 0x180, // 0x180 - 0x1F0
	APIC_REG_IRQ           = 0x200, // 0x200 - 0x270
	APIC_REG_ERROR_STAT    = 0x280,
	APIC_REG_LVT_CMCI      = 0x2F0,
	APIC_REG_ICR0          = 0x300,
	APIC_REG_ICR1          = 0x310,
	APIC_REG_LVT_TIMER     = 0x320,
	APIC_REG_LVT_THERMAL   = 0x330,
	APIC_REG_LVT_PERFMON   = 0x340,
	APIC_REG_LVT_LINT0     = 0x350,
	APIC_REG_LVT_LINT1     = 0x360,
	APIC_REG_LVT_ERROR     = 0x370,
	APIC_REG_TMR_INIT_CNT  = 0x380,
	APIC_REG_TMR_CURR_CNT  = 0x390,
	APIC_REG_TMR_DIV_CFG   = 0x3E0,
};

enum
{
	APIC_ICR0_DELIVERY_STATUS = (1 << 12),
};

enum
{
	APIC_ICR1_SINGLE           = (0 << 18),
	APIC_ICR1_SELF             = (1 << 18),
	APIC_ICR1_BROADCAST        = (2 << 18),
	APIC_ICR1_BROADCAST_OTHERS = (3 << 18),
};

// Write a register.
void APIC::WriteReg(uint32_t reg, uint32_t value)
{
	WritePhys(GetLapicBasePhys() + reg, value);
}

// Read a register.
uint32_t APIC::ReadReg(uint32_t reg)
{
	return ReadPhys(GetLapicBasePhys() + reg);
}

uintptr_t APIC::GetLapicBasePhys()
{
	// Read the specific MSR.
	/*uintptr_t msr = ReadMSR(IA32_APIC_BASE_MSR);
	
	return msr & 0x0000'000F'FFFF'F000;
	*/
	return 0xFEE0'0000; // really I think it's fine if we hardcode it
}

uintptr_t APIC::GetLapicBase()
{
	return GetHHDMOffset() + GetLapicBasePhys();
}

void APIC::EndOfInterrupt()
{
	APIC::WriteReg(APIC_REG_EOI, 0);
}

void APIC::EnsureOn()
{
	// Use the CPUID instruction.
	uint32_t eax, edx;
	ASM("cpuid":"=d"(edx),"=a"(eax):"a"(1));
	
	if (~edx & (1 << 9))
	{
		LogMsg("APIC is off. An APIC must be present before running NanoShell64.");
		IdleLoop();
	}
}

extern "C" void Arch_APIC_OnSpInterrupt_Asm();
extern "C" void Arch_APIC_OnSpInterrupt()
{
	APIC::EndOfInterrupt();
}

extern "C" void Arch_APIC_OnIPInterrupt_Asm();
extern "C" void Arch_APIC_OnIPInterrupt()
{
	using namespace Arch;
	
	// Get the current CPU.
	CPU* pCpu = CPU::GetCurrent();
	
	// Tell it that we've IPI'd.
	pCpu->OnIPI();
	
	// Send an EOI.
	APIC::EndOfInterrupt();
	
	// since a CPU::SendIPI() call was needed to reach this point, unlock the IPI spinlock.
	pCpu->UnlockIpiSpinlock();
}

extern "C" void Arch_APIC_OnTimerInterrupt_Asm();
extern "C" void Arch_APIC_OnTimerInterrupt(Registers* pRegs)
{
	using namespace Arch;
	
	// Get the current CPU.
	CPU* pCpu = CPU::GetCurrent();
	
	// make sure to let ourselves know that right now, interrupts are disabled.
	pCpu->InterruptsEnabledRaw() = false;
	
	// Tell it that we've IPI'd.
	pCpu->OnTimerIRQ(pRegs);
	
	// Send an EOI.
	APIC::EndOfInterrupt();
	
	// go back to the old state
	pCpu->InterruptsEnabledRaw() = (pRegs->rflags & C_RFLAGS_INTERRUPT_FLAG);
}

void APIC::Init()
{
	if (CPU::AreWeBootstrap())
	{
		LogMsg("Disabling legacy PIC(s).");
		
		constexpr uint16_t picComdPrim = 0x20, picComdSecd = 0xA0, picDataPrim = 0x21, picDataSecd = 0xA1;
		
		// Start legacy PIC init sequence.
		WriteByte(picComdPrim, 0x11);
		WriteByte(picComdSecd, 0x11);
		// Re-map IRQs.
		WriteByte(picDataPrim, 0x20);
		WriteByte(picDataSecd, 0x28);
		// Do other fancy stuff
		WriteByte(picDataPrim, 4);
		WriteByte(picDataSecd, 2);
		WriteByte(picDataPrim, 1);
		WriteByte(picDataSecd, 1);
		// Mask all interrupts.
		WriteByte(picDataPrim, 0xFF);
		WriteByte(picDataSecd, 0xFF);
	}
	
	// Set this CPU's IDT entry.
	CPU::GetCurrent()->SetInterruptGate(IDT::INT_SPURIOUS, uintptr_t(Arch_APIC_OnSpInterrupt_Asm));
	
	// Enable the spurious vector interrupt.
	WriteReg(APIC_REG_SPURIOUS, IDT::INT_SPURIOUS | 0x100);
}

void CPU::SendIPI(eIpiType type)
{
	// The destination is 'this'. The sender (us) is 'pSenderCPU'.
	CPU * pSenderCPU = GetCurrent();
	
	// Wait for any pending IPIs to finish on this CPU.
	while (APIC::ReadReg(APIC_REG_ICR0) & APIC_ICR0_DELIVERY_STATUS) Spinlock::SpinHint();
	
	m_ipiSpinlock.Lock();
	
	m_ipiType = type;
	m_ipiSenderID = pSenderCPU->m_processorID;
	
	// Write the destination CPU's LAPIC ID.
	APIC::WriteReg(APIC_REG_ICR1, m_pSMPInfo->lapic_id << 24);
	
	// Write the interrupt vector.
	APIC::WriteReg(APIC_REG_ICR0, IDT::INT_IPI | APIC_ICR1_SINGLE);
	
	// The CPU in question will unlock the IPI spinlock.
}

PolledSleepFunc g_PolledSleepFunc = PIT::PolledSleep;

void APIC::SetPolledSleepFunc(PolledSleepFunc func)
{
	g_PolledSleepFunc = func;
}

constexpr uint64_t C_FEMTOS_TO_NANOS = 1'000'000;
constexpr uint64_t C_MILLIS_TO_NANOS = 1'000'000;

void APIC::CalibrateHPET(uint64_t &apicOut, uint64_t &tscOut)
{
	uint64_t avg_apic = 0;
	uint64_t avg_tsc  = 0;
	constexpr int nRuns = 16;
	constexpr int nMs   = 20;
	
	for (int i = 0; i < nRuns; i++)
	{
		// Set APIC init counter to -1.
		APIC::WriteReg(APIC_REG_TMR_INIT_CNT, 0xFFFFFFFF);
		
		// Sleep for X ms.
		uint64_t time = nMs * C_MILLIS_TO_NANOS * C_FEMTOS_TO_NANOS / HPET::GetCounterClockPeriod();
		
		uint64_t tscThen  = TSC::Read();
		uint64_t hpetThen = HPET::GetRawTickCount();
		uint64_t target   = hpetThen + time;
		
		while (HPET::GetRawTickCount() < target)
			Spinlock::SpinHint();
		
		APIC::WriteReg(APIC_REG_LVT_TIMER, APIC_LVT_INT_MASKED);
		uint64_t tscNow   = TSC::Read();
		uint64_t hpetNow  = HPET::GetRawTickCount();
		uint64_t apicDiff = 0xFFFFFFFF - APIC::ReadReg(APIC_REG_TMR_CURR_CNT);
		
		uint64_t tscDiff  = tscNow - tscThen;
		uint64_t hpetDiff = hpetNow - hpetThen;
		
		// rescale the TSC and APIC diffs by the HPET diff
		tscDiff  = tscDiff  * time / hpetDiff;
		apicDiff = apicDiff * time / hpetDiff;
		
		avg_apic += apicDiff;
		avg_tsc  += tscDiff;
	}
	
	avg_apic /= nRuns * nMs;
	avg_tsc  /= nRuns * nMs;
	
	apicOut = avg_apic;
	tscOut  = avg_tsc;
}

// Despite being in the APIC namespace this also calibrates the TSC. Wow!
void APIC::CalibrateTimer(uint64_t &apicOut, uint64_t &tscOut)
{
	// Tell the APIC timer to use divider 16.
	APIC::WriteReg(APIC_REG_TMR_DIV_CFG, C_APIC_TIMER_DIVIDE_BY_16);
	
	if (g_PolledSleepFunc == HPET::PolledSleep)
	{
		return APIC::CalibrateHPET(apicOut, tscOut);
	}
	
	uint64_t avg_apic = 0;
	uint64_t avg_tsc  = 0;
	
	constexpr int nRuns = 16;
	constexpr int nMs   = 20;
	
	for (int i = 0; i < nRuns; i++)
	{
		uint64_t tscStart = TSC::Read();
		
		// Set APIC init counter to -1.
		APIC::WriteReg(APIC_REG_TMR_INIT_CNT, 0xFFFFFFFF);
		
		// Sleep for X ms.
		// subtract a small amount of time to compensate for the speed difference that also calibrating the TSC adds.
		g_PolledSleepFunc(nMs*1000*1000);
		
		uint64_t ticksPerMsTsc  = TSC::Read() - tscStart;
		
		// Stop the APIC timer.
		APIC::WriteReg(APIC_REG_LVT_TIMER, APIC_LVT_INT_MASKED);
		
		// Read the current count.
		uint64_t ticksPerMsApic = 0xFFFFFFFF - APIC::ReadReg(APIC_REG_TMR_CURR_CNT);
		//SLogMsg("CPU %d Run %d: %lld APIC ticks/ms, %lld TSC ticks/ms", CPU::GetCurrent()->ID(), i, ticksPerMsApic, ticksPerMsTsc);
		
		avg_apic += ticksPerMsApic;
		avg_tsc  += ticksPerMsTsc;
	}
	
	avg_apic /= nRuns * nMs;
	avg_tsc  /= nRuns * nMs;
	
	apicOut = avg_apic;
	tscOut  = avg_tsc;
}

void APIC::ScheduleInterruptIn(uint64_t nanoseconds)
{
	if (nanoseconds >= 1'000'000'000'000ULL)
		;//KernelPanic("nanoseconds value too big (%lld)", nanoseconds);
	
	uint64_t lvtTimerReg = 0;
	
	// bit 16: masked. That'll be 0
	
	// first 8 bits are the interrupt vector:
	lvtTimerReg |= IDT::INT_APIC_TIMER;
	
	// This is redundant since the oneshot mode's value is 0, but hey, this is for clarity
	lvtTimerReg |= C_APIC_TIMER_MODE_ONESHOT;
	
	CPU* pCpu = CPU::GetCurrent();
	
	bool bState = pCpu->SetInterruptsEnabled(false);
	
	// get the new timer value:
	uint64_t timerVal = pCpu->GetLapicTicksPerMS() * nanoseconds / C_MILLIS_TO_NANOS;
	
	// set the count:
	APIC::WriteReg(APIC_REG_TMR_INIT_CNT, timerVal);
	APIC::WriteReg(APIC_REG_LVT_TIMER, lvtTimerReg);
	
	// and off it goes !
	
	pCpu->SetInterruptsEnabled(bState);
}

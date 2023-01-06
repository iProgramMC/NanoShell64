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

#define C_SPURIOUS_INTERRUPT_VECTOR (0xFF)
#define C_APIC_REG_EOI      (0xB0)
#define C_APIC_REG_SPURIOUS (0xF0)

#define IA32_APIC_BASE_MSR (0x1B)

// Write a register.
void Arch::APIC::WriteReg(uint32_t reg, uint32_t value)
{
	WritePhys(GetLapicBasePhys() + reg, value);
}

// Read a register.
uint32_t Arch::APIC::ReadReg(uint32_t reg)
{
	return ReadPhys(GetLapicBasePhys() + reg);
}

uintptr_t Arch::APIC::GetLapicBasePhys()
{
	// Read the specific MSR.
	uintptr_t msr = ReadMSR(IA32_APIC_BASE_MSR);
	
	return msr & 0x0000'000F'FFFF'F000;
}

uintptr_t Arch::APIC::GetLapicBase()
{
	return GetHHDMOffset() + GetLapicBasePhys();
}

void Arch::APIC::EnsureOn()
{
	// Use the CPUID instruction.
	uint32_t eax, edx;
	ASM("cpuid":"=d"(edx),"=a"(eax):"a"(1));
	
	if (~edx & (1 << 9))
	{
		LogMsg("APIC is off. An APIC must be present before running NanoShell64.");
		Arch::IdleLoop();
	}
}

extern "C" void Arch_APIC_OnInterrupt_Asm();
extern "C" void Arch_APIC_OnInterrupt()
{
	Arch::APIC::OnInterrupt();
}

void Arch::APIC::OnInterrupt()
{
	WriteReg(C_APIC_REG_EOI, 0);
	LogMsg("APIC interrupt!");
}

void Arch::APIC::Init()
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
	CPU::GetCurrent()->SetInterruptGate(C_SPURIOUS_INTERRUPT_VECTOR, uintptr_t(Arch_APIC_OnInterrupt_Asm));
	
	// Enable the spurious vector interrupt.
	WriteReg(C_APIC_REG_SPURIOUS, C_SPURIOUS_INTERRUPT_VECTOR | 0x100);
}

void Arch::CPU::SendTestIPI()
{
	// The destination is 'this'. The sender (us) is 'pSenderCPU'.
	CPU * pSenderCPU = GetCurrent();
	
	// Wait for any pending IPIs to finish.
	
	
}

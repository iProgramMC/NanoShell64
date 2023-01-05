//  ***************************************************************
//  Arch.hpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#ifndef _ARCH_HPP
#define _ARCH_HPP

#include <stdint.h>
#include <stddef.h>
#include <_limine.h>

namespace Arch
{
	class CPUInfo
	{
		limine_smp_info* pSMPInfo;
	};
	
	// Waits until the next interrupt.
	void Halt();
	
	// This loop constantly idles. This is done so that, when the CPU is not
	// running any task, it can idle and not continue running on full throttle.
	void IdleLoop();
	
	// x86_64 architecture specific instructions.
	#ifdef TARGET_X86_64
	
	// Reads a single byte from an I/O port.
	uint8_t ReadByte(uint16_t port);
	
	// Writes a single byte to an I/O port.
	void WriteByte(uint16_t port, uint8_t data);
	
	// Clears interrupts for the current CPU.
	void DisableInterrupts();
	
	// Restore interrupts for the current CPU.
	void EnableInterrupts();
	
	// Writes to a model specific register.
	void WriteMSR(uint32_t msr, uint64_t value);
	
	// Writes to a model specific register.
	uint64_t ReadMSR(uint32_t msr);
	
	#endif
}

#endif//_ARCH_HPP
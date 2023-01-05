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

// TODO: Move this somewhere else.
inline void *operator new(size_t, void *p)     throw() { return p; }
inline void *operator new[](size_t, void *p)   throw() { return p; }
inline void  operator delete  (void *, void *) throw() { };
inline void  operator delete[](void *, void *) throw() { };

namespace Arch
{
	class CPU
	{
		/**** CPU specific variables ****/
	private:
		// The index of this processor.
		uint32_t m_processorID;
		
		// The SMP info we have been given.
		limine_smp_info* m_pSMPInfo;
		
		// Whether we are the bootstrap processor or not.
		bool m_bIsBSP;
		
		// Store other fields here such as current task, etc.
		
		
		/**** CPU specific operations ****/
	public:
		CPU(uint32_t processorID, limine_smp_info* pSMPInfo, bool bIsBSP) : m_processorID(processorID), m_pSMPInfo(pSMPInfo), m_bIsBSP(bIsBSP)
		{
		}
		
		// Start the CPU.
		void Go();
		
		/**** CPU agnostic operations ****/
	public:
		// Get the number of CPUs available to the system.
		static uint64_t GetCount();
		
		// Initialize the CPUs based on the Limine SMP response, from the bootstrap processor's perspective.
		static void InitAsBSP();
		
		// Get the SMP request's response.
		static limine_smp_response* GetSMPResponse();
		
		// Static function to initialize a certain CPU.
		static void Start(limine_smp_info* pInfo);
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
	
	// MSRs:
	enum eMSR
	{
		FS_BASE = 0xC0000100,
		GS_BASE = 0xC0000101,
		KERNEL_GS_BASE = 0xC0000102,
	};
	
	// Writes to a model specific register.
	void WriteMSR(uint32_t msr, uint64_t value);
	
	// Writes to a model specific register.
	uint64_t ReadMSR(uint32_t msr);
	
	#endif
}

#endif//_ARCH_HPP
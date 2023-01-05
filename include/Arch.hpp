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

#include <Nanoshell.hpp>
#include <_limine.h>

namespace Arch
{
#ifdef TARGET_X86_64
	
	#define C_IDT_ENTRIES
	
	struct TSS
	{
		uint32_t m_reserved;
		uint64_t m_rsp[3];
		uint64_t m_reserved1;
		uint64_t m_ist[7];
		uint64_t m_reserved2;
		uint16_t m_reserved3;
		uint16_t m_iopb;
	}
	PACKED;
	
	struct IDT
	{
		struct IDTEntry
		{
			uint64_t m_offsetLow  : 16;
			uint64_t m_segmentSel : 16;
			uint64_t m_ist        : 3;
			uint64_t m_reserved0  : 5;
			uint64_t m_gateType   : 4;
			uint64_t m_reserved1  : 1;
			uint64_t m_dpl        : 2;
			uint64_t m_present    : 1;
			uint64_t m_offsetHigh : 48;
			uint64_t m_reserved2  : 32;
		}
		PACKED;
		
		IDTEntry m_entries[256];
	};
	
	// The GDT structure. It contains an array of uint64s, which represents
	// each of the GDT entries, and potentially a TSS entry. (todo)
	struct GDT
	{
		// The segment numbers.
		enum
		{
			DESC_NULL,
			DESC_16BIT_CODE,
			DESC_16BIT_DATA,
			DESC_32BIT_CODE,
			DESC_32BIT_DATA,
			DESC_64BIT_RING0_CODE,
			DESC_64BIT_RING0_DATA,
			DESC_64BIT_RING3_CODE,
			DESC_64BIT_RING3_DATA,
		};
		
		uint64_t m_BasicEntries[9];
		
		TSS m_tss;
	};
	
	namespace APIC
	{
		// Ensure the APIC is supported by checking CPUID
		void EnsureOn();
	};
	
#endif
	
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
		
		// The GDT of this CPU.
		GDT m_gdt;
		
		// The IDT of this CPU.
		IDT m_idt;
		
		// Store other fields here such as current task, etc.
		
		
		/**** CPU specific operations ****/
	public:
		CPU(uint32_t processorID, limine_smp_info* pSMPInfo, bool bIsBSP) : m_processorID(processorID), m_pSMPInfo(pSMPInfo), m_bIsBSP(bIsBSP)
		{
#ifdef TARGET_X86_64
			ClearIDT();
#endif
		}
		
#ifdef TARGET_X86_64
		// Load the GDT.
		void LoadGDT();
		
		// Clear the IDT.
		void ClearIDT();
		
		// Load the IDT.
		// Note: Any ulterior changes should be done through an IPI to the CPU.
		void LoadIDT();
#endif
		
		// Setup the CPU.
		void Init();
		
		// Start the CPU's idle loop.
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
		
		// Get the current CPU.
		static CPU* GetCurrent();
	};
	
	// Waits until the next interrupt.
	void Halt();
	
	// This loop constantly idles. This is done so that, when the CPU is not
	// running any task, it can idle and not continue running on full throttle.
	void IdleLoop();
	
	// x86_64 architecture specific functions.
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
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
#include <MemoryManager.hpp>
#include <_limine.h>

namespace Arch
{
#ifdef TARGET_X86_64
	
	#define C_IDT_ENTRIES
	
	struct TSS
	{
		uint32_t m_reserved;
		uint64_t m_rsp[3];    // RSP 0-2
		uint64_t m_reserved1;
		uint64_t m_ist[7];    // IST 1-7
		uint64_t m_reserved2;
		uint16_t m_reserved3;
		uint16_t m_iopb;
	}
	PACKED;
	
	// The GDT structure. It contains an array of uint64s, which represents
	// each of the GDT entries, and potentially a TSS entry. (todo)
	struct GDT
	{
		// The segment numbers.
		enum
		{
			DESC_NULL             = 0x00,
			DESC_16BIT_CODE       = 0x08,
			DESC_16BIT_DATA       = 0x10,
			DESC_32BIT_CODE       = 0x18,
			DESC_32BIT_DATA       = 0x20,
			DESC_64BIT_RING0_CODE = 0x28,
			DESC_64BIT_RING0_DATA = 0x30,
			DESC_64BIT_RING3_CODE = 0x38,
			DESC_64BIT_RING3_DATA = 0x40,
		};
		
		uint64_t m_BasicEntries[9];
		
		TSS m_tss;
	};
	
	struct IDT
	{
		enum
		{
			INT_PAGE_FAULT = 0x0E,
			INT_IPI        = 0xF0,
		};
		
		struct Entry
		{
			// Byte 0, 1
			uint64_t m_offsetLow  : 16;
			// Byte 2, 3
			uint64_t m_segmentSel : 16;
			// Byte 4
			uint64_t m_ist        : 3;
			uint64_t m_reserved0  : 5;
			// Byte 5
			uint64_t m_gateType   : 4;
			uint64_t m_reserved1  : 1;
			uint64_t m_dpl        : 2;
			uint64_t m_present    : 1;
			// Byte 6, 7, 8, 9, 10, 11
			uint64_t m_offsetHigh : 48;
			// Byte 12, 13, 14, 15
			uint64_t m_reserved2  : 32;
			
			Entry() = default;
			
			Entry(uintptr_t handler, uint8_t ist = 0, uint8_t dpl = 0)
			{
				memset(this, 0, sizeof *this);
				
				m_offsetLow  = handler & 0xFFFF;
				m_offsetHigh = handler >> 16;
				m_gateType   = 0xE;
				m_dpl        = dpl;
				m_ist        = ist;
				m_present    = handler != 0;
				m_segmentSel = GDT::DESC_64BIT_RING0_CODE;
			}
		}
		PACKED;
		
		Entry m_entries[256];
		
		void SetEntry(uint8_t iv, Entry entry)
		{
			m_entries[iv] = entry;
		}
	};
	
	namespace APIC
	{
		// Ensure the APIC is supported by checking CPUID
		void EnsureOn();
		
		// Initialize the APIC for this CPU.
		void Init();
		
		// Write a register.
		void WriteReg(uint32_t reg, uint32_t value);
		
		// Read a register.
		uint32_t ReadReg(uint32_t reg);
		
		// Get the LAPIC's base address. This is not offset by the HHDM.
		uintptr_t GetLapicBasePhys();
		
		// Get the LAPIC's base address. This is offset by the HHDM.
		uintptr_t GetLapicBase();
		
		// On Interrupt.
		void OnInterrupt();
	};
	
#endif
	
	class CPU
	{
		static constexpr size_t C_INTERRUPT_STACK_SIZE = 8192;
		
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
		
		// The main page mapping.
		VMM::PageMapping* m_pPageMap;
		
		// The interrupt handler stack.
		void* m_pIsrStack;
		
		// Store other fields here such as current task, etc.
		
		/**** Private CPU object functions. ****/
	private:
		
#ifdef TARGET_X86_64
		// Load the GDT.
		void LoadGDT();
		
		// Clear the IDT.
		void ClearIDT();
		
		// Load the IDT.
		// Note: Any ulterior changes should be done through an IPI to the CPU.
		void LoadIDT();
#endif
		/**** Operations that should be run within this CPU's context, but are otherwise public ****/
	public:
		// The function called when we're inside of a page fault.
		void OnPageFault(Registers* pRegs);
		
		// Setup the CPU.
		void Init();
		
		// Start the CPU's idle loop.
		void Go();
		
		// Set an interrupt gate.
		void SetInterruptGate(uint8_t intNum, uintptr_t fnHandler, uint8_t ist = 0, uint8_t dpl = 0);
		
		/**** Operations that can be performed on a CPU object from anywhere. ****/
	public:
		CPU(uint32_t processorID, limine_smp_info* pSMPInfo, bool bIsBSP) : m_processorID(processorID), m_pSMPInfo(pSMPInfo), m_bIsBSP(bIsBSP)
		{
#ifdef TARGET_X86_64
			ClearIDT();
#endif
		}
		
		// Get whether this CPU is the bootstrap CPU.
		bool IsBootstrap() const
		{
			return m_bIsBSP;
		}
		
		// Send this CPU an IPI.
		void SendTestIPI();
		
		/**** CPU agnostic operations ****/
	public:
		// Get the number of CPUs available to the system.
		static uint64_t GetCount();
		
		// Initialize the CPUs based on the Limine SMP response, from the bootstrap processor's perspective.
		static void InitAsBSP();
		
		// Get the SMP request's response.
		static limine_smp_response* GetSMPResponse();
		
		// Get the HHDM request's response.
		static limine_hhdm_response* GetHHDMResponse();
		
		// Static function to initialize a certain CPU.
		static void Start(limine_smp_info* pInfo);
		
		// Get the current CPU.
		static CPU* GetCurrent();
		
		// Get the CPU with the specified processor ID.
		static CPU* GetCPU(uint64_t pid);
		
		// Relating to this, check if the current processor is the BSP.
		static bool AreWeBootstrap()
		{
			return GetCurrent()->IsBootstrap();
		}
	};
	
	// Waits until the next interrupt.
	void Halt();
	
	// This loop constantly idles. This is done so that, when the CPU is not
	// running any task, it can idle and not continue running on full throttle.
	void IdleLoop();
	
	// x86_64 architecture specific functions.
#ifdef TARGET_X86_64
	// Gets the contents of CR3.
	uintptr_t ReadCR3();
	
	// Sets the contents of CR3.
	void WriteCR3(uintptr_t cr3);
	
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
	
	// Get the HHDM offset (higher half direct map).
	uintptr_t GetHHDMOffset();
	
	// Invalidate a part of the TLB cache.
	void Invalidate(uintptr_t addr);
	
	// Writes to a model specific register.
	void WriteMSR(uint32_t msr, uint64_t value);
	
	// Writes to a model specific register.
	uint64_t ReadMSR(uint32_t msr);
	
	// Write a 32-bit integer to any address within physical memory.
	// This assumes an HHDM is present and the entire physical address space is mapped.
	void WritePhys(uintptr_t ptr, uint32_t thing);
	
	// Read a 32-bit integer from any address within physical memory.
	// This assumes an HHDM is present and the entire physical address space is mapped.
	uint32_t ReadPhys(uintptr_t ptr);
	
#endif
}

#endif//_ARCH_HPP
//  ***************************************************************
//  MemoryManager.hpp - Creation date: 07/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  File description:
//      This file contains header definitions for the memory
//    manager.
//
//  ***************************************************************
#ifndef _MEMORY_MANAGER_HPP
#define _MEMORY_MANAGER_HPP

// Address Layout:
// 0x0000'0000'0000'0000 - 0x0000'FFFF'FFFF'FFFF: User mappable memory region.
// 0x0001'0000'0000'0000 - 0xFFFE'FFFF'FFFF'FFFF: Non-canonical address gap.
// 0xFFFF'8000'0000'0000 - 0xFFFF'EFFF'FFFF'FFFF: The HHDM mapping. This part and the ones below will have their PML4's verbatim copied
// 0xFFFF'F000'0000'0000 - 0xFFFF'FFFE'FFFF'FFFF: The kernel heap.
// 0xFFFF'FFFF'0000'0000 - 0xFFFF'FFFF'FFFF'FFFF: The kernel itself.

#include <Nanoshell.hpp>

constexpr uint64_t PAGE_SIZE = 4096;

namespace PMM
{
	constexpr uintptr_t INVALID_PAGE = 0;
	
	struct PageFreeListNode
	{
		PageFreeListNode *pPrev, *pNext;
	};
	
	struct MemoryArea
	{
		// The link to the next MemoryArea entry.
		MemoryArea* m_pLink;
		// The start of the physical memory this bitmap represents.
		uintptr_t m_startAddr;
		// The length (in pages) of this PMM bitmap.
		size_t    m_length;
		// The amount of pages free. This should always be kept in sync.
		size_t    m_freePages;
		
		PageFreeListNode *m_pFirst, *m_pLast;
		
		MemoryArea(uintptr_t start, size_t len) : m_pLink(nullptr), m_startAddr(start), m_length(len), m_freePages(len)
		{
		}
		
		uintptr_t RemoveFirst();
		void PushBack(uintptr_t paddr);
	};
	
	// Get the total amount of pages available to the system. Never changes after init.
	uint64_t GetTotalPages();
	
	// Initializes the PMM using the Limine memory map request.
	// This function must be run on the bootstrap CPU.
	void Init();
	
	// Allocate a new page within the PMM.
	// Note: The hint reference should be treated as an opaque value and is designed for quick successive allocations.
	//uintptr_t AllocatePage(uint64_t& hint);
	uintptr_t AllocatePage();
	
	// Free a page within the PMM.
	void FreePage(uintptr_t page);
	
	// Test out the PMM.
	void Test();
}

namespace VMM
{
	// The PML4 indices of the memory regions.
	enum ePml4Limit
	{
		P_USER_START  = 0x000,
		P_USER_END    = 0x100,
		P_KERN_START  = 0x100,
		P_HHDM_START  = 0x100,
		P_HHDM_END    = 0x1D0,
		P_KHEAP       = 0x1D0, // one page is enough I would think.
		P_KERNEL_PML4 = 0x1FF,
		P_KERN_END    = 0x200,
	};
	
	// Represents a single page entry.
	union PageEntry
	{
		struct
		{
			bool m_present       : 1; // bit 0
			bool m_readWrite     : 1; // bit 1
			bool m_supervisor    : 1; // bit 2
			bool m_writeThrough  : 1; // bit 3
			bool m_cacheDisable  : 1; // bit 4
			bool m_accessed      : 1; // bit 5
			bool m_dirty         : 1; // bit 6
			bool m_pat           : 1; // bit 7
			bool m_global        : 1; // bit 8
			// 3 available bits.
			bool m_partOfPmm     : 1; // bit 9:  If this bit is set, this is a part of the PMM.
			bool m_needAllocPage : 1; // bit 10: If this bit is set, we will want to place a new address into the address field on page fault.
			bool m_pad           : 1; // bit 11: If this bit is set, upon a page fault, pad this with (protKey << 4 | protKey) bytes.
			uint64_t m_address   : 40;// bits 12-51 (MAXPHYADDR)
			int  m_available1    : 7; // bits 52-58 (ignored)
			int  m_protKey       : 4; // bits 59-62 (protection key, ignores unless CR4.PKE or CR4.PKS is set and this is a page tree leaf)
			bool m_execDisable   : 1; // bit 63: Disable execution from this page.
		};
		uint64_t m_data;
		
		PageEntry() = default;
		PageEntry(uint64_t addr, bool rw, bool us, bool xd, bool pop, bool nap, bool pr = true,
		          int pk = 0, bool wt = false, bool cd = false, bool pat = false, bool glb = false) // useless stuff
		{
			m_address       = addr >> 12;
			m_readWrite     = rw;
			m_present       = pr;
			m_supervisor    = us;
			m_execDisable   = xd;
			m_partOfPmm     = pop;
			m_needAllocPage = nap;
			m_writeThrough  = wt;
			m_cacheDisable  = cd;
			m_pat           = pat;
			m_global        = glb;
			m_protKey       = pk;
		}
	};
	
	struct PageTable
	{
		PageEntry m_entries[512];
		
		// Gets the page entry pointer as a virtual address.
		PageEntry* GetPageEntry(int index)
		{
			return &m_entries[index];
		}
		
		// Clone the page table.
		PageTable* Clone();
	};
	
	struct PageDirectory
	{
		PageEntry m_entries[512];
		
		// Gets the page table pointer as a virtual address.
		PageTable* GetPageTable(int index);
		
		// Clones the page directory.
		PageDirectory* Clone();
	};
	
	struct PML3 // PDPT
	{
		PageEntry m_entries[512];
		
		// Gets the page table pointer as a virtual address.
		PageDirectory* GetPageDirectory(int index);
		
		// Clones the PML3.
		PML3* Clone();
	};
	
	struct PageMapping
	{
		PageEntry m_entries[512];
		
		// Gets the current page mapping from the CR3.
		static PageMapping* GetFromCR3();
		
		// Gets the PML3 pointer as a virtual address.
		PML3* GetPML3(int index);
		
		// Clones a page mapping.
		PageMapping* Clone(bool keepLowerHalf = true);
		
		// Switches the executing CPU to use this page mapping.
		void SwitchTo();
		
		// Set a page mapping's page entry at a particular address.
		bool MapPage(uintptr_t addr, const PageEntry & pe);
		
		// Map a new page in. For now, not demand paged -- we need to wait until we add interrupts.
		bool MapPage(uintptr_t addr, bool rw = true, bool super = false, bool xd = true);
		
		// Removes a page mapping, and any now empty levels that it resided in.
		void UnmapPage(uintptr_t addr, bool removeUpperLevels = true);
	};
}

#endif//_MEMORY_MANAGER_HPP
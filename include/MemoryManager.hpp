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
// 0x0000000000000000 - 0x0000EFFFFFFFFFFF: User mappable memory region.
// 0x0000F00000000000 - 0x0000FFFFFFFFFFFF: User mappable memory region.
// 0x0001000000000000 - 0xFFFEFFFFFFFFFFFF: Non-canonical address gap.
// 0xFFFF000000000000 - 0xFFFFFFFFFFFFFFFF: The kernel and HHDM mapping. This part will have its PML4's verbatim copied

#include <Nanoshell.hpp>

constexpr uint64_t PAGE_SIZE = 4096;

namespace PMM
{
	constexpr uintptr_t INVALID_PAGE = 0;
	
	struct BitmapPart
	{
		// The link to the next BitmapPart entry.
		BitmapPart* m_pLink;
		// The start of the physical memory this bitmap represents.
		uintptr_t m_startAddr;
		// The length (in pages) of this PMM bitmap.
		size_t    m_length;
		// The amount of pages free. This should always be kept in sync.
		size_t    m_freePages;
		
		// The bits themselves. This is a dynamic array.
		uint64_t  m_bits[];
		
		BitmapPart(uintptr_t start, size_t len) : m_pLink(nullptr), m_startAddr(start), m_length(len), m_freePages(len)
		{
		}
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
	// Represents a single page entry.
	union PageEntry
	{
		struct
		{
			bool m_present      : 1;
			bool m_readWrite    : 1;
			bool m_supervisor   : 1;
			bool m_writeThrough : 1;
			bool m_cacheDisable : 1;
			bool m_accessed     : 1;
			bool m_dirty        : 1;
			bool m_pat          : 1;
			bool m_global       : 1;
			// 3 available bits.
			bool m_partOfPmm    : 1; // If this bit is set, this is a part of the PMM.
			bool m_needAllocPage: 1; // If this bit is set, we will want to place a new address into the address field on page fault.
			bool m_available0   : 1;
			uint64_t m_address  : 40;
			int  m_available1   : 7;
			int  m_protKey      : 4;
			bool m_execDisable  : 1;
		};
		uint64_t m_data;
	};
	
	struct PageTable
	{
		PageEntry m_entries[512];
		
		// Gets the page entry pointer as a virtual address.
		PageEntry* GetPageEntry(int index)
		{
			return &m_entries[index];
		}
	};
	
	struct PageDirectory
	{
		PageEntry m_entries[512];
		
		// Gets the page table pointer as a virtual address.
		PageTable* GetPageTable(int index);
	};
	
	struct PML3 // PDPT
	{
		PageEntry m_entries[512];
		
		// Gets the page table pointer as a virtual address.
		PageDirectory* GetPageDirectory(int index);
	};
	
	struct PML4
	{
		PageEntry m_entries[512];
		
		// Gets the page table pointer as a virtual address.
		PML3* GetPML3(int index);
	};
	
	struct PageMapping
	{
		PageEntry m_entries[512];
		
		// Gets the PML4 pointer as a virtual address.
		PML4* GetPML4(int index);
		
		// Gets the current page mapping from the CR3.
		static PageMapping* GetFromCR3();
		
		// Clones a page mapping.
		static PageMapping* Clone(PageMapping* pPageMapping);
		
		// Switches the executing CPU to use this page mapping.
		void SwitchTo();
	};
}

#endif//_MEMORY_MANAGER_HPP
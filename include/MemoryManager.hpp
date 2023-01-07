//  ***************************************************************
//  APIC.cpp - Creation date: 05/01/2023
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

#include <Arch.hpp>

constexpr uint64_t PAGE_SIZE = 4096;

namespace PhysicalMM
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
};

#endif//_MEMORY_MANAGER_HPP
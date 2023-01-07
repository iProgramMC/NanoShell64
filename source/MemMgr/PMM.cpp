//  ***************************************************************
//  PMM.cpp - Creation date: 07/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements a thread safe physical memory
//  manager.
//
//  ***************************************************************
#include <_limine.h>
#include <Arch.hpp>
#include <Spinlock.hpp>
#include <EternalHeap.hpp>

volatile limine_memmap_request g_MemMapRequest =
{
	.id       = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
	.response = NULL,
};

static PMM::BitmapPart* s_pFirstBMPart, *s_pLastBMPart;
static Spinlock         s_PMMSpinlock;
static uint64_t         s_totalAvailablePages; // The total amount of pages available to the system.

// Add a new BitmapPart entry. This may only be called during initialization.
// The reason I did it like this is because I really don't want to publicize this function.
namespace PMM
{

void AddBitmapPart(BitmapPart* pPart)
{
	if (s_pFirstBMPart == NULL)
	{
		s_pFirstBMPart = s_pLastBMPart = pPart;
		pPart->m_pLink = NULL;
	}
	
	s_pLastBMPart->m_pLink = pPart;
	s_pLastBMPart          = pPart;
}

};

uint64_t PMM::GetTotalPages()
{
	return s_totalAvailablePages;
}

void PMM::Init()
{
	// Check if we have the memmap response.
	if (!g_MemMapRequest.response)
	{
		// Just hang..
		LogMsg("No physical memory map response was given by limine.  Halting");
		Arch::IdleLoop();
	}
	
	// For each memory map entry.
	auto resp = g_MemMapRequest.response;
	for (uint64_t i = 0; i != resp->entry_count; i++)
	{
		auto entry = resp->entries[i];
		
		// If this entry is marked as usable...
		if (entry->type != LIMINE_MEMMAP_USABLE)
		{
			// TODO: maybe we should handle what's in here?
			continue;
		}
		
		// Check how many 64-bit ints we need to allocate for this entry.
		size_t nBits = (entry->length + PAGE_SIZE - 1) / PAGE_SIZE;
		
		// TODO OPTIMIZE: Maybe don't allocate everything just yet?
		size_t sz = (nBits + 7) / 8 * sizeof(uint64_t) + sizeof(BitmapPart);
		void* pMem = EternalHeap::Allocate(sz);
		memset(pMem, 0, sz);
		
		BitmapPart* pPart = new(pMem) BitmapPart(entry->base, nBits);
		
		s_totalAvailablePages += pPart->m_length;
		
		AddBitmapPart(pPart);
	}
}

uintptr_t PMM::AllocatePage()
{
	LockGuard lg (s_PMMSpinlock);
	
	// browse through the whole BitmapPart chain
	BitmapPart* bmp = s_pFirstBMPart;
	while (bmp)
	{
		// Look through all the bits.
		size_t nBitsArrLen = (bmp->m_length + 7) / 8;
		
		for (uint64_t i = 0; i < nBitsArrLen; i++)
		{
			const uint64_t bit = bmp->m_bits[i];
			
			// If there are no bits free here, skip
			if (bit == ~0ULL) continue;
			
			// Which bit was zero?
			for (uint64_t j = 0; j < 64; j++)
			{
				if (~bit & (1 << j))
				{
					bmp->m_bits[i] |= (1 << j);
					// Return this page.
					return bmp->m_startAddr + PAGE_SIZE * (i * 64 + j);
				}
			}
		}
		
		bmp = bmp->m_pLink;
	}
	
	return INVALID_PAGE;
}

void PMM::FreePage(uintptr_t page)
{
	LockGuard lg (s_PMMSpinlock);
	
	// Find the bitmap part where this page resides;
	BitmapPart* bmp = s_pFirstBMPart;
	while (bmp)
	{
		if (bmp->m_startAddr <= page) break;
		
		bmp = bmp->m_pLink;
	}
	
	if (!bmp)
	{
		SLogMsg("Error, trying to free page %p, not within any of our bitmap parts", page);
		return;
	}
	
	// just clear the bit
	page -= bmp->m_startAddr;
	page /= PAGE_SIZE;
	
	uint64_t i, j;
	i = page / 64;
	j = page % 64;
	
	bmp->m_bits[i] &= ~(1 << j);
}

void PMM::Test()
{
	// The two printed addresses should ideally be the same.
	uintptr_t addr = PMM::AllocatePage();
	LogMsg("Addr: %p", addr);
	PMM::FreePage(addr);
	
	addr = PMM::AllocatePage();
	LogMsg("Addr: %p", addr);
	PMM::FreePage(addr);
}

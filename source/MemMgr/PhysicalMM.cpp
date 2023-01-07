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
#include <_limine.h>
#include <Arch.hpp>
#include <MemoryManager.hpp>
#include <Spinlock.hpp>
#include <EternalHeap.hpp>

volatile limine_memmap_request g_MemMapRequest =
{
	.id       = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
	.response = NULL,
};

static PhysicalMM::BitmapPart* s_pFirstBMPart, *s_pLastBMPart;
static Spinlock                s_PMMSpinlock;
static uint64_t                s_totalAvailablePages; // The total amount of pages available to the system.

// Add a new BitmapPart entry. This may only be called during initialization.
// The reason I did it like this is because I really don't want to publicize this function.
namespace PhysicalMM
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

uint64_t PhysicalMM::GetTotalPages()
{
	return s_totalAvailablePages;
}

void PhysicalMM::Init()
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
		
		void* pMem = EternalHeap::Allocate((nBits + 7) / 8 * sizeof(uint64_t) + sizeof(BitmapPart));
		
		BitmapPart* pPart = new(pMem) BitmapPart(entry->base, nBits);
		
		s_totalAvailablePages += pPart->m_length;
		
		AddBitmapPart(pPart);
	}
}


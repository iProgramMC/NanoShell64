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

static PMM::MemoryArea* s_pFirstBMPart, *s_pLastBMPart;
static Spinlock         s_PMMSpinlock;
static uint64_t         s_totalAvailablePages; // The total amount of pages available to the system.

// The reason I put these in a "namespace PMM" block is because I don't want to publicize these functions.
namespace PMM
{

uintptr_t MemoryArea::RemoveFirst()
{
	PageFreeListNode * pFirst = m_pFirst;
	
	// remove it
	if (pFirst->pNext)
		pFirst->pNext->pPrev = NULL;
	m_pFirst = pFirst->pNext;
	if (m_pLast == pFirst)
		m_pLast =  NULL;
	
	m_freePages--;
	
	return (uintptr_t)pFirst - Arch::GetHHDMOffset();
}

void MemoryArea::PushBack(uintptr_t paddr)
{
	PageFreeListNode* pLast = m_pLast, *pThis = (PageFreeListNode*)(Arch::GetHHDMOffset() + paddr);
	
	pThis->pNext = NULL;
	pThis->pPrev = pLast;
	if (pLast)
		pLast->pNext = pThis;
	m_pLast = pThis;
	m_freePages++;
}

// Initialize a MemoryArea free list.
void InitializeMemoryArea(MemoryArea* pPart)
{
	uintptr_t ho = Arch::GetHHDMOffset();
	uintptr_t physAddr = pPart->m_startAddr + ho;
	
	PageFreeListNode* first = (PageFreeListNode*)physAddr;
	
	for (size_t sz = 0; sz != pPart->m_length; sz++, physAddr += PAGE_SIZE)
	{
		((PageFreeListNode*)physAddr)->pPrev = (PageFreeListNode*)(physAddr - PAGE_SIZE);
		((PageFreeListNode*)physAddr)->pNext = (PageFreeListNode*)(physAddr + PAGE_SIZE);
	}
	
	PageFreeListNode* last = (PageFreeListNode*)(physAddr - PAGE_SIZE);
	last ->pNext = NULL;
	first->pPrev = NULL;
	
	pPart->m_pFirst = first;
	pPart->m_pLast  = last;
	
	// ensure continuity
	/*
	PageFreeListNode* pNode = (PageFreeListNode*)(ho + pPart->m_startAddr);
	
	while (pNode)
	{
		SLogMsg("Node: %p", pNode);
		pNode = pNode->pNext;
	}
	*/
}

// Add a new MemoryArea entry. This may only be called during initialization.
void AddMemoryArea(MemoryArea* pPart)
{
	if (s_pFirstBMPart == NULL)
	{
		s_pFirstBMPart = s_pLastBMPart = pPart;
		pPart->m_pLink = NULL;
	}
	
	s_pLastBMPart->m_pLink = pPart;
	s_pLastBMPart          = pPart;
	
	InitializeMemoryArea(pPart);
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
		
		// Check how many pages we have.
		size_t nPages = (entry->length + PAGE_SIZE - 1) / PAGE_SIZE;
		
		// TODO: Assert that entry->base is page aligned
		void* pMem = EternalHeap::Allocate(sizeof(MemoryArea));
		memset(pMem, 0, sizeof(MemoryArea));
		
		MemoryArea* pPart = new(pMem) MemoryArea(entry->base, nPages);
		
		s_totalAvailablePages += pPart->m_length;
		
		AddMemoryArea(pPart);
	}
}

uintptr_t PMM::AllocatePage()
{
	LockGuard lg (s_PMMSpinlock);
	
	// browse through the whole MemoryArea chain
	MemoryArea* bmp = s_pFirstBMPart;
	while (bmp)
	{
		// if there are no free list nodes
		if (bmp->m_pFirst == NULL || bmp->m_pLast == NULL || bmp->m_freePages == 0)
		{
			bmp = bmp->m_pLink;
			continue;
		}
		
		// there is one. Remove it and return.
		return bmp->RemoveFirst();
	}
	
	return INVALID_PAGE;
}

void PMM::FreePage(uintptr_t page)
{
	LockGuard lg (s_PMMSpinlock);
	
	// Find the bitmap part where this page resides;
	MemoryArea* bmp = s_pFirstBMPart;
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
	
	// Append it to the last freelist index.
	bmp->PushBack(page);
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

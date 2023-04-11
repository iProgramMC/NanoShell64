//  ***************************************************************
//  KernelHeap.cpp - Creation date: 27/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//
//      This module implements the NanoShell64 kernel heap.
//
//      The kernel heap is a place in memory where allocations
//   wherein no particular property needs to be fulfilled, such
//   as page boundary alignment.
//      If page alignment is required we usually only need to
//   allocate one page at a time, so use the PMM directly + the
//   HHDM mapping.
//
//  ***************************************************************
#include <Arch.hpp>
#include <MemoryManager.hpp>

#define DEBUG_KERNEL_HEAP

using namespace VMM;

static Spinlock s_KernelHeapLock;

static KernelHeap::FreeListNode *s_FLStart, *s_FLEnd;

void KernelHeap::Init()
{
	LockGuard lg(s_KernelHeapLock);
	
	PageMapping* pPM = PageMapping::GetFromCR3();
	
	// map the kernel heap in.
	for (uint64_t i = 0; i < C_KERNEL_HEAP_SIZE; i += PAGE_SIZE)
	{
		pPM->MapPage(C_KERNEL_HEAP_START + i, true, true);
	}
	
	// setup the first free list block
	s_FLStart = (FreeListNode*)C_KERNEL_HEAP_START;
	s_FLStart->m_magic = FreeListNode::FLN_MAGIC;
	s_FLStart->m_size  = C_KERNEL_HEAP_SIZE - sizeof(FreeListNode);
	s_FLStart->m_next  = nullptr;
	s_FLStart->m_prev  = nullptr;
	s_FLEnd = s_FLStart;
}

void* KernelHeap::Allocate(size_t sz)
{
	// align our size to 16 bytes.
	sz = (sz + 15) & ~15;
	
	LockGuard lg(s_KernelHeapLock);
	
	FreeListNode* pNode = s_FLStart;
	FreeListNode* pBFN = nullptr;
	while (pNode)
	{
		// skip any candidate that won't fit our size...
		if (pNode->m_size < sz)
		{
			pNode = pNode->m_next;
			continue;
		}
		
		// or isn't free
		if (pNode->m_magic != FreeListNode::FLN_MAGIC)
		{
			#ifdef DEBUG_KERNEL_HEAP
			// make sure that our kernel heap ain't corrupted or anything
			if (pNode->m_magic != FreeListNode::FLA_MAGIC)
			{
				SLogMsg("ERROR: kernel heap corruption detected at %p. Magic: %p", pNode, pNode->m_magic);
			}
			#endif
			
			pNode = pNode->m_next;
			continue;
		}
		
		if (!pBFN || pBFN->m_size > pNode->m_size)
			pBFN = pNode;
		
		// if the node's size is more than 16 times our original size,
		// this is a great fit. Otherwise, continue trying for a better fit.
		if (pBFN->m_size >= 16 * sz)
			break;
		
		pNode = pNode->m_next;
	}
	
	if (!pBFN)
	{
		// oops. we ran out of kernel heap space
		return nullptr;
	}
	
	void *pArea = pBFN->GetArea();
	
	size_t old_size = pBFN->m_size;
	pBFN->m_magic = FreeListNode::FLA_MAGIC;
	
	// do we need to create an auxiliary node?
	if (pBFN->m_size >= sizeof(FreeListNode) + 32)
	{
		pBFN->m_size  = sz;
		FreeListNode* pAfter = pBFN->GetPtrDirectlyAfter();
		pAfter->m_magic = FreeListNode::FLN_MAGIC;
		pAfter->m_next  = pBFN->m_next;
		pAfter->m_prev  = pBFN;
		pAfter->m_size  = old_size - sizeof(FreeListNode) - sz;
		if (pBFN->m_next) pBFN->m_next->m_prev = pAfter;
		pBFN->m_next    = pAfter;
	}
	else
	{
		// simply mark it as allocated, it's fine.
	}
	
	return pArea;
}

void KernelHeap::Free(void* pArea)
{
	LockGuard lg(s_KernelHeapLock);
	
	FreeListNode* pNode = (FreeListNode*)pArea - 1;
	if (pNode->m_magic != FreeListNode::FLA_MAGIC)
	{
		// uh oh! Well, at least we were able to catch this, so just return.
		SLogMsg("ERROR: attempt to free region %p from kernel heap that wasn't actually allocated (its magic number is %p, a free nodes' is %p)", pArea, pNode->m_magic, FreeListNode::FLN_MAGIC);
		return;
	}
	
	// mark this as free
	pNode->m_magic = FreeListNode::FLN_MAGIC;
	
	// you may ask, "iProgram, why are you doing a for loop for 2 iterations here?"
	// the answer is very simple. The first iteration merges the actual node with its
	// 'next' neighbour. Afterwards, it changes the pNode variable to the pNode's previous
	// neighbour. If that node's not free, we simply return, but otherwise, we try to merge
	// with the next neighbor (which should be the node we are trying to merge with its
	// neighbors).
	for (int i = 0; i < 2; i++)
	{
		if (pNode->m_magic != FreeListNode::FLN_MAGIC) return;
		
		// Attempt to connect this with other nodes.
		if (pNode->m_next && pNode->m_next->m_magic == FreeListNode::FLN_MAGIC)
		{
			// merge this and the next together.
			pNode->m_size = ((uint8_t*)pNode->m_next - (uint8_t*)pNode) + pNode->m_next->m_size;
			
			pNode->m_next->m_magic = 0;
			
			pNode->m_next = pNode->m_next->m_next;
			if (pNode->m_next)
				pNode->m_next->m_prev = pNode;
		}
		
		pNode = pNode->m_prev;
		if (!pNode) return;
	}
}

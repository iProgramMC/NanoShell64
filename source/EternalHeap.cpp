//  ***************************************************************
//  EternalHeap.cpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include <Nanoshell.hpp>
#include <EternalHeap.hpp>
#include <Spinlock.hpp>

// The eternal heap is a small (4Mib) block of memory which allows very small and
// permanent blocks of memory to be given out during the initialization process.

#define C_ETERNAL_HEAP_SIZE (4 * 1024 * 1024)

static Spinlock gEternalHeapSpinlock;
static uint8_t  gEternalHeap[C_ETERNAL_HEAP_SIZE];
static uint8_t *gEternalHeapPtr = gEternalHeap;
static uint8_t * const gEternalHeapEnd = &gEternalHeap[C_ETERNAL_HEAP_SIZE];

void *EternalHeap::Allocate(size_t sz)
{
	LockGuard grd(gEternalHeapSpinlock);
	
	if (gEternalHeap + sz > gEternalHeapEnd)
	{
		// OOPS! We failed to allocate this block. Return NULL.
		LogMsg("Eternal heap failed to allocate. Oops");
		return NULL;
	}
	
	void *pMem = gEternalHeapPtr;
	
	gEternalHeapPtr += sz;
	
	return pMem;
}


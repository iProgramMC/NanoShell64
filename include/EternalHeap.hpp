//  ***************************************************************
//  EternalHeap.hpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

// The eternal heap is a small (1Mib) block of memory which allows very small and
// permanent blocks of memory to be given out during the initialization process.

#ifndef _ETERNAL_HEAP_HPP
#define _ETERNAL_HEAP_HPP

#include <stdint.h>
#include <stddef.h>

namespace EternalHeap
{
	// Permanently allocates a block of memory.
	void * Allocate(size_t sz);
};

#endif//_ETERNAL_HEAP_HPP

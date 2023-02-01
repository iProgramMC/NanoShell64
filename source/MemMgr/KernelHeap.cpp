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

static Spinlock s_KernelHeapLock;




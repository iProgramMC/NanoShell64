//  ***************************************************************
//  TSC.cpp - Creation date: 13/05/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2023 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements a small driver for the TSC.
//
//  ***************************************************************
#include <Arch.hpp>

using namespace Arch;

uint64_t TSC::Read()
{
	uint64_t low, high;
	
	// note: The rdtsc instruction is specified to zero out the top 32 bits of rax and rdx.
	asm("rdtsc":"=a"(low), "=d"(high));
	
	// So something like this is fine.
	return high << 32 | low;
}

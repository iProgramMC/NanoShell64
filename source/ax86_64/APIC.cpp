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
#include <Arch.hpp>
#include <Terminal.hpp>

void Arch::APIC::EnsureOn()
{
	// Use the CPUID instruction.
	uint32_t eax, edx;
	ASM("cpuid":"=d"(edx),"=a"(eax):"a"(1));
	
	if (~edx & (1 << 9))
	{
		LogMsg("APIC is off. An APIC must be present before running NanoShell64.");
		Arch::IdleLoop();
	}
}

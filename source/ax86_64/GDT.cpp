//  ***************************************************************
//  GDT.cpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements the GDT loader.
//
//  ***************************************************************
#include <Arch.hpp>

// This GDT will be cloned by every CPU, so that each CPU can have its own TSS.
static UNUSED Arch::GDT g_InitialGDT =
{
	{
		0x0000000000000000, // Null descriptor
		0x00009a000000ffff, // 16-bit code
		0x000093000000ffff, // 16-bit data
		0x00cf9a000000ffff, // 32-bit ring-0 code
		0x00cf93000000ffff, // 32-bit ring-0 data
		0x00af9b000000ffff, // 64-bit ring-0 code
		0x00af93000000ffff, // 64-bit ring-0 data
		0x00affb000000ffff, // 64-bit ring-3 code
		0x00aff3000000ffff, // 64-bit ring-3 data
	},
	{
		0,                         // reserved0
		{ 0, 0, 0 },               // RSP0-2
		0,                         // reserved1
		{ 0, 0, 0, 0, 0, 0, 0 },   // IST1-7
		0,                         // reserved2
		0,                         // reserved 3
		0xFFFF,                    // I/O bitmap offset. Setting it to a value like this means that we do not have an IOBP.
	},
};

// This function is responsible for loading the GDT for this CPU.
void Arch::CPU::LoadGDT()
{
	// Setup the GDT.
	m_gdt = g_InitialGDT;
	
	// Setup a descriptor.
	struct
	{
		uint16_t m_gdtLimit;
		uint64_t m_gdtBase;
	} PACKED gdtr;
	
	gdtr.m_gdtLimit = sizeof(Arch::GDT) - 1;
	gdtr.m_gdtBase  = uint64_t(&m_gdt);
	
	// Note: For now we do not need to reload segments such as CS and DS.
	// We will need to however, once we remove the 16- and 32-bit segments.
	__asm__("lgdt %0"::"m"(gdtr));
}

//  ***************************************************************
//  IDT.cpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements the IDT and its loader.
//
//  ***************************************************************
#include <Arch.hpp>

void __memset(void* pMemVoid, int c, size_t sz)
{
	uint8_t* bytes = (uint8_t*)pMemVoid;
	while (sz)
	{
		*(bytes++) = c;
		sz--;
	}
}

void Arch::CPU::ClearIDT()
{
	__memset(&m_idt, 0, sizeof m_idt);
}

void Arch::CPU::LoadIDT()
{
	struct
	{
		uint16_t m_idtLimit;
		uint64_t m_idtBase;
	}
	PACKED idtr;
	
	idtr.m_idtLimit = sizeof m_idt - 1;
	idtr.m_idtBase  = uint64_t(&m_idt);
	
	__asm__("lidt %0"::"m"(idtr));
}



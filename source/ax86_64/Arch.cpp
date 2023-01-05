//  ***************************************************************
//  ax86_64/Arch.cpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#include <Arch.hpp>

#ifdef TARGET_X86_64

void Arch::Halt()
{
	__asm__("hlt":::"memory");
}

void Arch::IdleLoop()
{
	for (;;)
		Halt();
}

uint8_t Arch::ReadByte(uint16_t port)
{
    uint8_t rv;
    __asm__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

void Arch::WriteByte(uint16_t port, uint8_t data)
{
	__asm__("outb %0, %1"::"a"((uint8_t)data),"Nd"((uint16_t)port));
}

void Arch::DisableInterrupts()
{
	__asm__("cli":::"memory");
}

void Arch::EnableInterrupts()
{
	__asm__("sti":::"memory");
}

void Arch::WriteMSR(uint32_t msr, uint64_t value)
{
	uint32_t edx = uint32_t(value >> 32);
	uint32_t eax = uint32_t(value);
	
	__asm__("wrmsr"::"d"(edx),"a"(eax),"c"(msr));
}

#endif//TARGET_X86_64

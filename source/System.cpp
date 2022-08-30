//  ***************************************************************
//  System.cpp - Creation date: 30/08/2022
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include "System.hpp"

void System::Stop()
{
	__asm__("cli");
	
	for (;;) {
		__asm__("hlt");
	}
}

void System::WritePort(uint16_t port, uint8_t thing)
{
	__asm__ volatile ("out %%al, %%dx"::"a"(thing),"d"(port));
}

uint8_t System::ReadPort(uint16_t port)
{
	uint8_t thing = 0;
	__asm__ volatile ("in %%dx, %%al":"=a"(thing):"d"(port));
	return thing;
}

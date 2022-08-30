//  ***************************************************************
//  main.cpp - Creation date: 30/08/2022
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include <stdint.h>
#include <stddef.h>

#include "Console.hpp"
#include "System.hpp"

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

volatile limine_terminal_request g_TerminalRequest =
{
	.id = LIMINE_TERMINAL_REQUEST,
	.revision = 0,
	.response = NULL,
	.callback = NULL,
};

volatile limine_bootloader_info_request g_BootloaderInfoRequest =
{
	.id = LIMINE_BOOTLOADER_INFO_REQUEST,
	.revision = 0,
	.response = NULL,
};

// The following will be our kernel's entry point.
extern "C" void _start(void)
{
	using namespace System;
	
	Console::InitializeFrom(&g_TerminalRequest);
	
	WritePort(0xe9, 'A');
	
	LogToConsole("Hello, world! %p", (void*)0xFFEEDDCCBBAA9988);

	WritePort(0xe9, 'X');
	
	// We're done, just hang...
	Stop();
}

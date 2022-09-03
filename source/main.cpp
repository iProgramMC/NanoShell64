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

class Something
{
private:
	int m_crap = 0;
	
public:
	Something()
	{
		m_crap = 1;
	}
	
	void SetCrap(int c)
	{
		m_crap = c;
	}
	
	int GetCrap()
	{
		return m_crap;
	}
};

Something g_something;

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
	RunAllConstructors();
	
	Console::InitializeFrom(&g_TerminalRequest);
	
	LogToConsole("Hello, world! %p", (void*)0xFFEEDDCCBBAA9988);
	LogToConsole("g_something.GetCrap() => %d", g_something.GetCrap());
	g_something.SetCrap(2);
	LogToConsole("g_something.GetCrap() => %d", g_something.GetCrap());
	
	// We're done, just hang...
	RunAllDestructors();
	Stop();
}

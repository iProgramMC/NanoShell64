//  ***************************************************************
//  Console.cpp - Creation date: 30/08/2022
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include "Console.hpp"
#include "System.hpp"

static volatile limine_terminal_request* s_pTermRequest;

void LogMsg(const char *fmt, ...)
{
	// allocate a buffer well sized
	char cr[8192];
	va_list list;
	va_start(list, fmt);
	vsprintf(cr, fmt, list);
	
	sprintf (cr + strlen(cr), "\n");
	Console::LogDebug(cr);
	
	va_end(list);
}
void LogToConsole(const char *fmt, ...)
{
	// allocate a buffer well sized
	char cr[8192];
	va_list list;
	va_start(list, fmt);
	vsprintf(cr, fmt, list);
	
	sprintf (cr + strlen(cr), "\n");
	Console::Log(cr);
	
	va_end(list);
}

void Console::InitializeFrom(volatile limine_terminal_request* pRequest)
{
	s_pTermRequest = pRequest;
	
	// check if there's actually a console here
	if (!s_pTermRequest->response)
	{
		LogMsg("No console available");
		System::Stop();
	}
	if (s_pTermRequest->response->terminal_count < 1)
	{
		LogMsg("No terminals specified");
		System::Stop();
	}
}

void Console::Log(const char* pText)
{
	// Log a piece of text to the terminal.
	size_t len = strlen (pText);
	
	struct limine_terminal *pTerminal = s_pTermRequest->response->terminals[0];
	
	s_pTermRequest->response->write(pTerminal, pText, len);
}

void Console::LogDebug(const char* pText)
{
	// Log a piece of text to the terminal.
	while (*pText)
	{
		System::WritePort(System::PORT_DEBUGCON, *pText);
		pText++;
	}
}

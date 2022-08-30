//  ***************************************************************
//  Console.hpp - Creation date: 30/08/2022
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#ifndef _CONSOLE_HPP
#define _CONSOLE_HPP

#include <_limine.h>
#include <_string.h>
#include <_print.h>

namespace Console
{
	// Logs a message to the debug console
	void LogDebug(const char *pText);
	
	// Logs a message to the user's screen
	void Log     (const char *pText);
	
	void InitializeFrom(volatile limine_terminal_request* pRequest);
};

void LogMsg(const char* pText, ...); // logs a debug string
void LogToConsole(const char *pText, ...); // logs a string to the console

#endif//_CONSOLE_HPP

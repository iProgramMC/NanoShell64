//  ***************************************************************
//  Terminal.hpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#ifndef _TERMINAL_HPP
#define _TERMINAL_HPP

#include <NanoShell.hpp>

namespace Terminal
{
	// Called when the kernel first starts up.
	void Setup();
	
	// Called when the kernel first starts up. Checks Limine's response field.
	bool CheckResponse();
	
	// Writes a string to the terminal.
	void Write(const char * str);
	
	// Writes a string to the terminal, and adds a new line.
	void WriteLn(const char * str);
	
	// Writes a string to the E9 port.
	void E9Write(const char * str);
	
	// Writes a string to the E9 port, and adds a new line.
	void E9WriteLn(const char * str);
};

#endif//_TERMINAL_HPP

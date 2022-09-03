//  ***************************************************************
//  Console.hpp - Creation date: 30/08/2022
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#ifndef _SYSTEM_HPP
#define _SYSTEM_HPP

#include <stdint.h>

namespace System
{
	enum ePort : uint16_t
	{
		PORT_DEBUGCON = 0xE9,
	};
	
	// Stop the system forcefully
	void Stop();
	
	// Writes a byte to the specified port
	void   WritePort(uint16_t port, uint8_t thing);
	
	// Reads a byte from the specified port
	uint8_t ReadPort(uint16_t port);
	
	// Run all global constructors
	void RunAllConstructors();
	
	// Run all global destructors
	void RunAllDestructors();
}

#endif//_SYSTEM_HPP


//  ***************************************************************
//  APIC.cpp - Creation date: 04/05/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements a small driver for the PIT, to allow
//    the calibration of the LAPIC timer.
//
//  ***************************************************************
#include <Spinlock.hpp>
#include <Arch.hpp>

constexpr size_t   C_PIT_PERIOD = 838;     // 1 second / 1193182 hz = ~838.09 nanoseconds.
constexpr uint16_t C_PIT_DATA_PORT = 0x40; // Channel 0.
constexpr uint16_t C_PIT_CMD_PORT  = 0x43;

using namespace Arch;

uint16_t PIT::Read()
{
	uint16_t data = ReadByte(C_PIT_DATA_PORT);
	
	data |= (ReadByte(C_PIT_DATA_PORT) << 8);
	
	return data;
}

void PIT::Sleep(uint64_t ns)
{
	uint64_t periods = ns / C_PIT_PERIOD;
	
	if (periods >= 0xFFF0)
	{
		// can't do!
		SLogMsg("Error: PIT::Sleep(%llu) is not possible", ns);
		return;
	}
	
	// calculate our target
	uint16_t target = 0xFFFF - uint16_t(periods);
	
	// reset the PIT counter to max:
	WriteByte(C_PIT_CMD_PORT,  0x34);
	WriteByte(C_PIT_DATA_PORT, 0xFF);
	WriteByte(C_PIT_DATA_PORT, 0xFF);
	
	// wait until the PIT hits our target
	while (Read() > target)
		Spinlock::SpinHint();
}

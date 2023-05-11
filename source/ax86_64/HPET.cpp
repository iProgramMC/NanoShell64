//  ***************************************************************
//  ax86_64/HPET.cpp - Creation date: 09/05/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2023 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements the HPET timer.
//
//  ***************************************************************

// Note: Huge thanks to https://github.com/DeanoBurrito/northport

#include <Arch.hpp>

using namespace Arch;

struct HPETAddress
{
	uint8_t  m_AddressSpaceID;
	uint8_t  m_RegisterBitWidth;
	uint8_t  m_RegisterBitOffset;
	uint8_t  m_Reserved;
	uint64_t m_Address;
}
PACKED;

struct HPETTable
{
	RSD::Table m_header;
	
	// HPET specific data.
	uint8_t m_HardwareRevID;
	
	uint8_t m_ComparatorCount   : 5;
	uint8_t m_CounterSize       : 1;
	uint8_t m_Reserved0         : 1;
	uint8_t m_LegacyReplacement : 1;
	
	uint16_t m_PCIVendorID;
	
	HPETAddress m_Address;
	
	uint8_t  m_HPETNumber;
	uint16_t m_MinimumTick;
	uint8_t  m_PageProtection;
}
PACKED;

union HPETGeneralCaps
{
	struct
	{
		unsigned m_RevID        : 8;
		unsigned m_NumTimCap    : 5;
		unsigned m_CountSizeCap : 1;
		unsigned m_Reserved     : 1;
		unsigned m_LegRouteCap  : 1;
		unsigned m_VendorID     : 16;
		unsigned m_CounterClockPeriod : 32;
	}
	PACKED;
	
	uint64_t m_Contents;
};

struct HPETTimerInfo
{
	uint64_t m_ConfigAndCaps;
	uint64_t m_ComparatorValue;
	uint64_t m_FSBInterruptRoute;
	uint64_t m_reserved;
};

struct HPETRegisters
{
	uint64_t m_GeneralCapsRegister;
	uint64_t m_reserved0;
	uint64_t m_GeneralConfig;
	uint64_t m_reserved1;
	uint64_t m_GeneralIrqStatus;
	uint64_t m_reserved2;
	uint64_t m_CounterValue;
	uint64_t m_reserved3;
	HPETTimerInfo m_timers[32];
};

HPETTable g_HpetTable;
volatile HPETRegisters* g_pHpetRegisters;
HPETGeneralCaps g_HpetGeneralCaps;

void HPET::Found(RSD::Table* pTable)
{
	memcpy(&g_HpetTable, pTable, sizeof g_HpetTable);
	
	using namespace VMM;
	
	// map it in.
	PageMapping* pPM = PageMapping::GetFromCR3();
	
	pPM->MapPage(C_HPET_MAP_ADDRESS, PageEntry(g_HpetTable.m_Address.m_Address, PE_PRESENT | PE_READWRITE | PE_SUPERVISOR | PE_EXECUTEDISABLE | PE_CACHEDISABLE));
	
	g_pHpetRegisters = (HPETRegisters*)C_HPET_MAP_ADDRESS;
	
	g_HpetGeneralCaps.m_Contents = g_pHpetRegisters->m_GeneralCapsRegister;
	
	// Read and dump the general caps and ID registers.
	LogMsg("HPET Capabilities: %Q  vendor ID: %W", g_HpetGeneralCaps.m_Contents, g_HpetGeneralCaps.m_VendorID);
}


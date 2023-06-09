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
	uint64_t m_reserved8;
	uint64_t m_GeneralConfig;
	uint64_t m_reserved18;
	uint64_t m_GeneralIrqStatus;
	uint64_t m_reserved28;
	uint64_t m_reservedArr[24];
	uint64_t m_CounterValue;
	uint64_t m_reservedF8;
	HPETTimerInfo m_timers[32];
};

constexpr uint64_t C_HPET_GEN_CFG_ENABLE_CNF = BIT(0); // Overall Enable. This bit MUST be set to allow any of the timers to generate interrupts and increment the main counter.
constexpr uint64_t C_HPET_GEN_CFG_LEG_RT_CNF = BIT(1); // supports legacy replacement route

HPETTable g_HpetTable;
volatile HPETRegisters* g_pHpetRegisters;
HPETGeneralCaps g_HpetGeneralCaps;

// A nanosecond is 10^6, or 1'000'000, femtoseconds
constexpr uint64_t C_FEMTOS_TO_NANOS = 1'000'000;
constexpr uint64_t C_HPET_MAX_PERIOD = 100'000'000; // Note: The spec mandates 05F5E100h (which looks like a random magic value but it's just this)
constexpr uint64_t C_HPET_MIN_PERIOD = 100'000;     // 1/10 of a nanosecond. Really doubt any kind of timer runs that fast

uint64_t HPET::GetRawTickCount()
{
	// note: this contacts the system bus, so it's not as fast as the TSC.
	return g_pHpetRegisters->m_CounterValue;
}

uint64_t HPET::GetCounterClockPeriod()
{
	return g_HpetGeneralCaps.m_CounterClockPeriod;
}

uint64_t HPET::GetTickCount()
{
	// note: constant divide should be optimized to a fixed point multiply combo
	return uint64_t(g_pHpetRegisters->m_CounterValue) * g_HpetGeneralCaps.m_CounterClockPeriod / C_FEMTOS_TO_NANOS;
}

void HPET::PolledSleep(uint64_t nanoseconds)
{
	uint64_t time = nanoseconds * C_FEMTOS_TO_NANOS / g_HpetGeneralCaps.m_CounterClockPeriod;
	
	uint64_t current_time = GetRawTickCount();
	uint64_t target = current_time + time;
	
	while (GetRawTickCount() < target)
		Spinlock::SpinHint();
}

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
	LogMsg("Counter clock period: %d femtoseconds per tick (%d nanoseconds)", g_HpetGeneralCaps.m_CounterClockPeriod, g_HpetGeneralCaps.m_CounterClockPeriod / C_FEMTOS_TO_NANOS);
	
	if (g_HpetGeneralCaps.m_CounterClockPeriod > C_HPET_MAX_PERIOD)
	{
		LogMsg("WARNING: HPET counter clock period is %d, bigger than 100 nanoseconds. The spec doesn't allow that!", g_HpetGeneralCaps.m_CounterClockPeriod);
	}
	if (g_HpetGeneralCaps.m_CounterClockPeriod < C_HPET_MIN_PERIOD)
	{
		LogMsg("WARNING: HPET counter clock period is %d, smaller than 1/10 nanoseconds. You may be seeing issues caused by timer overflow.", g_HpetGeneralCaps.m_CounterClockPeriod);
	}
	
	// eh, but we will use the HPET to calibrate the LAPIC timer anyways
	if (g_HpetGeneralCaps.m_CountSizeCap != 1)
	{
		LogMsg("WARNING: HPET cannot operate in 64-bit mode. We cannot handle overflow well...");
	}
	
	// Enable and reset the main counter.
	g_pHpetRegisters->m_GeneralConfig = 0;
	g_pHpetRegisters->m_CounterValue  = 0;
	g_pHpetRegisters->m_GeneralConfig = C_HPET_GEN_CFG_ENABLE_CNF;
	
	LogMsg("Counter Config: %lld", g_pHpetRegisters->m_GeneralConfig);
	
	uint64_t lastValue = 0;
	
	for (int i = 0; i < 20; i++)
	{
		uint64_t value = g_pHpetRegisters->m_CounterValue;
		
		if (value <= lastValue)
		{
			LogMsg("FATAL ERROR: value %llu smaller than lastValue %llu. Cannot continue.", value, lastValue);
			return;
		}
		
		PIT::PolledSleep(1*1000*1000);
	}
	
	APIC::SetPolledSleepFunc(HPET::PolledSleep);
}


//  ***************************************************************
//  ax86_64/RSD.cpp - Creation date: 08/05/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2023 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements the code that allows for the usage
//    of the RSDP table.
//
//  ***************************************************************
#include <Arch.hpp>

using namespace Arch;

KList<RSD::Table*> g_RSDTables;

volatile limine_rsdp_request g_RSDPRequest =
{
	.id = LIMINE_RSDP_REQUEST,
	.revision = 0,
	.response = NULL,
};

void RSD::LoadTable(RSD::Table* pTable)
{
	int count = pTable->GetSubSDTCount();
	
	// add it to the list of RSD tables:
	
	for (int i = 0; i < count; i++)
	{
		uintptr_t addr = pTable->m_SubSDTs[i];
		
		Table* pItem = (Table*)(Arch::GetHHDMOffset() + addr);
		g_RSDTables.AddBack(pItem);
	}
}

void RSD::Load()
{
	Descriptor* pDesc = (Descriptor*)g_RSDPRequest.response->address;
	
	uintptr_t rsdtAddr = pDesc->GetRSDTAddress();
	
	Table* pTable = (Table*)(Arch::GetHHDMOffset() + rsdtAddr);
	
	LoadTable(pTable);
	
	for (auto it = g_RSDTables.Begin(); it.Valid(); it++)
	{
		char thing[5];
		thing[4] = 0;
		
		Table* pTable = *it;
		memcpy(thing, pTable->m_Signature, 4);
		
		LogMsg("RSD entry: %s", thing);
		
		if (strcmp(thing, "HPET") == 0)
			HPET::Found(pTable);
	}
}



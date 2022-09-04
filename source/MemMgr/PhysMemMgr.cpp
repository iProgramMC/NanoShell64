//  ***************************************************************
//  PhysMemMgr.cpp - Creation date: 03/09/2022
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include <stdint.h>
#include <stddef.h>
#include "System.hpp"
#include "Console.hpp"
#include "PhysMemMgr.hpp"
#include "VirtMemMgr.hpp"

//TODO: spinlock
static uint8_t* s_pBitmap = NULL;
static uint64_t s_highestPageIndex = 0;
static uint64_t s_lastUsedIndex = 0;
static uint64_t s_usablePages = 0;
static uint64_t s_usedPages = 0;
static uint64_t s_reservedPages = 0;

volatile limine_memmap_request g_memmap_request =
{
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
	.response = NULL,
};

void PhysMemMgr::Init()
{
	auto pMemMap = g_memmap_request.response;
	auto pHHDM   = g_hhdm_request.response;
	
	if (!pMemMap)
		System::Panic("No mem map was specified!");
	
	uintptr_t highestAddr = 0;
	
	// calculate how big the memmap needs to be
	for (size_t i = 0; i < pMemMap->entry_count; i++)
	{
		auto pEntry = pMemMap->entries[i];
		
		LogMsg("Memory map entry: base: %p length: %p type: %p", pEntry->base, pEntry->length, pEntry->type);
		
		switch (pEntry->type)
		{
			//...
		}
	}
}



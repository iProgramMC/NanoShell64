//  ***************************************************************
//  PMM.cpp - Creation date: 07/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements the NanoShell virtual memory
//  manager.
//
//  ***************************************************************
#include <Arch.hpp>

// Get the next level for each of the levels.
namespace VMM
{

PageTable* PageDirectory::GetPageTable(int index)
{
	if (!m_entries[index].m_present) return NULL;
	return (PageTable*)(Arch::GetHHDMOffset() + PAGE_SIZE * (m_entries[index].m_address));
}

PageDirectory* PML3::GetPageDirectory(int index)
{
	if (!m_entries[index].m_present) return NULL;
	return (PageDirectory*)(Arch::GetHHDMOffset() + PAGE_SIZE * (m_entries[index].m_address));
}

PML3* PML4::GetPML3(int index)
{
	if (!m_entries[index].m_present) return NULL;
	return (PML3*)(Arch::GetHHDMOffset() + PAGE_SIZE * (m_entries[index].m_address));
}

PML4* PageMapping::GetPML4(int index)
{
	if (!m_entries[index].m_present) return NULL;
	return (PML4*)(Arch::GetHHDMOffset() + PAGE_SIZE * (m_entries[index].m_address));
}

PageMapping* PageMapping::GetFromCR3()
{
	return (PageMapping*)(Arch::GetHHDMOffset() + Arch::ReadCR3());
}

void PageMapping::SwitchTo()
{
	if (this < (PageMapping*)Arch::GetHHDMOffset())
	{
		LogMsg("This page mapping ain't part of the hhdm");
		return;
	}
	
	// Go!!
	Arch::WriteCR3((uintptr_t)this - Arch::GetHHDMOffset());
}

PageMapping* PageMapping::Clone(PageMapping* pPM)
{
	uintptr_t pmPage = PMM::AllocatePage();
	
	if (pmPage == PMM::INVALID_PAGE)
	{
		LogMsg("Could not clone page mapping! (souce/MemMgr/VMM.cpp:%d)", __LINE__);
		return NULL;
	}
	
	// Allocate the pagemapping itself.
	PageMapping* pNewPM = (PageMapping*)(Arch::GetHHDMOffset() + pmPage);
	memset(pNewPM, 0, sizeof *pNewPM);
	
	// Look through the lower canonical half's PML4 entries and clone them recursively.
	for (int i = 0; i < 256; i++)
	{
		// If there is no PML4, continue;
		PML4* pOldPML4 = pPM->GetPML4(i);
		if (!pOldPML4) continue;
		
		PageEntry& oldEnt = pPM->m_entries[i];
		
		PML4* pPML4 = ClonePML4(pPM->GetPML4(i));
		
		// Build a page entry.
		PageEntry entry = oldEnt;
		
		// If this is part of the PMM, just copy. TODO Copy on write
		if (entry.m_partOfPmm)
		{
			uintptr_t newPMMPage = PMM::AllocatePage();
			if (newPMMPage == PMM::INVALID_PAGE)
			{
				LogMsg("Could not clone page mapping (PMM page could not be copied) (souce/MemMgr/VMM.cpp:%d)", __LINE__);
				// revert everything we already have
				// TODO: FreePageMapping(pNewPM);
				return NULL;
			}
		}
		
		pNewPM->m_entries[i].m_data = entry.m_data;
	}
	
	// Just copy the other 256.
	for (int i = 0; i < 512; i++)
	{
		PageEntry& oldEnt = pPM->m_entries[i];
		pNewPM->m_entries[i].m_data = oldEnt.m_data;
	}
	
	return pNewPM;
}

}; // namespace VMM

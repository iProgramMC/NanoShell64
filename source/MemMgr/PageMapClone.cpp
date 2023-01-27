//  ***************************************************************
//  PageMapClone.cpp - Creation date: 07/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements the page mapping's clone functions.
//
//  ***************************************************************
#include <Arch.hpp>

namespace VMM
{

/**** Cloning ****/

PageTable* PageTable::Clone()
{
	uintptr_t pmPage = PMM::AllocatePage();
	
	if (pmPage == PMM::INVALID_PAGE)
		KernelPanic("Could not clone page table! (source/MemMgr/VMM.cpp:%d)", __LINE__);
	
	// Allocate the PageTable itself.
	PageTable* pNewPT = (PageTable*)(Arch::GetHHDMOffset() + pmPage);
	memset(pNewPT, 0, sizeof *pNewPT);
	
	for (int i = 0; i < 512; i++)
	{
		// Clone the page.
		PageEntry& oldEnt = m_entries[i];
		if (!oldEnt.m_present) continue;
		
		PageEntry newEnt = oldEnt;
		
		// if this is part of the PMM...
		if (oldEnt.m_partOfPmm)
		{
			// clone the page. TODO: Copy on write
			uintptr_t newPage = PMM::AllocatePage();
			if (pmPage == PMM::INVALID_PAGE)
				KernelPanic("Could not clone page! (source/MemMgr/VMM.cpp:%d)", __LINE__);
			
			newEnt.m_address = newPage >> 12;
		}
		
		pNewPT->m_entries[i].m_data = newEnt.m_data;
	}
	
	return pNewPT;
}

PageDirectory* PageDirectory::Clone()
{
	uintptr_t pmPage = PMM::AllocatePage();
	
	if (pmPage == PMM::INVALID_PAGE)
		KernelPanic("Could not clone page directory! (source/MemMgr/VMM.cpp:%d)", __LINE__);
	
	// Allocate the PageDirectory itself.
	PageDirectory* pNewPD = (PageDirectory*)(Arch::GetHHDMOffset() + pmPage);
	memset(pNewPD, 0, sizeof *pNewPD);
	
	for (int i = 0; i < 512; i++)
	{
		PageTable* pOldPT = GetPageTable(i);
		if (!pOldPT) continue;
		
		PageEntry& oldEnt = m_entries[i];
		
		// Clone the page table.
		PageTable* pPT = pOldPT->Clone();
		if (!pPT) continue;
		uintptr_t ptPhys = (uintptr_t)pPT - Arch::GetHHDMOffset();
		
		// Build a page entry.
		PageEntry entry = oldEnt;
		entry.m_address = ptPhys >> 12;
		
		pNewPD->m_entries[i].m_data = entry.m_data;
	}
	
	return pNewPD;
}

PML3* PML3::Clone()
{
	uintptr_t pmPage = PMM::AllocatePage();
	
	if (pmPage == PMM::INVALID_PAGE)
		KernelPanic("Could not clone PML3! (source/MemMgr/VMM.cpp:%d)", __LINE__);
	
	// Allocate the PML3 itself.
	PML3* pNewPM = (PML3*)(Arch::GetHHDMOffset() + pmPage);
	memset(pNewPM, 0, sizeof *pNewPM);
	
	for (int i = 0; i < 512; i++)
	{
		PageDirectory* pOldPD = GetPageDirectory(i);
		if (!pOldPD) continue;
		
		PageEntry& oldEnt = m_entries[i];
		
		// Clone the page directory.
		PageDirectory* pPD = pOldPD->Clone();
		if (!pPD) continue;
		uintptr_t pdPhys = (uintptr_t)pPD - Arch::GetHHDMOffset();
		
		// Build a page entry.
		PageEntry entry = oldEnt;
		entry.m_address = pdPhys >> 12;
		
		pNewPM->m_entries[i].m_data = entry.m_data;
	}
	
	return pNewPM;
}

PageMapping* PageMapping::Clone(bool keepLowerHalf)
{
	uintptr_t pmPage = PMM::AllocatePage();
	
	if (pmPage == PMM::INVALID_PAGE)
		KernelPanic("Could not clone page mapping! (source/MemMgr/VMM.cpp:%d)", __LINE__);
	
	// Allocate the pagemapping itself.
	PageMapping* pNewPM = (PageMapping*)(Arch::GetHHDMOffset() + pmPage);
	memset(pNewPM, 0, sizeof *pNewPM);
	
	// Look through the lower canonical half's PML3 entries and clone them recursively.
	for (int i = P_USER_START; keepLowerHalf && i < P_USER_END; i++)
	{
		// If there is no PML3, continue;
		PML3* pOldPML3 = GetPML3(i);
		if (!pOldPML3) continue;
		
		PageEntry& oldEnt = m_entries[i];
		
		PML3* pPML3 = pOldPML3->Clone();
		if (!pPML3) continue;
		uintptr_t pml4Phys = (uintptr_t)pPML3 - Arch::GetHHDMOffset();
		
		// Build a page entry.
		PageEntry entry = oldEnt;
		entry.m_address = pml4Phys >> 12;
		
		pNewPM->m_entries[i].m_data = entry.m_data;
	}
	
	// Just copy the other 256.
	for (int i = P_KERN_START; i < P_KERN_END; i++)
	{
		PageEntry& oldEnt = m_entries[i];
		pNewPM->m_entries[i].m_data = oldEnt.m_data;
	}
	
	using namespace Arch;
	
	CPU* pCpu = CPU::GetCurrent();
	if (!pCpu->WerePml4EntriesInitted())
	{
		// Ideally this is called before we get to any tasking code
		pCpu->Pml4EntriesInitted();
		
		// Initialize the PML4 entries of the user heap.
		for (int i = P_KHEAP_START; i < P_KHEAP_END; i++)
		{
			uintptr_t addr = PMM::AllocatePage();
			
			if (addr == PMM::INVALID_PAGE)
				KernelPanic("Could not allocate kernel heap page.");
			
			PageEntry ent(addr, true, false, true, true, false);
			pNewPM->m_entries[i] = ent;
		}
	}
	
	return pNewPM;
}

} // namespace VMM
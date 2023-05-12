//  ***************************************************************
//  VMM.cpp - Creation date: 07/01/2023
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

/**** Get the next level ****/

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

PML3* PageMapping::GetPML3(int index)
{
	if (!m_entries[index].m_present) return NULL;
	return (PML3*)(Arch::GetHHDMOffset() + PAGE_SIZE * (m_entries[index].m_address));
}

PageMapping* PageMapping::GetFromCR3()
{
	return (PageMapping*)(Arch::GetHHDMOffset() + Arch::ReadCR3());
}

/**** Unmap pages ****/
void PageMapping::UnmapPage(uintptr_t addr, bool removeUpperLevels)
{
	// Remove a page mapping.
	constexpr uintptr_t mask = 0x1FF;
	uintptr_t index_PML4 = (addr >> 39) & mask;
	uintptr_t index_PML3 = (addr >> 30) & mask;
	uintptr_t index_PML2 = (addr >> 21) & mask;
	uintptr_t index_PML1 = (addr >> 12) & mask;
	
	PML3          *pml3 =       GetPML3         (index_PML4); if (!pml3) return;
	PageDirectory *pml2 = pml3->GetPageDirectory(index_PML3); if (!pml2) return;
	PageTable     *pml1 = pml2->GetPageTable    (index_PML2); if (!pml1) return;
	
	PageEntry& ent = pml1->m_entries[index_PML1];
	
	// If this was a part of the PMM, free the page.
	if (ent.m_partOfPmm)
	{
		PMM::FreePage(ent.m_address << 12);
	}
	
	memset(&ent, 0, sizeof ent);
	
	if (removeUpperLevels)
	{
		// TODO: Remove upper levels too if they're empty.
	}
}

/**** Map pages ****/
bool PageMapping::MapPage(uintptr_t addr, const PageEntry & pe)
{
	constexpr uintptr_t mask = 0x1FF;
	uintptr_t index_PML4 = (addr >> 39) & mask;
	uintptr_t index_PML3 = (addr >> 30) & mask;
	uintptr_t index_PML2 = (addr >> 21) & mask;
	uintptr_t index_PML1 = (addr >> 12) & mask;
	
	// if we don't have a pml3 here:
	PML3* pml3 = GetPML3(index_PML4);
	if (!pml3)
	{
		// allocate one
		uintptr_t page = PMM::AllocatePage();
		if (page == PMM::INVALID_PAGE) return false;
		pml3 = (PML3*)(Arch::GetHHDMOffset() + page);
		memset(pml3, 0, sizeof *pml3);
		m_entries[index_PML4] = PageEntry(page, PE_PRESENT | PE_READWRITE | PE_PARTOFPMM);
	}
	
	// if we don't have a pml2 here:
	PageDirectory* pd = pml3->GetPageDirectory(index_PML3);
	if (!pd)
	{
		// allocate one
		uintptr_t page = PMM::AllocatePage();
		if (page == PMM::INVALID_PAGE) return false;
		pd = (PageDirectory*)(Arch::GetHHDMOffset() + page);
		memset(pd, 0, sizeof *pd);
		pml3->m_entries[index_PML3] = PageEntry(page, PE_PRESENT | PE_READWRITE | PE_PARTOFPMM);
	}
	
	// if we don't have a pml1 here:
	PageTable* pt = pd->GetPageTable(index_PML2);
	if (!pt)
	{
		// allocate one
		uintptr_t page = PMM::AllocatePage();
		if (page == PMM::INVALID_PAGE) return false;
		pt = (PageTable*)(Arch::GetHHDMOffset() + page);
		memset(pt, 0, sizeof *pt);
		pd->m_entries[index_PML2] = PageEntry(page, PE_PRESENT | PE_READWRITE | PE_PARTOFPMM);
	}
	
	// unmap whatever was here previously.
	UnmapPage(addr, false);
	
	// map the page in.
	PageEntry& entry = pt->m_entries[index_PML1];
	
	entry = pe;
	
	return true;
}

bool PageMapping::MapPage(uintptr_t addr, bool rw, bool super, bool xd)
{
	/*
	uintptr_t pm = PMM::AllocatePage();
	if (pm == PMM::INVALID_PAGE) return false;
	
	// Create a page entry. TODO don't actually allocate from the PMM but instead fault the page in.
	PageEntry pe(pm, rw, super, xd, true, false);
	*/
	
	uint64_t flags = PE_PARTOFPMM | PE_NEEDALLOCPAGE;
	if (rw)
		flags |= PE_READWRITE;
	if (super)
		flags |= PE_SUPERVISOR;
	if (xd)
		flags |= PE_EXECUTEDISABLE;
	
	// Create a page entry. Note that the default_flags are overridden here.
	PageEntry pe(0, flags, 0);
	
	return MapPage(addr, pe);
}

PageEntry* PageMapping::GetPageEntry(uintptr_t addr)
{
	// Remove a page mapping.
	constexpr uintptr_t mask = 0x1FF;
	uintptr_t index_PML4 = (addr >> 39) & mask;
	uintptr_t index_PML3 = (addr >> 30) & mask;
	uintptr_t index_PML2 = (addr >> 21) & mask;
	uintptr_t index_PML1 = (addr >> 12) & mask;
	
	PML3          *pPml3;
	PageDirectory *pPageDir; 
	PageTable     *pPageTable; 
	
	if (!(pPml3      = GetPML3                   (index_PML4))) return NULL;
	if (!(pPageDir   = pPml3   ->GetPageDirectory(index_PML3))) return NULL;
	if (!(pPageTable = pPageDir->GetPageTable    (index_PML2))) return NULL;
	return pPageTable->GetPageEntry(index_PML1);
}

/**** Switch To ****/

void PageMapping::SwitchTo()
{
	if (this < (PageMapping*)Arch::GetHHDMOffset())
	{
		SLogMsg("This page mapping ain't part of the hhdm");
		return;
	}
	
	// Go!!
	Arch::WriteCR3((uintptr_t)this - Arch::GetHHDMOffset());
}

}; // namespace VMM

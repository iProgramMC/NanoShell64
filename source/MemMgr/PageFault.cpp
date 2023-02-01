//  ***************************************************************
//  PageFault.cpp - Creation date: 01/02/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements the page fault function.
//
//  ***************************************************************
#include <Arch.hpp>
#include <MemoryManager.hpp>

void Arch::CPU::OnPageFault(Registers* pRegs)
{
	LogMsg("Page fault!!  CR2: %p  RIP: %p  ErrorCode: %p", pRegs->cr2, pRegs->rip, pRegs->error_code);
	
	union
	{
		struct
		{
			bool bPresent : 1;
			bool bWrite   : 1;
			bool bUser    : 1;
			//......
		};
		uint64_t value;
	}
	errorCode;
	
	errorCode.value = pRegs->error_code;
	
	using namespace VMM;
	
	// Check if the accessed page is valid or not
	PageMapping* pPM = PageMapping::GetFromCR3();
	
	PageEntry* pPageEntry = pPM->GetPageEntry(pRegs->cr2);
	
	if (!pPageEntry)
	{
		// Since the page entry isn't even there, I'm going to have to make you bail
		goto _INVALID_PAGE_FAULT;
	}
	
	if (!errorCode.bPresent)
	{
		// The thing isn't present. Maybe it is in the page entry?
		// Not sure why it would, though. Page mappings should only
		// be messed with by their owner CPU.
		if (pPageEntry->m_present)
			return;
		
		// still not present.  Look into why that is.
		if (pPageEntry->m_needAllocPage)
		{
			// ah hah! I see why that is now - we need to allocate a PMM page.
			uintptr_t page = PMM::AllocatePage();
			if (page == PMM::INVALID_PAGE)
			{
				// Uh oh. We need to get rid of some pages, TODO
				KernelPanic("TODO: out of memory, need to free some caches and stuff. CR2: %p  RIP: %p  ErrorCode: %p", pRegs->cr2, pRegs->rip, pRegs->error_code);
			}
			
			// fill it with some byte
			uint8_t someByte = (pPageEntry->m_protKey << 4 | pPageEntry->m_protKey);
			memset((void*)(Arch::GetHHDMOffset() + page), someByte, PAGE_SIZE);
			
			// make it present now
			pPageEntry->m_address = page >> 12;
			pPageEntry->m_needAllocPage = false;
			pPageEntry->m_partOfPmm     = true;
			pPageEntry->m_present       = true;
			
			Arch::Invalidate(pRegs->cr2);
			return;
		}
		
		// not sure of any other cases, so...
		goto _INVALID_PAGE_FAULT;
	}
	
	// If the page was present but we have an access error...
	if (errorCode.bWrite)
	{
		// TODO COW
		goto _INVALID_PAGE_FAULT;
	}
	
	// don't know of any other cases.
	
_INVALID_PAGE_FAULT:
	KernelPanic("Invalid page fault at CR2: %p RIP: %p  ErrorCode: %p", pRegs->cr2, pRegs->rip, pRegs->error_code);
}

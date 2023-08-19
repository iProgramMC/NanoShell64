#include <mm.h>
#include <hal.h>
#include <ke.h>
#include <_limine.h>

extern volatile struct limine_hhdm_request   g_HHDMRequest;
extern volatile struct limine_memmap_request g_MemMapRequest;

typedef struct {
	uint8_t data[PAGE_SIZE];
} OnePage;

struct PmmSentinelTAG;
typedef struct PmmSentinelTAG
{
	struct PmmSentinelTAG * m_pNext;
	struct PmmSentinelTAG * m_pPrev;
	size_t m_nPages;
}
PmmSentinel;

uint8_t* MmGetHHDMBase()
{
	return (uint8_t*)g_HHDMRequest.response->offset;
}

void* MmGetHHDMOffsetAddr(uintptr_t physAddr)
{
	return (void*) (MmGetHHDMBase() + physAddr);
}

PmmSentinel *g_pFirstPMMSentinel, *g_pLastPMMSentinel;

// Allocates a page from the memmap for eternity during init.  Used to prepare the PFN database.
static uintptr_t MiAllocatePageFromMemMap()
{
	struct limine_memmap_response* pResponse = g_MemMapRequest.response;
	
	for (uint64_t i = 0; i < pResponse->entry_count; i++)
	{
		// if the entry isn't usable, skip it
		struct limine_memmap_entry* pEntry = pResponse->entries[i];
		
		if (pEntry->type != LIMINE_MEMMAP_USABLE)
			continue;
		
		// Note! Usable entries in limine are guaranteed to be aligned to
		// page size, and not overlap any other entries. So we are good
		
		// if it's got no pages, also skip it..
		if (pEntry->length == 0)
			continue;
		
		uintptr_t curr_addr = pEntry->base;
		pEntry->base   += PAGE_SIZE;
		pEntry->length -= PAGE_SIZE;
		
		return curr_addr;
	}
	
	LogMsg("Error, out of memory in the memmap allocate function");
	return 0;
}

static bool MiMapNewPageAtAddressIfNeeded(uintptr_t pageTable, uintptr_t address)
{
	// Maps a new page at an address, if needed.
	PageMapLevel *pPML[4];
	pPML[3] = (PageMapLevel*) MmGetHHDMOffsetAddr(pageTable);
	
	for (int i = 3; i >= 0; i--)
	{
		int index = (address >> (12 + 9 * i)) & 0x1FF;
		if (pPML[i]->entries[index] & MM_PTE_PRESENT)
		{
			if (i == 0)
				return true; // didn't allocate anything
			
			pPML[i - 1] = (PageMapLevel*) MmGetHHDMOffsetAddr(PTE_ADDRESS(pPML[i]->entries[index]));
		}
		else
		{
			uintptr_t addr = MiAllocatePageFromMemMap();
			
			if (!addr)
			{
				// TODO: Allow rollback
				return false;
			}
			
			if (i != 0)
				pPML[i - 1] = (PageMapLevel*) MmGetHHDMOffsetAddr(addr);
			
			pPML[i]->entries[index] = addr | MM_PTE_PRESENT | MM_PTE_READWRITE | MM_PTE_SUPERVISOR | MM_PTE_GLOBAL | MM_PTE_NOEXEC;
		}
	}
	
	return true;
}

int MmPhysPageToPFN(uintptr_t physAddr)
{
	return physAddr / PAGE_SIZE;
}

uintptr_t MmPFNToPhysPage(int pfn)
{
	return (uintptr_t) pfn * PAGE_SIZE;
}

PageFrame* MmGetPageFrameFromPFN(int pfn)
{
	PageFrame* pPFNDB = (PageFrame*) MM_PFNDB_BASE;
	
	return &pPFNDB[pfn];
}

// for the page frame allocator
int g_firstPFN = -1, g_lastPFN = -1;

// Note! Initialization is done on the BSP. So no locking needed
void MiInitPMM()
{
	if (!g_MemMapRequest.response || !g_HHDMRequest.response)
	{
		LogMsg("Error, no memory map was provided");
		KeStopCurrentCPU();
	}
	
	// with 4-level paging, limine seems to be hardcoded at this address, so we're probably good. Although
	// the protocol does NOT specify that, and it does seem to be affected by KASLR...
	if (g_HHDMRequest.response->offset != 0xFFFF800000000000)
	{
		LogMsg("The HHDM isn't at 0xFFFF 8000 0000 0000, things may go wrong! (It's actually at %p)", (void*) g_HHDMRequest.response->offset);
	}
	
	uintptr_t currPageTablePhys = HalGetCurrentPageTable();
	
	// allocate the entries in the page frame number database
	struct limine_memmap_response* pResponse = g_MemMapRequest.response;
	
	// pass 0: print out all the entries for debugging
	for (uint64_t i = 0; i < pResponse->entry_count; i++)
	{
		// if the entry isn't usable, skip it
		struct limine_memmap_entry* pEntry = pResponse->entries[i];
		
		if (pEntry->type != LIMINE_MEMMAP_USABLE)
			continue;
		
		SLogMsg("%p-%p (%d pages)", pEntry->base, pEntry->base + pEntry->length, pEntry->length / PAGE_SIZE);
	}
	
	// pass 1: mapping the pages themselves
	for (uint64_t i = 0; i < pResponse->entry_count; i++)
	{
		// if the entry isn't usable, skip it
		struct limine_memmap_entry *pEntry = pResponse->entries[i];
		
		if (pEntry->type != LIMINE_MEMMAP_USABLE)
			continue;
		
		// make a copy since we might tamper with this
		struct limine_memmap_entry entry = *pEntry;
		
		int pfnStart = MmPhysPageToPFN(entry.base);
		int pfnEnd   = MmPhysPageToPFN(entry.base + entry.length);
		
		uint64_t lastAllocatedPage = 0;
		for (int pfn = pfnStart; pfn < pfnEnd; pfn++)
		{
			uint64_t currPage = (MM_PFNDB_BASE + sizeof(PageFrame) * pfn) & ~(PAGE_SIZE - 1);
			if (lastAllocatedPage == currPage) // check is probably useless
				continue;
			
			if (!MiMapNewPageAtAddressIfNeeded(currPageTablePhys, currPage))
				LogMsg("Error, couldn't actually setup PFN database");
			
			lastAllocatedPage = currPage;
		}
	}
	
	SLogMsg("Initializing the PFN database");
	// pass 2: Initting the PFN database
	int lastPfnOfPrevBlock = -1;
	
	for (uint64_t i = 0; i < pResponse->entry_count; i++)
	{
		// if the entry isn't usable, skip it
		struct limine_memmap_entry* pEntry = pResponse->entries[i];
		
		if (pEntry->type != LIMINE_MEMMAP_USABLE)
			continue;
		
		for (uint64_t j = 0; j < pEntry->length; j += PAGE_SIZE)
		{
			int currPFN = MmPhysPageToPFN(pEntry->base + j);
			
			if (g_firstPFN == -1)
				g_firstPFN =  currPFN;
			
			PageFrame* pPF = MmGetPageFrameFromPFN(currPFN);
			
			// initialize this PFN
			pPF->m_flags = 0;
			pPF->m_refCount = 0;
			
			if (j == 0)
			{
				pPF->m_prevFrame = lastPfnOfPrevBlock;
				
				// also update the last PF's next frame idx
				if (lastPfnOfPrevBlock != -1)
				{
					PageFrame* pPrevPF = MmGetPageFrameFromPFN(lastPfnOfPrevBlock);
					pPrevPF->m_nextFrame = currPFN;
				}
			}
			else
			{
				pPF->m_prevFrame = currPFN - 1;
			}
			
			if (j + PAGE_SIZE >= pEntry->length)
			{
				pPF->m_nextFrame = -1; // it's going to be updated by the next block if there's one
			}
			else
			{
				pPF->m_nextFrame = currPFN + 1;
			}
			
			lastPfnOfPrevBlock = currPFN;
		}
	}
	
	g_lastPFN = lastPfnOfPrevBlock;
	
	SLogMsg("PFN database initialized.");
	LogMsg("PFN database initialized.");
}


//  ***************************************************************
//  ax86_64/Arch.cpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
//  
//  Module description:
//      This module implements the x86_64 architecture specific
//    functions that don't work on a particular CPU object.
//    (those reside in CPU.cpp)
//
//  ***************************************************************
#include <Arch.hpp>
#include <Spinlock.hpp>
#include <EternalHeap.hpp>
#include <Terminal.hpp>

#ifdef TARGET_X86_64

void Arch::Halt()
{
	ASM("hlt":::"memory");
}

void Arch::IdleLoop()
{
	for (;;)
		Halt();
}

uint8_t Arch::ReadByte(uint16_t port)
{
    uint8_t rv;
    ASM("inb %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

void Arch::WriteByte(uint16_t port, uint8_t data)
{
	ASM("outb %0, %1"::"a"((uint8_t)data),"Nd"((uint16_t)port));
}

void Arch::DisableInterrupts()
{
	ASM("cli":::"memory");
}

void Arch::EnableInterrupts()
{
	ASM("sti":::"memory");
}

void Arch::WriteMSR(uint32_t msr, uint64_t value)
{
	uint32_t edx = uint32_t(value >> 32);
	uint32_t eax = uint32_t(value);
	
	ASM("wrmsr"::"d"(edx),"a"(eax),"c"(msr));
}

uint64_t Arch::ReadMSR(uint32_t msr)
{
	uint32_t edx, eax;
	
	ASM("rdmsr":"=d"(edx),"=a"(eax):"c"(msr));
	
	return uint64_t(edx) << 32 | eax;
}

Atomic<int> g_CPUsInitialized;

// Since the pointer to the structure is passed into RDI, assuming
// the x86_64 System V ABI, the first argument corresponds to RDI.
void Arch::CPU::Start(limine_smp_info* pInfo)
{
	Arch::CPU* pCpu = (Arch::CPU*)pInfo->extra_argument;
	
	pCpu->Init();
	
	if (!pCpu->m_bIsBSP)
		pCpu->Go();
}

volatile limine_smp_request g_SMPRequest =
{
	.id = LIMINE_SMP_REQUEST,
	.revision = 0,
	.response = NULL,
	.flags = 0,
};

// Initialize the CPUs from the bootloader's perspective.

limine_smp_response* Arch::CPU::GetSMPResponse()
{
	return g_SMPRequest.response;
}

uint64_t Arch::CPU::GetCount()
{
	return GetSMPResponse()->cpu_count;
}

void Arch::CPU::InitAsBSP()
{
	limine_smp_response* pSMP = g_SMPRequest.response;
	
	// Initialize all the CPUs in series.
	for (uint64_t i = 0; i < pSMP->cpu_count; i++)
	{
		bool bIsBSP = pSMP->bsp_lapic_id == pSMP->cpus[i]->lapic_id;
		
		// Note: The reason I don't just overload `operator new' with the eternal heap is because
		// I really do not want to do that.
		void* pCpuInfo = EternalHeap::Allocate(sizeof(CPU));
		
		pSMP->cpus[i]->extra_argument = (uint64_t)pCpuInfo;
		
		// Fill in the pCpuInfo structure by running a placement new on it.
		new (pCpuInfo) CPU (i, pSMP->cpus[i], bIsBSP);
		
		// Run
		
		if (bIsBSP)
		{
			Start(pSMP->cpus[i]);
		}
		else
		{
			__atomic_store_n(&pSMP->cpus[i]->goto_address, &Start, ATOMIC_DEFAULT_MEMORDER);
		}
	}
	
	while (g_CPUsInitialized.Load(ATOMIC_MEMORD_RELAXED) != (int)pSMP->cpu_count)
	{
		Spinlock::SpinHint();
	}
	
	char procs[] = "All X processors have been initialized.\n";
	procs[4] = '0' + pSMP->cpu_count;
	
	Terminal::Write(procs);
	
	CPU::GetCurrent()->Go();
}


#endif//TARGET_X86_64

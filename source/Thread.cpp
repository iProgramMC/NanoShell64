//  ***************************************************************
//  Thread.cpp - Creation date: 11/04/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#include <Arch.hpp>

extern "C" RETURNS_TWICE uint64_t SetThreadEC(Thread::ThreadExecutionContext* pEC);
extern "C" NO_RETURN     void    JumpThreadEC(Thread::ThreadExecutionContext* pEC, uint64_t value);

void Thread::Beginning()
{
	SLogMsg("Thread started!");
	Thread* pThread = Arch::CPU::GetCurrent()->GetScheduler()->GetCurrentThread();
	
	// call the entry point
	if (!pThread->m_EntryPoint)
	{
		SLogMsg("Warning: Starting a thread without an entry point is considered an error (Thread::Beginning)");
		pThread->Kill();
		return;
	}
	
	pThread->m_EntryPoint();
	pThread->Kill();
}

void Thread::SetEntryPoint(ThreadEntry pEntry)
{
	if (m_Status.Load() != SETUP)
	{
		SLogMsg("Calling Thread::SetEntryPoint(%p) on a thread (ID %d) which is in state %d is an error", pEntry, m_ID, m_Status.Load());
		return;
	}
	
	m_EntryPoint = pEntry;
}

void Thread::SetPriority(ePriority prio)
{
	m_Priority.Store(prio);
}

void Thread::Detach()
{
	m_bOwned.Store(false);
}

void Thread::Join()
{
	// you can't join an unjoinable thread
	if (!m_bOwned.Load())
		return;
	
	while (m_Status.Load() != ZOMBIE)
	{
		// TODO: yield our current thread?
		Spinlock::SpinHint();
	}
}

void Thread::SetStackSize(size_t sz)
{
	if (m_Status.Load() != SETUP)
	{
		SLogMsg("Calling Thread::SetStackSize(%z) on a thread (ID %d) which is in state %d is an error", sz, m_ID, m_Status.Load());
		return;
	}
	
	m_StackSize = sz;
}

void Thread::Kill()
{
	m_Status.Store(ZOMBIE);
	
	// note: I mean, yielding is harmless, but this is better to do
	if (this == m_pScheduler->GetCurrentThread())
		Yield();
}

void Thread::Suspend()
{
	m_Status.Store(SUSPENDED);
	
	if (this == m_pScheduler->GetCurrentThread())
		Yield();
}

void Thread::Resume()
{
	auto pCpu = Arch::CPU::GetCurrent();
	
	// avoid a TOCTOU bug:
	bool bOldState = pCpu->SetInterruptsEnabled(false);
	
	if (m_Status.Load() == SUSPENDED)
	{
		m_Status.Store(RUNNING);
	}
	
	pCpu->SetInterruptsEnabled(bOldState);
}

void Thread::Start()
{
	using namespace Arch;
	auto pCpu = CPU::GetCurrent();
	
	// This function starts the thread.
	
	// Clear interrupts. This prevents the scheduler from running during its manipulation:
	bool bOldState = pCpu->SetInterruptsEnabled(false);
	
	// Set up the stack.
	size_t nStackSizeLongs = m_StackSize / 8;
	
	m_pStack = new uint64_t[nStackSizeLongs];
	
	// Ensure the stack won't fault on access by faulting it in ourselves
	for (size_t i = 0; i < nStackSizeLongs; i++)
		m_pStack[i] = 0;
	
	// Preparing the execution context:
	
	// Set the execution context as 'here', to copy the rflags over.
	SetThreadEC(&m_ExecContext);
	
	// force the rflags to have interrupts enabled:
	m_ExecContext.rflags |= C_RFLAGS_INTERRUPT_FLAG;
	
	m_ExecContext.rip = (uint64_t)Thread::Beginning;
	m_ExecContext.rsp = (uint64_t)&m_pStack[nStackSizeLongs - 2];
	m_ExecContext.cs  = GDT::DESC_64BIT_RING0_CODE;
	m_ExecContext.ss  = GDT::DESC_64BIT_RING0_DATA;
	
	m_Status.Store(RUNNING);
	
	m_pScheduler->Done(this);
	
	// Restore the old interrupt state after we're done.
	pCpu->SetInterruptsEnabled(bOldState);
}

// static
void Thread::Yield()
{
	// clear interrupts.
	auto pCpu = Arch::CPU::GetCurrent();
	auto pSched = pCpu->GetScheduler();
	
	bool bOldState = pCpu->SetInterruptsEnabled(false);
	
	Thread* pThrd = pSched->GetCurrentThread();
	
	if (pThrd == nullptr)
	{
		pSched->Schedule();
		ASSERT_UNREACHABLE;
	}
	
	// save our execution point. If needed, we will return.
	if (SetThreadEC(&pThrd->m_ExecContext))
	{
		pCpu->SetInterruptsEnabled(bOldState);
		return;
	}
	
	pSched->Done(pThrd);
	pSched->Schedule();
	ASSERT_UNREACHABLE;
}

void Thread::JumpExecContext()
{
	using namespace Arch;
	CPU* pCpu = CPU::GetCurrent();
	
	pCpu->InterruptsEnabledRaw() = (m_ExecContext.rflags & C_RFLAGS_INTERRUPT_FLAG);	
	JumpThreadEC(&m_ExecContext, 1);
}

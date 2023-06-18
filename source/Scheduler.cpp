//  ***************************************************************
//  Scheduler.cpp - Creation date: 11/04/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#include <Scheduler.hpp>
#include <Arch.hpp>
#include <EternalHeap.hpp>

static Atomic<int> g_NextThreadID(1);

void Scheduler::IdleThread()
{
	while (true)
	{
		LogMsg("Idle thread from CPU %u", Arch::CPU::GetCurrent()->ID());
		//Thread::Yield();
		Arch::Halt();
	}
}
void Scheduler::Idle2Thread()
{	
	while (true)
	{
		LogMsg("Normal thread on CPU %u", Arch::CPU::GetCurrent()->ID());
		//Thread::Yield();
		Arch::Halt();
	}
}

Thread* Scheduler::CreateThread()
{
	Thread* pThrd = new(nopanic) Thread;
	
	pThrd->m_pScheduler = this;
	pThrd->m_ID  = g_NextThreadID.FetchAdd(1);
	
	m_AllThreads.AddBack(pThrd);
	
	return pThrd;
}

Thread* Scheduler::GetCurrentThread()
{
	return m_pCurrentThread;
}

void Scheduler::Init()
{
	// create an idle thread now
	Thread* pIdle = CreateThread();
	Thread* pIdle2 = CreateThread();
	
	if (!pIdle || !pIdle2)
		KernelPanic("could not create idle or idle2 thread");
	
	pIdle->SetEntryPoint(Scheduler::IdleThread);
	pIdle->SetPriority(Thread::NORMAL);
	pIdle->Start();
	pIdle->Detach();
	
	pIdle2->SetEntryPoint(Scheduler::Idle2Thread);
	pIdle2->SetPriority(Thread::NORMAL);
	pIdle2->Start();
	pIdle2->Detach();
}

// note that when a thread is scheduled for execution, it is removed from any queue
void Scheduler::Done(Thread* pThread)
{
	switch (pThread->m_Status.Load())
	{
		case Thread::RUNNING:
			m_ExecutionQueue.PushBack(pThread);
			break;
		case Thread::SETUP:
			// not sure how we got there. the scheduler really shouldn't
			// schedule threads during their setup phase
			ASSERT_UNREACHABLE;
			break;
		case Thread::ZOMBIE:
			// if this thread is owned, it's the owner's job to clean it up...
			if (pThread->m_bOwned.Load())
				break;
			// add it to the list of threads to dispose of
			m_ZombieThreads.AddBack(pThread);
			break;
		case Thread::SUSPENDED:
			// If this thread is now suspended, but has been running before, this means that
			// it does not belong to the suspended threads array and should be added there.
			m_SuspendedThreads.AddBack(pThread);
			break;
		default:
			ASSERT_UNREACHABLE;
			break;
	}
}

Thread* Scheduler::PopNextThread()
{
	Thread* pThrd = nullptr;
	
	if (m_ExecutionQueue.Size())
	{
		pThrd = m_ExecutionQueue.Front();
		m_ExecutionQueue.Erase(0);
	}
	
	return pThrd;
}

// looks through the list of suspended threads and checks if any are supposed to be unsuspended.
// Note: This could be a performance concern.
void Scheduler::CheckUnsuspensionConditions()
{
	// TODO
}

// looks through the list of zombie threads and kills them.
void Scheduler::CheckZombieThreads()
{
	// TODO
}

// this is only to be called from Thread::Yield!!!
void Scheduler::Schedule(bool bRunFromTimerIRQ)
{
	using namespace Arch;
	m_pCurrentThread = nullptr;
	
	CheckEvents();
	
	Thread* pThread = PopNextThread();
	
	// if no thread is to be executed, well.......
	if (!pThread)
	{
		KernelPanic("nothing to execute on CPU %u", Arch::CPU::GetCurrent()->ID());
	}
	
	// set this thread as the current thread.
	m_pCurrentThread = pThread;
	
	// set when the thread's time slice will expire:
	pThread->m_TimeSliceUntil = Arch::GetTickCount() + C_THREAD_MAX_TIME_SLICE;
	
	// schedule an interrupt for the next event:
	uint64_t currTime  = Arch::GetTickCount();
	uint64_t nextEvent = NextEvent();
	uint64_t timeWait  = nextEvent - 10 - currTime;
	
	APIC::ScheduleInterruptIn(timeWait);
	
	// if run from the timer IRQ, don't forget to send the APIC an EOI:
	if (bRunFromTimerIRQ)
		APIC::EndOfInterrupt();
	
	// go!
	m_pCurrentThread->JumpExecContext();
}

void Scheduler::DeleteThread(Thread* pThread)
{
	for (auto it = m_AllThreads.Begin(); it.Valid(); ++it)
	{
		if (*it != pThread) continue;
		
		m_AllThreads.Erase(it);
		
		return;
	}
}

void Scheduler::CheckEvents()
{
	CheckUnsuspensionConditions();
	CheckZombieThreads();
}

void Scheduler::OnTimerIRQ(Registers* pRegs)
{
	using namespace Arch;
	CPU* pCpu = CPU::GetCurrent();
	uint64_t currTime = Arch::GetTickCount();
	
	CheckEvents();
	
	// if we're running a thread right now... (note the reference. This is important)
	Thread* &t = m_pCurrentThread;
	if (t)
	{
		// If the thread's time slice has not expired yet, simply check for events, reprogram the APIC, and return.
		if (t->m_TimeSliceUntil - 100 > currTime)
		{
			uint64_t nextEvent = NextEvent();
			uint64_t timeWait  = nextEvent - 10 - currTime;
			Arch::APIC::ScheduleInterruptIn(timeWait);
			return;
		}
		
		// Save its context.
		t->m_bNeedRestoreAdditionalRegisters = true;
		
		Thread::AdditionalRegisters & ar = t->m_AdditionalRegisters;
		Thread::ExecutionContext    & ec = t->m_ExecContext;
		
		// this is long...
		ar.rax = pRegs->rax;
		ar.rcx = pRegs->rcx;
		ar.rdx = pRegs->rdx;
		ar.rsi = pRegs->rsi;
		ar.rdi = pRegs->rdi;
		ar.r8  = pRegs->r8;
		ar.r9  = pRegs->r9;
		ar.r10 = pRegs->r10;
		ar.r11 = pRegs->r11;
		ar.ds  = pRegs->ds;
		ar.es  = pRegs->es;
		ar.fs  = pRegs->fs;
		ar.gs  = pRegs->gs;
		ec.rbp = pRegs->rbp;
		ec.rbx = pRegs->rbx;
		ec.r12 = pRegs->r12;
		ec.r13 = pRegs->r13;
		ec.r14 = pRegs->r14;
		ec.r15 = pRegs->r15;
		ec.rip = pRegs->rip;
		ec.rsp = pRegs->rsp;
		ec.cs  = pRegs->cs;
		ec.ss  = pRegs->ss;
		ec.rflags = pRegs->rflags;
		
		//Mmark the thread as 'done' and set the current thread to nullptr, then schedule
		Done(t);
		t = nullptr;
	}
	
	Schedule(true);
}

uint64_t Scheduler::NextEvent()
{
	uint64_t currTime = Arch::GetTickCount();
	
	// Figure out when the next event will be.
	uint64_t time = currTime + C_THREAD_MAX_TIME_SLICE;
	
	if (m_pCurrentThread)
		time = m_pCurrentThread->m_TimeSliceUntil;
	
	// TODO: for each suspended thread, check if it's ready to be woken up
	
	return time;
}

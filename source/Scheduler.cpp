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
	pIdle->SetPriority(Thread::IDLE);
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
void Scheduler::Schedule()
{
	m_pCurrentThread = nullptr;
	
	CheckUnsuspensionConditions();
	
	CheckZombieThreads();
	
	Thread* pThread = PopNextThread();
	
	// if no thread is to be executed, well.......
	if (!pThread)
	{
		KernelPanic("nothing to execute on CPU %u", Arch::CPU::GetCurrent()->ID());
	}
	
	// set this thread as the current thread.
	m_pCurrentThread = pThread;
	
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

void Scheduler::OnTimerIRQ(Registers* pRegs)
{
	// TODO
}

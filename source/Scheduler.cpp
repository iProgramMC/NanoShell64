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
		LogMsg("Normal thread from CPU %u", Arch::CPU::GetCurrent()->ID());
		
		//Thread::Yield();
		Arch::Halt();
	}
}

Thread* Scheduler::CreateThread()
{
	// no threads to dish out, I'm afraid
	if (m_ThreadFreeList.Empty())
		return nullptr;
	
	Thread* pThread = m_ThreadFreeList.Front();
	m_ThreadFreeList.PopFront();
	
	// Initialize it with a placement new.
	new (pThread) Thread;
	
	return pThread;
}

Thread* Scheduler::GetCurrentThread()
{
	return m_pCurrentThread;
}

void Scheduler::Init()
{
	m_pThreadArray = (Thread*) EternalHeap::Allocate(MIN_THREADS * sizeof(Thread));
	
	for (size_t i = 0; i < MIN_THREADS; i++)
	{
		// call placement new on the thread to initialize it
		m_ThreadFreeList.AddBack(new(&m_pThreadArray[i]) Thread);
		
		m_pThreadArray[i].m_ID = int(i);
		m_pThreadArray[i].m_pScheduler = this;
	}
	
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
			switch (pThread->m_Priority.Load())
			{
				case Thread::IDLE:
					m_IdleExecutionQueue.AddBack(pThread);
					break;
				case Thread::NORMAL:
					m_ExecutionQueue.AddBack(pThread);
					break;
				case Thread::REALTIME:
					m_RTExecutionQueue.AddBack(pThread);
					break;
			}
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
	// look in the realtime priority queue
	if (!m_RTExecutionQueue.Empty())
	{
		Thread *pThread = m_RTExecutionQueue.Front();
		m_RTExecutionQueue.PopFront();
		return pThread;
	}
	
	// look in the normal priority queue
	if (!m_ExecutionQueue.Empty())
	{
		Thread *pThread = m_ExecutionQueue.Front();
		m_ExecutionQueue.PopFront();
		return pThread;
	}
	
	// nothing in the normal queue, check the idle queue now
	if (!m_IdleExecutionQueue.Empty())
	{
		Thread *pThread = m_IdleExecutionQueue.Front();
		m_IdleExecutionQueue.PopFront();
		return pThread;
	}
	
	return nullptr;
}

// looks through the list of suspended threads and checks if any are supposed to be unsuspended.
// Note: This could be a performance concern.
void Scheduler::CheckUnsuspensionConditions()
{
	// .....
}

// this is only to be called from Thread::Yield!!!
void Scheduler::Schedule()
{
	m_pCurrentThread = nullptr;
	
	CheckUnsuspensionConditions();
	
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











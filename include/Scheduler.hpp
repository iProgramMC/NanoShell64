//  ***************************************************************
//  Scheduler.hpp - Creation date: 11/04/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#ifndef _SCHEDULER_HPP
#define _SCHEDULER_HPP

#include <Thread.hpp>
#include <KList.hpp>
#include <KPriorityQueue.hpp>

// Forward declare the CPU class since we need it as a friend of Scheduler.
namespace Arch
{
	class CPU;
}

struct Thread_ExecQueueComparator
{
	bool operator() (Thread* threadA, Thread* threadB) const
	{
		return threadA->m_Priority.Load() > threadB->m_Priority.Load();
	}
};

// The way this works is simple. When a thread is to be scheduled, the pointer:
// - is popped off the relevant queue
// - is placed as "the current thread"
// - the old "current thread" is placed on the relevant queue, or the suspended threads list.

class Scheduler
{
	// Maximum time slice for a thread.
	constexpr static uint64_t C_THREAD_MAX_TIME_SLICE = 1'000'000;
	
public:
	// Creates a new thread object.
	Thread* CreateThread();
	
protected:
	friend class Arch::CPU;
	friend class Thread;
	
	// Initializes the scheduler.
	void Init();
	
	// Pops the next thread from the relevant execution queue.
	Thread* PopNextThread();
	
	// Gets the current thread.
	Thread* GetCurrentThread();
	
	// Let the scheduler know that this thread's quantum is over.
	void Done(Thread* pThrd);
	
	// Schedules in a new thread. This is used within Thread::Yield(), so use that instead.
	void Schedule(bool bRunFromTimerIRQ);
	
	// The function run when an interrupt comes in.
	void OnTimerIRQ(Registers* pRegs);
	
private:
	// A list of ALL threads ever.
	KList<Thread*> m_AllThreads;
	
	KList<Thread*> m_ThreadFreeList;
	
	KPriorityQueue<Thread*, Thread_ExecQueueComparator> m_ExecutionQueue;
	
	KList<Thread*> m_SuspendedThreads;
	KList<Thread*> m_ZombieThreads;      // Threads to clean up and dispose.
	
	Thread *m_pCurrentThread = nullptr;
	
	static void IdleThread();
	static void Idle2Thread();
	
	void DeleteThread(Thread* pThread);
	
	// Looks for the next event that will happen, such as a thread wake up.
	uint64_t NextEvent();
	
	// For each suspended thread, check if it's suspended anymore.
	void CheckUnsuspensionConditions();
	
	// Kill every zombie thread that isn't owned by anybody.
	void CheckZombieThreads();
	
	// Check for events for the scheduler.
	void CheckEvents();
};

#endif//_SCHEDULER_HPP


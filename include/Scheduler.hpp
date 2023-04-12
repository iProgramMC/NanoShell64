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

// Forward declare the CPU class since we need it as a friend of Scheduler.
namespace Arch
{
	class CPU;
}

class Scheduler
{
	static constexpr int MIN_THREADS = 4096;
	
	// A limited set of thread objects. This cannot be expanded, sadly. TODO
	Thread* m_pThreadArray = nullptr;
	
	KList<Thread*> m_ThreadFreeList;
	KList<Thread*> m_ExecutionQueue;
	KList<Thread*> m_IdleExecutionQueue; // The execution queue for idle threads.
	KList<Thread*> m_SuspendedThreads;
	KList<Thread*> m_ZombieThreads;      // Threads to clean up and dispose.
	
	Thread *m_pCurrentThread = nullptr;
	
	static void IdleThread();
	static void Idle2Thread();
	
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
	
	// For each suspended thread, check if it's suspended anymore.
	void CheckUnsuspensionConditions();
	
	// Schedules in a new thread. This is used within Thread::Yield(), so use that instead.
	void Schedule();
	
public:
	// Creates a new thread object.
	Thread* CreateThread();
};

#endif//_SCHEDULER_HPP


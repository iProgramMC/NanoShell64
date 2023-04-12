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
	
	Thread *m_pCurrentThread = nullptr;
	
	static void IdleThread();
	
protected:
	friend class Arch::CPU;
	
	// Initializes the scheduler.
	void Init();
	
	// Gets the next thread on the execution queue.
	Thread* GetNextThread();
	
public:
	// Creates a new thread object.
	Thread* CreateThread();
};

#endif//_SCHEDULER_HPP


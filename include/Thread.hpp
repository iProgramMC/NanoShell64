//  ***************************************************************
//  Thread.hpp - Creation date: 11/04/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#ifndef _THREAD_HPP
#define _THREAD_HPP

#include <Nanoshell.hpp>
#include <Spinlock.hpp>

/**
	Explanation on how thread creation and deletion would be done:
	
Step 1: Creating the thread object:
	
	```
	Thread* thread = Scheduler::CreateThread();
	```
		
	This creates a thread on the current CPU. (could be changed)
	
Step 2: Set up properties about the thread, such as its priority.

	```
	thread->SetPriority(Thread::NORMAL);
	// ...
	```
	
Step 3: Starting the thread:
	```
	thread->Start();
	```
	
Note: There are now 2 ways of operating on a thread object:
	
	1. Detach the thread. This allows the kernel to destroy the thread
	automatically upon its exit, however, the ownership of this thread
	object is forfeited. After a Thread::Detach() call, assume that the
	thread is no longer valid. (While it may still be valid, it's good
	to assume the worst. A good strategy in avoiding bugs.)
	
	Note: As soon as the thread is detached, it can also be relocated to
	any other running CPU. The scheduler may pass threads around to balance
	the load on the system. Another reason to consider the thread invalid
	after a detach call.
	```
	thread->Detach();
	```
	
	2. Join the thread. This waits for the thread's death. If the thread
	has died, this does nothing.
	```
	thread->Join();
	```
**/

// Forward declaration of the scheduler class. We would like to later give this class
// access to our protected members.
class Scheduler;

typedef void(*ThreadEntry)();

class Thread
{
public:
	enum ePriority
	{
		IDLE,       // Idle priority. This thread will only be run when no other threads can be scheduled.
		NORMAL,     // Normal priority. This thread will execute normally along with other threads.
	};
	
	enum eStatus
	{
		SUSPENDED, // The thread has been manually suspended.
		RUNNING,   // The thread is active.
		ZOMBIE,    // The thread has completely died. The owner of this thread object now has to clean it up.
	};
	
	/**** Protected variables. ****/
protected:
	// The scheduler manages the thread linked queue. We will give it permission
	// to access our stuff below:
	friend class Scheduler;
	
	// The ID of the thread.
	int m_ID = 0;
	
	// The next and previous items in the thread queue.
	Thread *m_Prev = nullptr, *m_Next = nullptr;
	
	// The priority of the thread.
	Atomic<ePriority> m_Priority { NORMAL };
	
	// The status of the thread.
	Atomic<eStatus> m_Status { SUSPENDED };
	
	// If the thread is currently owned by the creator thread.
	// Detach() sets this to false.
	Atomic<bool> m_bOwned { true };
	
	// The owner scheduler.
	Scheduler* m_pScheduler;
	
	// Entry point of the thread.
	ThreadEntry m_EntryPoint;
	
public:
	// This sets the entry point of the thread.
	// This is only possible before the Start() function is called.
	void SetEntryPoint(ThreadEntry pEntry);
	
	// This starts the thread object. This is meant to be called
	// after the thread's properties have been setup.
	void Start();
	
	// Suspends the thread's execution.
	void Suspend();
	
	// Resumes the thread's execution, if it was suspended.
	void Resume();
	
	// Marks the thread as a zombie. It will get removed by the scheduler
	// at some point in time, so best to assume the thread object is invalid
	// after the call right away.
	void Kill();
	
	// Set the priority of this thread.
	void SetPriority(ePriority prio);
	
	// Detaches a thread from the current thread of execution.
	// This forfeits control of this thread object to the scheduler.
	void Detach();
	
	// Waits until the thread exits. This is not possible if the thread
	// has been detached.
	void Join();
	
	
	// Yields execution of the current thread.
	static void Yield();
};

#endif//_THREAD_HPP
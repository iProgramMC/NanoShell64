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
		Arch::Halt();
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
	
	// Add it to the suspended threads list.
	m_SuspendedThreads.AddBack(pThread);
	
	return pThread;
}

void Scheduler::Init()
{
	LogMsg("Thread sizeof is %d",sizeof(Thread));
	m_pThreadArray = (Thread*) EternalHeap::Allocate(MIN_THREADS * sizeof(Thread));
	
	for (size_t i = 0; i < MIN_THREADS; i++)
		// call placement new on the thread to initialize it
		m_ThreadFreeList.AddBack(new(&m_pThreadArray[i]) Thread);
	
	// create an idle thread now
	Thread* pIdle = CreateThread();
	
	if (!pIdle)
		KernelPanic("could not create idle thread");
	
	pIdle->SetEntryPoint(Scheduler::IdleThread);
	pIdle->SetPriority(Thread::IDLE);
	//pIdle->Start();
	pIdle->Detach();
}

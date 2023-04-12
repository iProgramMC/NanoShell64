//  ***************************************************************
//  Thread.cpp - Creation date: 11/04/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#include <Scheduler.hpp>

void Thread::SetEntryPoint(ThreadEntry pEntry)
{
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

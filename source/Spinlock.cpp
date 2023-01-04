//  ***************************************************************
//  Spinlock.cpp - Creation date: 04/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include <Spinlock.hpp>

void Spinlock::SpinHint()
{
	#ifdef TARGET_X86_64
		__builtin_ia32_pause();
	#else
		#warning "Spinlock may benefit from adding a pause instruction or similar"
	#endif
}

bool Spinlock::IsLocked() const
{
	return m_lockBool.Load();
}

bool Spinlock::TryLock()
{
	// "Exchange" puts "true" into the m_lockBool atomic boolean, and
	// returns whatever was before.  If there was a 'false' it means
	// we have acquired the lock and we are all good.
	return !m_lockBool.Exchange(true, ATOMIC_MEMORD_ACQUIRE);
}

void Spinlock::Lock()
{
	while (true)
	{
		if (TryLock())
			break;
		
		// Spin until the lock is acquirable again.
		while (!m_lockBool.Load(ATOMIC_MEMORD_RELAXED))
		{
			SpinHint();
		}
	}
}

void Spinlock::Unlock()
{
	if (!m_lockBool.Exchange(false, ATOMIC_MEMORD_RELEASE))
	{
		// TODO: Error out. We're trying to release an unlocked spinlock.
	}
}

LockGuard::LockGuard(Spinlock& lock) : m_lock(lock)
{
	m_lock.Lock();
}

LockGuard::LockGuard(Spinlock& lock, AdoptLock) : m_lock(lock)
{
}

LockGuard::~LockGuard()
{
	m_lock.Unlock();
}

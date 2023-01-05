//  ***************************************************************
//  Spinlock.hpp - Creation date: 04/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#ifndef _SPINLOCK_HPP
#define _SPINLOCK_HPP

#include "Atomic.hpp"

class Spinlock
{
private:
	Atomic<bool> m_lockBool;
	
public:
	Spinlock() : m_lockBool(false) {}

	// Checks if a spin lock is locked. Not sure why you would need this.
	bool IsLocked() const
	{
		return m_lockBool.Load();
	}
	
	// Tries to lock the current Spinlock object. If the lock is already
	// taken, this will return 'false', but if the lock has been acquired
	// through this function, this will return 'true'.
	bool TryLock()
	{
		return !m_lockBool.TestAndSet(ATOMIC_MEMORD_ACQUIRE);
	}
	
	// Lock the current Spinlock object. If the lock is already taken at
	// the time of this call, this function will spin and wait until it's
	// no longer locked.
	inline void Lock()
	{
		while (true)
		{
			if (!m_lockBool.TestAndSet(ATOMIC_MEMORD_ACQUIRE)) break;
			while (m_lockBool.Load(ATOMIC_MEMORD_ACQUIRE))
			{
				Spinlock::SpinHint();
			}
		}
	}
	
	// Unlock the current Spinlock object.
	void Unlock()
	{
		m_lockBool.Clear(ATOMIC_MEMORD_RELEASE);
	}
	
	// Hints to the current processor that it is currently spinning.
	// This is done on x86_64 with a "pause" instruction.
	static void SpinHint()
	{
		#ifdef TARGET_X86_64
			__builtin_ia32_pause();
		#else
			#warning "Spinlock may benefit from adding a pause instruction or similar"
		#endif
	}
};

// Note: Currently the only viable locking strategy to implement right now is
// AdoptLock. TryToLock implicitly depends on OwnsLock, and since we don't have
// threading yet we can't really implement it. And DeferLock... I don't see its
// purpose just yet.

// Empty tag class used in LockGuard to let it know that we already own
// the lock. Behavior is undefined if this is used on an unlocked lock,
// or a lock that's already been locked by another context.
class AdoptLock
{
	explicit AdoptLock() = default;
};

// The lock guard locks a spin lock throughout its lifetime. Best used as
// a stack-allocated object, this guarantees that the lock will never be
// left locked.
class LockGuard
{
private:
	Spinlock &m_lock;
	
public:
	// Constructs a LockGuard object which adopts the passed in lock and locks it.
	LockGuard(Spinlock& lock);
	
	// Constructs a LockGuard object which adopts the passed in lock but doesn't lock it by itself.
	LockGuard(Spinlock& lock, AdoptLock);
	
	// Constructs a LockGuard object 
	
	// This will delete any LockGuard copiers. This is an object which may not be copied.
	LockGuard(const LockGuard &) = delete;
	LockGuard& operator=(const LockGuard &) = delete;
	
	// The destructor unlocks the lock associated with this object.
	~LockGuard();
};

#endif//_SPINLOCK_HPP
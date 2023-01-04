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
	// Checks if a spin lock is locked. Not sure why you would need this.
	bool IsLocked() const;
	
	// Lock the current SpinLock object.
	void Lock();
	
	// Unlock the current SpinLock object.
	void Unlock();
	
	// Hints to the current processor that it is currently spinning.
	// This is done on x86_64 with a "pause" instruction.
	static void SpinHint();
};

#endif//_SPINLOCK_HPP
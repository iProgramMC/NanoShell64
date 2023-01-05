//  ***************************************************************
//  Spinlock.cpp - Creation date: 04/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include <Spinlock.hpp>

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

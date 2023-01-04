//  ***************************************************************
//  Atomic.hpp - Creation date: 04/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#ifndef _ATOMIC_HPP
#define _ATOMIC_HPP

// If our compiler claims we're hosted, we're really not..

#if __STDC_HOSTED__ || defined(__STDC_NO_ATOMICS__)
#error "Hey"
#endif

// TODO: Allow default atomic memory order to be changed?
#define ATOMIC_DEFAULT_MEMORDER __ATOMIC_SEQ_CST

template <typename T>
class Atomic
{
private:
	T m_content;

public:
	Atomic()
	{
		
	}
	
	Atomic(T init)
	{
		__atomic_store_n(&m_content, init, ATOMIC_DEFAULT_MEMORDER);
	}
	
	void Load(T* ret)
	{
		__atomic_load(&m_content, ret, ATOMIC_DEFAULT_MEMORDER);
	}
	
	void FetchAdd(T value)
	{
		__atomic_add_fetch(&m_content, value, ATOMIC_DEFAULT_MEMORDER);
	}
};


#endif//_ATOMIC_HPP

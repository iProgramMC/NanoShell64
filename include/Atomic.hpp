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

// This is a wrapper for C++ atomic builtins.

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
	
	void Load(T* ret) const
	{
		__atomic_load(&m_content, ret, ATOMIC_DEFAULT_MEMORDER);
	}
	
	T Load() const
	{
		return __atomic_load_n(&m_content, ATOMIC_DEFAULT_MEMORDER);
	}
	
	void Store(T store)
	{
		__atomic_store_n(&m_content, store, ATOMIC_DEFAULT_MEMORDER);
	}
	
	T Exchange(T val)
	{
		return __atomic_exchange_n(&m_content, val, ATOMIC_DEFAULT_MEMORDER);
	}
	
	// This class of operations fetches the result AFTER the operation is performed.
	
	T AddFetch(T value)
	{
		return __atomic_add_fetch(&m_content, value, ATOMIC_DEFAULT_MEMORDER);
	}
	
	T SubFetch(T value)
	{
		return __atomic_sub_fetch(&m_content, value, ATOMIC_DEFAULT_MEMORDER);
	}
	
	T AndFetch(T value)
	{
		return __atomic_and_fetch(&m_content, value, ATOMIC_DEFAULT_MEMORDER);	
	}
	
	T NandFetch(T value)
	{
		return __atomic_nand_fetch(&m_content, value, ATOMIC_DEFAULT_MEMORDER);	
	}
	
	T OrFetch(T value)
	{
		return __atomic_or_fetch(&m_content, value, ATOMIC_DEFAULT_MEMORDER);	
	}
	
	T XorFetch(T value)
	{
		return __atomic_xor_fetch(&m_content, value, ATOMIC_DEFAULT_MEMORDER);	
	}
	
	// This class of operations fetches the result and THEN performs the operation. Otherwise, basically the same.
	
	T FetchAdd(T value)
	{
		return __atomic_fetch_add(&m_content, value, ATOMIC_DEFAULT_MEMORDER);
	}
	
	T FetchSub(T value)
	{
		return __atomic_fetch_sub(&m_content, value, ATOMIC_DEFAULT_MEMORDER);
	}
	
	T FetchAnd(T value)
	{
		return __atomic_fetch_and(&m_content, value, ATOMIC_DEFAULT_MEMORDER);
	}
	
	T FetchNand(T value)
	{
		return __atomic_fetch_nand(&m_content, value, ATOMIC_DEFAULT_MEMORDER);
	}
	
	T FetchOr(T value)
	{
		return __atomic_fetch_or(&m_content, value, ATOMIC_DEFAULT_MEMORDER);
	}
	
	T FetchXor(T value)
	{
		return __atomic_fetch_xor(&m_content, value, ATOMIC_DEFAULT_MEMORDER);
	}
	
	void Exchange(T* val, T* ret)
	{
		return __atomic_exchange(&m_content, val, ret, ATOMIC_DEFAULT_MEMORDER);
	}
	
	bool CompareExchange(T* expected, T desired, bool weak)
	{
		return __atomic_compare_exchange_n(&m_content, expected, weak, ATOMIC_DEFAULT_MEMORDER, ATOMIC_DEFAULT_MEMORDER);
	}
};


#endif//_ATOMIC_HPP

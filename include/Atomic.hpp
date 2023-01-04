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

#define ATOMIC_MEMORD_SEQ_CST __ATOMIC_SEQ_CST
#define ATOMIC_MEMORD_ACQ_REL __ATOMIC_ACQ_REL
#define ATOMIC_MEMORD_ACQUIRE __ATOMIC_ACQUIRE
#define ATOMIC_MEMORD_RELEASE __ATOMIC_RELEASE
#define ATOMIC_MEMORD_CONSUME __ATOMIC_CONSUME
#define ATOMIC_MEMORD_RELAXED __ATOMIC_RELAXED

// TODO: Allow default atomic memory order to be changed?
#define ATOMIC_DEFAULT_MEMORDER ATOMIC_MEMORD_SEQ_CST

template <typename T>
class Atomic
{
private:
	T m_content;

public:
	Atomic()
	{
		
	}
	
	Atomic(T init, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		__atomic_store_n(&m_content, init, memoryOrder);
	}
	
	void Load(T* ret, int memoryOrder = ATOMIC_DEFAULT_MEMORDER) const
	{
		__atomic_load(&m_content, ret, memoryOrder);
	}
	
	T Load(int memoryOrder = ATOMIC_DEFAULT_MEMORDER) const
	{
		return __atomic_load_n(&m_content, memoryOrder);
	}
	
	void Store(T store, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		__atomic_store_n(&m_content, store, memoryOrder);
	}
	
	// This class of operations fetches the result AFTER the operation is performed.
	
	T AddFetch(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_add_fetch(&m_content, value, memoryOrder);
	}
	
	T SubFetch(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_sub_fetch(&m_content, value, memoryOrder);
	}
	
	T AndFetch(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_and_fetch(&m_content, value, memoryOrder);	
	}
	
	T NandFetch(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_nand_fetch(&m_content, value, memoryOrder);	
	}
	
	T OrFetch(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_or_fetch(&m_content, value, memoryOrder);	
	}
	
	T XorFetch(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_xor_fetch(&m_content, value, memoryOrder);	
	}
	
	// This class of operations fetches the result and THEN performs the operation. Otherwise, basically the same.
	
	T FetchAdd(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_fetch_add(&m_content, value, memoryOrder);
	}
	
	T FetchSub(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_fetch_sub(&m_content, value, memoryOrder);
	}
	
	T FetchAnd(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_fetch_and(&m_content, value, memoryOrder);
	}
	
	T FetchNand(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_fetch_nand(&m_content, value, memoryOrder);
	}
	
	T FetchOr(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_fetch_or(&m_content, value, memoryOrder);
	}
	
	T FetchXor(T value, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_fetch_xor(&m_content, value, memoryOrder);
	}
	
	void Exchange(T* val, T* ret, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		__atomic_exchange(&m_content, val, ret, memoryOrder);
	}
	
	T Exchange(T val, int memoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_exchange_n(&m_content, val, memoryOrder);
	}
	
	bool CompareExchange(T* expected, T desired, bool weak, int successMemoryOrder = ATOMIC_DEFAULT_MEMORDER, int failureMemoryOrder = ATOMIC_DEFAULT_MEMORDER)
	{
		return __atomic_compare_exchange_n(&m_content, expected, weak, successMemoryOrder, failureMemoryOrder);
	}
};


#endif//_ATOMIC_HPP

//  ***************************************************************
//  KArray.hpp - Creation date: 02/05/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#ifndef _KARRAY_HPP
#define _KARRAY_HPP

// NOTE: This structure is NOT thread safe.

// NOTE: TODO: Use the kernel heap directly instead of initializing extra elements that will go unused.

// This will mostly be used with pointer types, so this is probably fine.

template<typename T>
class KArray
{
public:
	KArray()
	{
		m_container = nullptr;
		m_container_capacity = 0;
		m_container_size     = 0;
	}
	~KArray()
	{
		if (m_container)
		{
			delete[] m_container;
			m_container = nullptr;
			m_container_size = 0;
			m_container_capacity = 0;
		}
	}
	
	void Reserve(size_t sz)
	{
		if (m_container_capacity == 0)
		{
			SetupDefaultContainer();
			// call reserve() again;
			
			Reserve(sz);
			
			return;
		}
		
		if (sz == 0) return; // considered an error
		if (sz == m_container_capacity) return;
		
		if (sz < m_container_size)
			sz = m_container_size;
		
		T* newData = new T[sz];
		for (size_t i = 0; i < m_container_size; i++)
		{
			newData[i] = m_container[i];
		}
		
		T* oldContainer = m_container;
		m_container = newData;
		
		delete[] oldContainer;
	}
	
	virtual void PushBack(const T &t)
	{
		if (m_container_size >= m_container_capacity)
			Reserve(m_container_capacity * 2);
		
		m_container[m_container_size++] = t;
	}
	
	T& Front()
	{
		return m_container[0];
	}
	
	T& Back()
	{
		return m_container[m_container_size - 1];
	}
	
	T* At(size_t x)
	{
		if (x >= m_container_size) return nullptr;
		
		return &m_container[x];
	}
	
	// unsafe access
	T& operator[](size_t index)
	{
		return m_container[index];
	}
	
	virtual void Erase(size_t index)
	{
		if (index >= m_container_size) return;
		
		for (size_t i = index + 1; i < m_container_size; i++)
			m_container[i] = m_container[i + 1];
		
		m_container_size--;
	}
	
	// this is faster, but doesn't keep the order
	virtual void EraseUnordered(size_t index)
	{
		if (index >= m_container_size) return;
		
		m_container[index] = m_container[--m_container_size];
	}
	
	void Clear()
	{
		delete m_container;
		SetupDefaultContainer();
	}
	
	size_t Size()
	{
		return m_container_size;
	}
	
	size_t Capacity()
	{
		return m_container_capacity;
	}
	
private:
	void SetupDefaultContainer()
	{
		m_container_capacity = 16; // the default.
		m_container = new T[m_container_capacity];
		m_container_size = 0;
	}
	
private:
	T* m_container;
	size_t m_container_size;
	size_t m_container_capacity;
};

#endif

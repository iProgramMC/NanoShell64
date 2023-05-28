//  ***************************************************************
//  KPriorityQueue.hpp - Creation date: 02/05/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#ifndef _KPRIORITYQUEUE_HPP
#define _KPRIORITYQUEUE_HPP

#include <KArray.hpp>

// Maybe we'll move these to a different header file.
template <typename T>
struct KGreater
{
	bool operator() (const T& a, const T& b) const
	{
		return a > b;
	}
};

// This is implemented with KArray and a binary max heap on top.

// Note: Why won't the compiler simply accept "Size()" instead of "this->Size()"?

// Comp(a, b) essentially calls a > b by default.
template <typename T, typename Comp = KGreater<T>>
class KPriorityQueue : public KArray<T>
{
public:
	void PushBack(const T& t) override
	{
		KArray<T>::PushBack(t);
		SortUp(this->Size() - 1);
	}
	
	void Erase(size_t index) override
	{
		if (index >= this->Size()) return;
		
		KArray<T>::EraseUnordered(index);
		SortDown(index);
	}
	
	void EraseUnordered(size_t index) override
	{
		this->Erase(index);
	}
	
private:
	void SortUp(size_t index)
	{
		if (index >= this->Size()) return;
		
		if (index == 0) // already at the top
			return;
		
		auto& me = *this;
		
		Comp comp;
		
		size_t parentIndex = (index - 1) / 2;
		while (true)
		{
			if (parentIndex == index) break;
			
			T& parent = me[parentIndex];
			
			if (comp(me[index], parent))
			{
				T temp = parent;
				parent = me[index];
				me[index] = temp;
			}
			else break;
			
			index = parentIndex;
			
			if (index == 0) break;
			
			parentIndex = (index - 1) / 2;
		}
	}
	
	void SortDown(size_t index)
	{
		if (index >= this->Size()) return;
		
		auto& me = *this;
		
		Comp comp;
		
		while (true)
		{
			size_t ciLeft  = index * 2 + 1;
			size_t ciRight = index * 2 + 2;
			size_t swapIdx = 0;
			
			if (ciLeft < this->Size())
			{
				swapIdx = ciLeft;
				
				if (ciRight < this->Size() && comp(me[ciRight], me[ciLeft]))
					swapIdx = ciRight;
			}
			else break;
			
			if (comp(me[swapIdx], me[index])) // should change this to !comp I guess?
			{
				T temp = me[swapIdx];
				me[swapIdx] = me[index];
				me[index] = temp;
				
				index = swapIdx;
			}
			else break;
		}
	}
};

#endif

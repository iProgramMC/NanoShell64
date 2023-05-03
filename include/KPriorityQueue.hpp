//  ***************************************************************
//  KPriorityQueue.hpp - Creation date: 12/04/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include <KArray.hpp>

// This is implemented with KArray and a binary max heap on top.

// Note: Why won't the compiler simply accept "Size()" instead of "this->Size()"?

template <typename T>
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
		
		KPriorityQueue<T>& me = *this;
		
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
		
		KPriorityQueue<T>& me = *this;
		
		size_t parentIndex = (index - 1) / 2;
		while (true)
		{
			if (parentIndex == index) break;
			
			T& parent = me[parentIndex];
			
			if (parent < me[index])
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
		
		KPriorityQueue<T>& me = *this;
		
		while (true)
		{
			size_t ciLeft  = index * 2 + 1;
			size_t ciRight = index * 2 + 2;
			size_t swapIdx = 0;
			
			if (ciLeft < this->Size())
			{
				swapIdx = ciLeft;
				
				if (ciRight < this->Size() && me[ciLeft] < me[ciRight])
					swapIdx = ciRight;
			}
			else break;
			
			if (me[index] < me[swapIdx])
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

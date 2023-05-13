//  ***************************************************************
//  KList.hpp - Creation date: 12/04/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#ifndef _KLIST_HPP
#define _KLIST_HPP

// NOTE: This structure is NOT thread safe.

// This linked list based structure can be used as either a linked list,
// or a queue, or a deque (double-ended queue). The scheduler uses this
// data structure in all three ways.

template<typename T>
class KList
{
	class ListNode
	{
		friend class KList;
		
		ListNode *m_pPrev = nullptr, *m_pNext = nullptr;
		
		T m_data;
		
	public:
		ListNode(T data)
		{
			m_data = data;
		}
	};
	
	class ListNodeIterator
	{
		friend ListNode;
		friend KList;
		
		ListNode* m_pNode;
		
	public:
		ListNodeIterator(ListNode* pNode)
		{
			m_pNode = pNode;	
		}
		
		T& operator*() const
		{
			return m_pNode->m_data;
		}
		
		bool Valid() const
		{
			return m_pNode != nullptr;
		}
		
		ListNodeIterator& operator++()
		{
			m_pNode = m_pNode->m_pNext;
			
			return (*this);
		}
		
		ListNodeIterator operator++(UNUSED int unused)
		{
			ListNodeIterator iter = (*this);
			++(*this);
			return iter;
		}
		
		ListNodeIterator& operator--()
		{
			m_pNode = m_pNode->m_pPrev;
			
			return (*this);
		}
		
		ListNodeIterator operator--(UNUSED int unused)
		{
			ListNodeIterator iter = (*this);
			--(*this);
			return iter;
		}
	};
	
	ListNode *m_pFirst = nullptr, *m_pLast = nullptr;
	
public:
	bool Empty()
	{
		return m_pFirst == nullptr && m_pLast == nullptr;
	}
	
	void AddBack(const T& element)
	{
		ListNode* pNode = new ListNode(element);
		
		if (m_pLast)
			m_pLast->m_pNext = pNode;
		
		pNode->m_pPrev = m_pLast;
		
		m_pLast = pNode;
		
		if (!m_pFirst)
			m_pFirst = pNode;
	}
	
	void AddFront(const T& element)
	{
		ListNode* pNode = new ListNode(element);
		
		if (m_pFirst)
			m_pFirst->m_pPrev = pNode;
		
		pNode->m_pNext = m_pFirst;
		
		m_pFirst = pNode;
		
		if (!m_pLast)
			m_pLast = pNode;
	}
	
	~KList()
	{
		while (!Empty())
			PopBack();
	}
	
	T Front()
	{
		// WARNING: Don't call this while the list is empty!
		return m_pFirst->m_data;
	}
	
	T Back()
	{
		// WARNING: Don't call this while the list is empty!
		return m_pLast->m_data;
	}
	
	ListNodeIterator Begin()
	{
		return ListNodeIterator(m_pFirst);
	}
	
	ListNodeIterator End()
	{
		return ListNodeIterator(m_pLast);
	}
	
	void PopFront()
	{
		if (Empty()) return;
		
		ListNode* pNode = m_pFirst->m_pNext;
		
		if (!pNode)
		{
			// this is the only node.
			delete m_pFirst;
			
			m_pFirst = m_pLast = nullptr;
			return;
		}
		
		pNode->m_pPrev = nullptr;
		
		delete m_pFirst;
		m_pFirst = pNode;
	}
	
	void Remove(ListNode* pNode)
	{
		if (m_pFirst == pNode)
			m_pFirst =  m_pFirst->m_pNext;
		if (m_pLast  == pNode)
			m_pLast  =  m_pLast ->m_pPrev;
		if (pNode->m_pPrev)
			pNode->m_pPrev->m_pNext = pNode->m_pNext;
		if (pNode->m_pNext)
			pNode->m_pNext->m_pPrev = pNode->m_pPrev;
		delete pNode;
	}
	
	void Erase(const ListNodeIterator& iter)
	{
		if (!iter.Valid())
			return;
		
		Remove(iter.m_pNode);
	}
	
	void PopBack()
	{
		if (Empty()) return;
		
		ListNode* pNode = m_pLast->m_pPrev;
		
		if (!pNode)
		{
			// this is the only node.
			delete m_pLast;
			
			m_pLast = m_pFirst = nullptr;
			return;
		}
		
		pNode->m_pNext = nullptr;
		delete m_pLast;
		m_pLast = pNode;
	}
};

#endif

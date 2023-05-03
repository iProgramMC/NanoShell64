//  ***************************************************************
//  KList.hpp - Creation date: 12/04/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

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
		
		T m_pData;
		
	public:
		T& operator*()
		{
			return m_pData;
		}
		
		T* operator->()
		{
			return &m_pData;
		}
		
		ListNode(T pData)
		{
			m_pData = pData;
		}
		
		ListNode* Next()
		{
			return m_pNext;
		}
		
		ListNode* Prev()
		{
			return m_pPrev;
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
		return **m_pFirst;
	}
	
	T Back()
	{
		// WARNING: Don't call this while the list is empty!
		return *m_pLast;
	}
	
	ListNode* FrontNode()
	{
		return m_pFirst;
	}
	
	ListNode* BackNode()
	{
		return m_pLast;
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

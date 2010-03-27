#include "stdafx.h"
#include "../include/sp_ext_array.h"
#include <windows.h>

#ifndef WIN32

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

#endif

namespace sp_ext
{
///  ptr_array
	
ptr_array::ptr_array()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

ptr_array::~ptr_array()
{
	delete[] (BYTE*)m_pData;
}

void ptr_array::SetSize(int nNewSize, int nGrowBy)
{
	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size
	
	if (nNewSize == 0)
	{
		// shrink to nothing
		delete[] (BYTE*)m_pData;
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
		m_pData = (void**) new BYTE[nNewSize * sizeof(void*)];
		
		memset(m_pData, 0, nNewSize * sizeof(void*));  // zero fill
		
		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements
			
			memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));
			
		}
		
		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		int nGrowBy = m_nGrowBy;
		if (nGrowBy == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = min(1024, max(4, m_nSize / 8));
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush
		
		void** pNewData = (void**) new BYTE[nNewMax * sizeof(void*)];
		
		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * sizeof(void*));
		
		// construct remaining elements
		
		memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));
		
		
		// get rid of old stuff (note: no destructors called)
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

int ptr_array::Append(const ptr_array& src)
{
	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);
	
	memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(void*));
	
	return nOldSize;
}

void ptr_array::Copy(const ptr_array& src)
{
	SetSize(src.m_nSize);
	
	memcpy(m_pData, src.m_pData, src.m_nSize * sizeof(void*));
	
}

void ptr_array::FreeExtra()
{
	
	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
		void** pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (void**) new BYTE[m_nSize * sizeof(void*)];
			// copy new data from old
			memcpy(pNewData, m_pData, m_nSize * sizeof(void*));
		}
		
		// get rid of old stuff (note: no destructors called)
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

/////////////////////////////////////////////////////////////////////////////

void ptr_array::SetAtGrow(int nIndex, void* newElement)
{
	if (nIndex >= m_nSize)
		SetSize(nIndex+1);
	m_pData[nIndex] = newElement;
}





void ptr_array::InsertAt(int nIndex, void* newElement, int nCount)
{
	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		SetSize(nIndex + nCount);  // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount);  // grow it to new size
		// shift old data up to fill gap
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(void*));
		
		// re-init slots we copied from
		
		memset(&m_pData[nIndex], 0, nCount * sizeof(void*));
		
	}
	
	// insert new value in the gap
	
	
	
	// copy elements into the empty space
	while (nCount--)
		m_pData[nIndex++] = newElement;
	
}



void ptr_array::RemoveAt(int nIndex, int nCount)
{
	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);
	
	if (nMoveCount)
		memmove(&m_pData[nIndex], &m_pData[nIndex + nCount],
		nMoveCount * sizeof(void*));
	m_nSize -= nCount;
}

void ptr_array::InsertAt(int nStartIndex, ptr_array* pNewArray)
{
	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}

void ptr_array::RemoveTail()
{
	if(m_nSize<=0)
		return ;
	RemoveAt(m_nSize-1);
}

void* ptr_array::RemoveHead()
{
	if(m_nSize>0)
	{
		LPVOID p=GetAt(0);
		RemoveAt(0);
		return p;
	}
	return NULL;
}


} // sp_ext
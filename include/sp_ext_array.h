/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_array.h 
* 摘	 要:    MFC抽取出来的指针数组
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:		2009年3月28日
*/
#ifndef __SP_EXT_ARRAY_H__
#define __SP_EXT_ARRAY_H__

namespace sp_ext
{

class ptr_array 
{
public:
	// Construction
	ptr_array();
	
	// Attributes
	int GetSize() const
	{ return m_nSize; }

	int GetUpperBound() const
	{ return m_nSize-1; }

	void SetSize(int nNewSize, int nGrowBy = -1);
	
	// Operations
	// Clean up
	void FreeExtra();

	void RemoveAll()
	{ SetSize(0); }
	
	// Accessing elements
	void* GetAt(int nIndex) const
	{ return m_pData[nIndex]; }

	void SetAt(int nIndex, void* newElement)
	{ m_pData[nIndex] = newElement; }
	
	void*& ElementAt(int nIndex)
	{return m_pData[nIndex]; }
	
	// Direct Access to the element data (may return NULL)
	const void** GetData() const
	{ return (const void**)m_pData; }

	void** GetData()
	{ return (void**)m_pData; }
	
	// Potentially growing the array
	void SetAtGrow(int nIndex, void* newElement);
	
	int Add(void* newElement)
	{ int nIndex = m_nSize;
	SetAtGrow(nIndex, newElement);
	return nIndex; }

	int Append(const ptr_array& src);
	void Copy(const ptr_array& src);
	
	// overloaded operator helpers
	void* operator[](int nIndex) const
	{ return GetAt(nIndex); }

	void*& operator[](int nIndex)
	{ return ElementAt(nIndex); }
	
	// Operations that move elements around
	void InsertAt(int nIndex, void* newElement, int nCount = 1);
	
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, ptr_array* pNewArray);
	
	// Implementation
protected:
	void** m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount
	
	
public:
	void* RemoveHead();
	void RemoveTail();
	~ptr_array();
	
protected:
	// local typedefs for class templates
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};

} //sp_ext 

#endif // __SP_EXT_ARRAY_H__

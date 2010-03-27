/** Copyright (c) 2008-2010
* All rights reserved.
* 
* �ļ�����:	sp_ext_boyermor.h 
* ժ	 Ҫ:    boyermor �ַ������ٲ����㷨��װ
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:		2009��3��28��
*/

#ifndef __SP_EXT_BOYERMOR_H__
#define __SP_EXT_BOYERMOR_H__

#include <windows.h>

//=============================================================================
//+ Name BoyerMoore -- Class for executing Boyer-Moore string search algorithm
//+ Description
//. <var>BoyerMoore</var> implements the Boyer-Moore algorithm for searching
//. for a string.  The Boyer-Moore algorithm is fast, but requires a bit
//. of start-up overhead compared to a brute force algorithm.
//=============================================================================
// Last modified 1998-04-28
//+ Noentry ~BoyerMoore

/***
	���ٲ����㷨�ඨ�壬�÷����£�
	CString	strToFind	=	"Ҫ���ҵ��ַ���";
	CString	strSrc		=	"Դ�ַ���";
	BoyerMoore moore( strToFind );
	LONG value = moore.GetNumberAfter( strSrc, strSrc.GetLength(), 0 );
	LONG date  = moore.GetDateAfter( strSrc, strSrc.GetLength(), 0 );
*/
namespace sp_ext
{ 

class BoyerMoore 
{
public:
    //. Constructs a <var>BoyerMoore</var> object for searching for
    //. a particular string.
    BoyerMoore(const char* to_find_str);
    

    virtual ~BoyerMoore();

	//. Sets the string to search for.
    void Assign(const char* to_find_str);
    

	//. Searches for the search string in <var>string</var> starting at position
    //. <var>startpos</var>.  If found, the function returns the first position in
    //. <var>string</var> where the search string was found.  If not found, the
    //. function returns <var>CSPString::npos</var>.
    size_t FindIn(const char *string, size_t length, size_t startpos);
   

    /* ��ȡҪ���ҵ��ַ������������ */
	double	GetNumberAfter( const char *string, size_t length, size_t startpos );
	
	/* 
	   ��ȡҪ���ҵ��ַ�����������ڸ�ʽ2010-03-24����10-3-24��
	   ���صĸ�ʽ20100324
	*/
	DWORD	GetDateAfter( const char *string, size_t length, size_t startpos );
	
	/* 
	   ��ȡҪ���ҵ��ַ�������Ķ��䣬������ \r ���� \n
	*/
	int		GetPhraseAfter( const char *string, size_t length, size_t startpos, char * rstring, size_t rmaxlen );

private:
	int		GetNumStringAfter( const char *string, size_t length, size_t startpos,
								  char * rstring, size_t rmaxlen );
    size_t mPatLen;
    char* mPat;
    unsigned char mSkipAmt[256];

    void _Assign(const char* aPat, size_t aPatLen);
};

}// sp_ext
#endif// __SP_EXT_BOYERMOR_H__

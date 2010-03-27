/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_boyermor.h 
* 摘	 要:    boyermor 字符串快速查找算法封装
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:		2009年3月28日
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
	快速查找算法类定义，用法如下：
	CString	strToFind	=	"要查找的字符串";
	CString	strSrc		=	"源字符串";
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
   

    /* 获取要查找的字符串后面的数字 */
	double	GetNumberAfter( const char *string, size_t length, size_t startpos );
	
	/* 
	   获取要查找的字符串后面的日期格式2010-03-24或者10-3-24，
	   返回的格式20100324
	*/
	DWORD	GetDateAfter( const char *string, size_t length, size_t startpos );
	
	/* 
	   获取要查找的字符串后面的段落，结束符 \r 或者 \n
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

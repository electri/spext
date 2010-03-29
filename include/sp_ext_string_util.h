/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_string_util.h
* 摘	 要:    字符串相关有用的函数
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:    2009年3月28日
*/
#ifndef __SP_EXT_STRING_UTIL_H__
#define __SP_EXT_STRING_UTIL_H__

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <functional>
#include <stdarg.h>
#include <fstream>
#include <Windows.h>

namespace sp_ext
{

class StrUtil
{
public:
	typedef std::vector<std::string> StringVector;
public:
    
	/** Removes any whitespace characters, be it standard space or
	TABs and so on.
	@remarks
	The user may specify wether they want to trim only the
	beginning or the end of the std::string ( the default action is
	to trim both).
	*/
    static void Trim( std::string& str, bool left = true, bool right = true )
    {
        static const std::string delims = " \t\r";
        if(right)
            str.erase(str.find_last_not_of(delims)+1);
        if(left)
            str.erase(0, str.find_first_not_of(delims));
    }
    
    ////  去掉所有空格
    static void TrimAllSpace(std::string &str)
    {
        std::string::iterator destEnd=std::remove_if(str.begin(),str.end(),isspace);
        str.resize(destEnd-str.begin());
    }
    
    /// 去除所有特定字符
    static void TrimAll(std::string &str, char ch)
    {
        std::string::iterator destEnd=std::remove_if(str.begin(),str.end(),std::bind1st(std::equal_to<char>(),ch));
        str.resize(destEnd-str.begin());
    }
    
    /** Returns a std::stringVector that contains all the sub std::strings delimited
    by the characters in the passed <code>delims</code> argument.
    @param 
    delims A list of delimiter characters to split by
    @param 
    maxSplits The maximum number of splits to perform (0 for unlimited splits). If this
    parameters is > 0, the splitting process will stop after this many splits, left to right.
    */
    static StringVector Split( const std::string& str, const std::string& delims = "\t\n ", unsigned int maxSplits = 0)
    {
        StringVector ret;
        unsigned int numSplits = 0;
        
        // Use STL methods 
        size_t start, pos;
        start = 0;
        do 
        {
            pos = str.find_first_of(delims, start);
            if (pos == start)
            {
                // Do nothing
                start = pos + 1;
            }
            else if (pos == std::string::npos || (maxSplits && numSplits == maxSplits))
            {
                // Copy the rest of the std::string
                ret.push_back( str.substr(start) );
                break;
            }
            else
            {
                // Copy up to delimiter
                ret.push_back( str.substr(start, pos - start) );
                start = pos + 1;
            }
            // parse up to next real data
            start = str.find_first_not_of(delims, start);
            ++numSplits;
            
        } while (pos != std::string::npos);
        return ret;
    }
    
    /** Upper-cases all the characters in the std::string.
    */
    static void ToLowerCase( std::string& str )
    {
        std::transform(
            str.begin(),
            str.end(),
            str.begin(),
            tolower);
    }
    
    /** Lower-cases all the characters in the std::string.
    */
    static void ToUpperCase( std::string& str )
    {
        std::transform(
            str.begin(),
            str.end(),
            str.begin(),
            toupper);
    }
    
    /** Converts the contents of the std::string to a float.
    @remarks
    Assumes the only contents of the std::string are a valid parsable float. Defaults to  a
    value of 0.0 if conversion is not possible.
    */
    static float ToFloat( const std::string& str )
    {
        return (float)atof(str.c_str());
    }
    
    static double ToDouble( const std::string& str )
    {
        return atof(str.c_str());
    }
    
    /** Returns whether the std::string begins with the pattern passed in.
    @param pattern The pattern to compare with.
    @param lowerCase If true, the end of the std::string will be lower cased before 
    comparison, pattern should also be in lower case.
    */
    static bool StartsWith(const std::string& str, const std::string& pattern, bool lowerCase = true)
    {
        size_t thisLen = str.length();
        size_t patternLen = pattern.length();
        if (thisLen < patternLen || patternLen == 0)
            return false;
        
        std::string startOfThis = str.substr(0, patternLen);
        if (lowerCase)
            ToLowerCase(startOfThis);
        
        return (startOfThis == pattern);
    }
    
    /** Returns whether the std::string ends with the pattern passed in.
    @param pattern The pattern to compare with.
    @param lowerCase If true, the end of the std::string will be lower cased before 
    comparison, pattern should also be in lower case.
    */
    static bool EndsWith(const std::string& str, const std::string& pattern, bool lowerCase = true)
    {
        size_t thisLen = str.length();
        size_t patternLen = pattern.length();
        if (thisLen < patternLen || patternLen == 0)
            return false;
        
        std::string endOfThis = str.substr(thisLen - patternLen, patternLen);
        if (lowerCase)
            ToLowerCase(endOfThis);
        
        return (endOfThis == pattern);
    }
    
    /** Method for standardising paths - use forward slashes only, end with slash. 
    */
    static std::string StandardisePath( const std::string &init)
    {
        std::string path = init;
        
        std::replace( path.begin(), path.end(), '\\', '/' );
        if( path[path.length() - 1] != '/' )
            path += '/';
        
        return path;
    }
    
    /** Method for splitting a fully qualified filename into the base name 
    and path. 
    @remarks
    Path is standardised as in standardisePath
    */
    static void SplitFilename(const std::string& qualifiedName,std::string& outBasename, std::string& outPath)
    {
        std::string path = qualifiedName;
        // Replace \ with / first
        std::replace( path.begin(), path.end(), '\\', '/' );
        // split based on final /
        size_t i = path.find_last_of('/');
        
        if (i == std::string::npos)
        {
            outPath = "";
            outBasename = qualifiedName;
        }
        else
        {
            outBasename = path.substr(i+1, path.size() - i - 1);
            outPath = path.substr(0, i+1);
        }
    }
    
    /** Simple pattern-matching routine allowing a wildcard pattern. 
    @param str std::string to test
    @param pattern Pattern to match against; can include simple '*' wildcards
    @param caseSensitive Whether the match is case sensitive or not
    */
    static bool Match(const std::string& str, const std::string& pattern, bool caseSensitive = true)
    {
        std::string tmpStr = str;
        std::string tmpPattern = pattern;
        if (!caseSensitive)
        {
            ToLowerCase(tmpStr);
            ToLowerCase(tmpPattern);
        }
        
        std::string::const_iterator strIt = tmpStr.begin();
        std::string::const_iterator patIt = tmpPattern.begin();
        std::string::const_iterator lastWildCardIt = tmpPattern.end();
        while (strIt != tmpStr.end() && patIt != tmpPattern.end())
        {
            if (*patIt == '*')
            {
                lastWildCardIt = patIt;
                // Skip over looking for next character
                ++patIt;
                if (patIt == tmpPattern.end())
                {
                    // Skip right to the end since * matches the entire rest of the string
                    strIt = tmpStr.end();
                }
                else
                {
                    // scan until we find next pattern character
                    while(strIt != tmpStr.end() && *strIt != *patIt)
                        ++strIt;
                }
            }
            else
            {
                if (*patIt != *strIt)
                {
                    if (lastWildCardIt != tmpPattern.end())
                    {
                        // The last wildcard can match this incorrect sequence
                        // rewind pattern to wildcard and keep searching
                        patIt = lastWildCardIt;
                        lastWildCardIt = tmpPattern.end();
                    }
                    else
                    {
                        // no wildwards left
                        return false;
                    }
                }
                else
                {
                    ++patIt;
                    ++strIt;
                }
            }
            
        }
        // If we reached the end of both the pattern and the string, we succeeded
        if (patIt == tmpPattern.end() && strIt == tmpStr.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    /// Constant blank std::string, useful for returning by ref where local does not exist
    static const std::string BLANK()
    {
        static const std::string temp = std::string("");
        return temp;
    }
    
    /// 地球人都知道，恶心的std::string是没有CString的Format这个函数的，所以我们自己造
    static std::string Printf(const char *pszFormat, ...)
    {
        va_list argptr;
        va_start(argptr, pszFormat);
        std::string result=Printf(pszFormat,argptr);
        va_end(argptr);
        return result;
    }
    
    /// 地球人都知道，恶心的std::string是没有CString的Format这个函数的，所以我们自己造
    static std::string Printf2(const char *pszFormat, ...)
    {
        va_list argptr;
        va_start(argptr, pszFormat);
        std::string result=Printf2(pszFormat,argptr);
        va_end(argptr);
        return result;
    }
    
    /// 地球人都知道，恶心的std::string是没有CString的Format这个函数的，所以我们自己造
    static std::string Printf2(const char *pszFormat,va_list argptr)
    {
        int         size   = 1024;
        char*       buffer = new char[size];
        
        while (1) 
        {
            int n = _vsnprintf(buffer, size, pszFormat, argptr);
            
            // If that worked, return a string.
            if (n > -1 && n < size) 
            {
                std::string s(buffer);
                delete [] buffer;
                return s;
            }
            
            if (n > -1)     size  = n+1; // ISO/IEC 9899:1999
            else            size *= 2;   // twice the old size
            
            delete [] buffer;
            buffer = new char[size];
        }
    }
    
    /// 地球人都知道，恶心的std::string是没有CString的Format这个函数的，所以我们自己造
    static std::string Printf(const char* pszFormat, va_list argptr)
    {
        int size = 1024;
        int len=0;
        std::string ret;
        for ( ;; )
        {
            ret.resize(size + 1,0);
            char *buf=(char *)ret.c_str();   
            if ( !buf )
            {
                return BLANK();
            }
            
            va_list argptrcopy;
            VaCopy(argptrcopy, argptr);
            
            len = _vsnprintf(buf, size, pszFormat, argptrcopy);
            va_end(argptrcopy);
            
            if ( len >= 0 && len <= size )
            {
                // ok, there was enough space
                break;
            }
            size *= 2;
        }
        ret.resize(len);
        return ret;
    }
    
    /// 取得右边的N个字符
    static std::string Right(const std::string &src,size_t nCount)
    {
        if(nCount>src.length())
            return BLANK();
        return src.substr(src.length()-nCount,nCount);
    }
    
    /// 取左边的N个字符
    static std::string left(const std::string &src,size_t nCount)
    {
        return src.substr(0,nCount);
    }
    
    size_t CharCount(const std::string &src,char ch)
    {
        size_t result=0;
        for(size_t i=0;i<src.length();i++)
        {
            if(src[i]==ch)result++;
        }
        return result;
    }

    static std::string Replace(std::string &str, const char* string_to_replace, const char* new_string)
    {
        //   Find   the   first   string   to   replace   
        int index = str.find(string_to_replace);   
        //   while   there   is   one   
        while( index != std::string::npos )   
        {   
            //   Replace   it   
            str.replace(index, strlen(string_to_replace), new_string);   
            //   Find   the   next   one   
            index = str.find(string_to_replace, index+strlen(new_string));   
        }   
        return   str;   
    }

    /// 移除字符串中的html标签
    static bool RemoveHtml(std::string strSrcStr, std::string& strTargetStr)
    {
        std::string s = strSrcStr;
        bool inTag = false;
        bool done = false;
        while(!done) 
        {
            if(inTag)
            {
                // The previous line started an HTML tag
                // but didn't finish. Must search for '>'.
                size_t rightPos = s.find('>');
                if(rightPos != std::string::npos) 
                {
                    inTag = false;
                    s.erase(0, rightPos + 1);
                }
                else 
                {
                    done = true;
                    s.erase();
                }
            }
            else 
            {
                // Look for start of tag:
                size_t leftPos = s.find('<');
                if(leftPos != std::string::npos) 
                {
                    // See if tag close is in this line:
                    size_t rightPos = s.find('>');
                    if(rightPos == std::string::npos) 
                    {
                        inTag = done = true;
                        s.erase(leftPos);
                    }
                    else
                        s.erase(leftPos, rightPos - leftPos + 1);
                }
                else
                    done = true;
            }
        }
        // Remove all special HTML characters
        Replace(s, "&lt;", "<");
        Replace(s, "&gt;", ">");
        Replace(s, "&amp;", "&");
        Replace(s, "&nbsp;", " ");
        // Etc...
        //return s;
		strTargetStr = s;
		return true;
    }



    //std::string=>std::wstring
    static std::wstring String2WString(const std::string& strValue)
    {
        int size = (int)strValue.size() * 2 + 2;
        wchar_t* wStr = new wchar_t[size];
        MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), -1, wStr, size);
        std::wstring Result = wStr;
        delete []  wStr;
        return Result;
    }

    static std::string WChar2string(LPCWSTR pwszSrc)   
    {   
        int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);   
        if (nLen<= 0) return std::string("");   
        char* pszDst = new char[nLen];   
        if (NULL == pszDst) return std::string("");   
        WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);   
        pszDst[nLen -1] = 0;   
        std::string strTemp(pszDst);   
        delete [] pszDst;   
        return strTemp;   
    }   

    static std::string WString2String(const std::wstring& inputws)
    { 
       return WChar2string(inputws.c_str()); 
    }

    static std::string GetStrFromFile(const std::string& strFileName)
    {
        std::ifstream   tfile( String2WString(strFileName).c_str(),std::ios::binary );
        std::string str;

        char   currch;   
        str.erase();   
        while( !tfile.eof() )   
        {       
            tfile.get(currch);   
            if (tfile.bad())//there   is   an   unrecoverable   error.   
                return   "";   

            str += currch;   
        }   
        return   str;  
    }


    //strTargetFile的目录必须存在，否则不会保存成功
    static bool SaveStr2File(const std::string& strSrc, const std::string& strTargetFile) 
    {
        std::ofstream outfile(String2WString(strTargetFile).c_str(), std::ios::binary);
        if( !outfile ) 
        { 
            return false; 
        } 

        try
        {
            int len1 = strSrc.length(); 
            outfile.write((char*)strSrc.c_str(), len1); 
            outfile.close();
            return true;
        }
        catch (...)
        {
            return false;
        }
    }


    /// 删除前后空格
    static std::string& SpaceRrim(std::string &s)
    {
        if (s.empty()) 
        {
            return s;
        }

        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
        return s;
    }

    /// 删除字符串中指定字符
    static std::string& RemoveChars(std::string &s, const std::string strChars = "-,.\'\"")
    {
        std::string::size_type pos = 0;
        while ((pos = s.find_first_of(strChars, pos)) != std::string::npos)
        {
            s.erase(pos, 1);
        }
        return s;
    }
    
protected:
 static inline void VaCopy(va_list &dest,va_list &src)
 {
     dest=src;
 }
};


class StrConvert
{
public:
 /** Converts a float to a String. */
 static std::string ToString(float val, unsigned short precision = 6, 
     unsigned short width = 0, char fill = ' ', 
     std::ios::fmtflags flags = std::ios::fmtflags(0) )
 {
     std::stringstream stream;
     stream.precision(precision);
     stream.width(width);
     stream.fill(fill);
     if (flags)
         stream.setf(flags);
     stream << val;
     return stream.str();
 }
 
 /** Converts a double to a String. */
 static std::string ToString(double val, unsigned short precision = 6, 
     unsigned short width = 0, char fill = ' ', 
     std::ios::fmtflags flags = std::ios::fmtflags(0) )
 {
     std::stringstream stream;
     stream.precision(precision);
     stream.width(width);
     stream.fill(fill);
     if (flags)
         stream.setf(flags);
     stream << val;
     return stream.str();
 }
 
 /** Converts an int to a String. */
 static std::string ToString(int val, unsigned short width = 0, 
     char fill = ' ', 
     std::ios::fmtflags flags = std::ios::fmtflags(0) )
 {
     std::stringstream stream;
     stream.width(width);
     stream.fill(fill);
     if (flags)
         stream.setf(flags);
     stream << val;
     return stream.str();
 }
 
 /** Converts a size_t to a String. */
 static std::string ToString(size_t val, 
     unsigned short width = 0, char fill = ' ', 
     std::ios::fmtflags flags = std::ios::fmtflags(0) )
 {
     std::stringstream stream;
     stream.width(width);
     stream.fill(fill);
     if (flags)
         stream.setf(flags);
     stream << val;
     return stream.str();
 }
 
 /** Converts an unsigned long to a String. */
 static std::string ToString(unsigned long val, 
     unsigned short width = 0, char fill = ' ', 
     std::ios::fmtflags flags = std::ios::fmtflags(0) )
 {
     std::stringstream stream;
     stream.width(width);
     stream.fill(fill);
     if (flags)
         stream.setf(flags);
     stream << val;
     return stream.str();
 }
 
 /** Converts a long to a String. */
 static std::string ToString(long val, 
     unsigned short width = 0, char fill = ' ', 
     std::ios::fmtflags flags = std::ios::fmtflags(0) )
 {
     std::stringstream stream;
     stream.width(width);
     stream.fill(fill);
     if (flags)
         stream.setf(flags);
     stream << val;
     return stream.str();
 }
 /** Converts a boolean to a String. 
 @param yesNo If set to true, result is 'yes' or 'no' instead of 'true' or 'false'
 */
 static std::string ToString(bool val, bool yesNo = false)
 {
     if (val)
     {
         if (yesNo)
         {
             return "yes";
         }
         else
         {
             return "true";
         }
     }
     else
     {
         if (yesNo)
         {
             return "no";
         }
         else
         {
             return "false";
         }
     }
 }
 
 /** Converts a StringVector to a string.
 @remarks
 Strings must not contain spaces since space is used as a delimeter in
 the output.
 */
 static std::string ToString(const StrUtil::StringVector& val)
 {
     std::stringstream stream;
     StrUtil::StringVector::const_iterator i, iend, ibegin;
     ibegin = val.begin();
     iend = val.end();
     for (i = ibegin; i != iend; ++i)
     {
         if (i != ibegin)
             stream << " ";
         
         stream << *i; 
     }
     return stream.str();
 }
 
 /** Converts a std::string to a float. 
 @returns
 0.0 if the value could not be parsed, otherwise the float version of the String.
 */
 static float Parsefloat(const std::string& val)
 {
     return (float)atof(val.c_str());
 }
 
 /** Converts a std::string to a double. 
 @returns
 0.0 if the value could not be parsed, otherwise the float version of the String.
 */
 static double ParseDouble(const std::string& val)
 {
     return atof(val.c_str());
 }
 
 /** Converts a std::string to a whole number. 
 @returns
 0.0 if the value could not be parsed, otherwise the numeric version of the String.
 */
 static int ParseInt(const std::string& val)
 {
     return atoi(val.c_str());
 }
 
 /** Converts a std::string to a whole number. 
 @returns
 0.0 if the value could not be parsed, otherwise the numeric version of the String.
 */
 static unsigned int ParseUnsignedInt(const std::string& val)
 {
     return static_cast<unsigned int>(strtoul(val.c_str(), 0, 10));
 }
 
 /** Converts a std::string to a whole number. 
 @returns
 0.0 if the value could not be parsed, otherwise the numeric version of the String.
 */
 static long ParseLong(const std::string& val)
 {
     return strtol(val.c_str(), 0, 10);
 }
 
 /** Converts a std::string to a whole number. 
 @returns
 0.0 if the value could not be parsed, otherwise the numeric version of the String.
 */
 static unsigned long ParseUnsignedLong(const std::string& val)
 {
     return strtoul(val.c_str(), 0, 10);
 }
 
 /** Converts a std::string to a boolean. 
 @remarks
 Accepts 'true' or 'false' as input.
 */
 static bool ParseBool(const std::string& val)
 {
     return (val == "true" || val == "yes");
 }
 
 
 /** Pareses a StringVector from a string.
 @remarks
 Strings must not contain spaces since space is used as a delimeter in
 the output.
 */
 static StrUtil::StringVector ParseStringVector(const std::string& val)
 {
     return StrUtil::Split(val);
 }
     
 };

 }// sp_ext


#endif // __SP_EXT_STRING_UTIL_H__

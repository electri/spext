/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:		api_tools.h  
* 摘	 要:	一些非常有用的工具函数
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:		2009年3月24日
*/

#ifndef __SP_EXT_API_TOOLS_H__
#define __SP_EXT_API_TOOLS_H__

#include <string>
#include <windows.h>
#include "sp_ext_define.h"

namespace sp_ext
{ 

/**
 *	取该模块所在路径
 */
std::string	get_module_path(HMODULE h);

/**
 *	取exe主程序所在路径, 结尾包含 \
 */
std::string	get_app_path();

/**
 *	取SYSTEM32目录, 结尾不包含 \
 */
std::string get_sys_path();

/**
 *	取windows目录, 结尾不包含 \
 */
std::string get_windows_path();

/**
 *	取临时文件目录, 结尾包含 \, 例如："C:\DOCUME~1\wy\LOCALS~1\Temp\"
 */
std::string get_sys_temp_path();



/**
 *	保证buf[buf_size - 1] 总是为'\0'
 */
int safe_snprintf(char* buf, int buf_size, const char* fmt, ...);

/** 
 *	if( strlen(src) >= n) , 将拷贝n个字符， 并且dest[n] = 0, 所以应该: n < dest的缓冲区长度
 *	 返回实际拷贝的有效字符数（不含'\0')
 */
size_t safe_strncpy(char* dest, const char* src, size_t n);

/// 判断目录是否存在
bool dir_exist(const char *path);

/// 创建目录, @path 末尾加'\\'
bool dir_create(const char * path);

/// 删除目录(包括子目录及文件)
bool dir_delete(const char* dir);

bool del_all_files(const char* dir, bool enter_sub_dir = true);

/// 彻底删除文件包括可读文件
bool delete_file_ex(const char* file_name);

/// 获取文件大小 (字节)
long get_file_size(const char* file_name); 

/// 查找文件、目录是否存在  目录 － 分区名:/目录名, 文件 － 分区名:/目录名/文件名
bool find_file(const char* file_name);

enum e_win_ver
{
	e_win_ver_95 = 0,
	e_win_ver_98,
	e_win_ver_MELLINNIUM,
	e_win_ver_NT,
	e_win_ver_2000,
	e_win_ver_XP,
	e_win_ver_SERVER2003,
	e_win_ver_VISTA,
	e_win_ver_7,
};
//判断当前用户使用的操作系统类型
e_win_ver get_windows_ver();

/*	查找str中最后一个 ==key 的字符, 返回该位置的指针, 否则返回NULL
 *	如果指定了len, 则从str[len-1]开始反向查找, 效率可能会更高
 */
char* str_find_lastof(char* str, char key, int len = -1);
/// 同上
const char* str_find_lastof(const char* str, char key, int len = -1);

/**	反向查找str, 如果key中的任意一个字符在str中出现, 则返回该位置的指针, 否则返回NULL
 *	如果指定了len, 则从str[len-1]开始反向查找, 效率可能会更高
 */
char* str_find_lastof(char* str, const char* key, int len = -1);
/// 同上	
const char* str_find_lastof(const char* str, const char* key, int len = -1);

std::string to_str(int v);
std::string to_str(unsigned int v);
std::string to_str(long v);
std::string to_str(unsigned long v);
std::string to_str(float v);
std::string to_str(double v);

char* to_str(char* buf, int v);
char* to_str(char* buf, unsigned int v);
char* to_str(char* buf, long v);
char* to_str(char* buf, unsigned long v);
char* to_str(char* buf, float v);
char* to_str(char* buf, double v);

bool str_format(std::string& s, const char* fmt, ...);

void str_trim_all(std::string& s, const char* k = "\t\n\f\v\b\r ");
void str_trim_left(std::string& s, const char* k = "\t\n\f\v\b\r ");
void str_trim_right(std::string& s, const char* k = "\t\n\f\v\b\r ");

inline char* safe_itoa(int number, char *buf, int buf_size, int radix=10)
{
	/*
	assert( radix>=2 && radix <= 36 );
	assert( buf );
	assert( buf_size );
	*/
	static const char C[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char s[50];
	int i=0;
	do
	{
		s[i++] = C[ number % radix ];
		number /= radix;
	}
	while( number );
	if( buf_size > i )
	{
		int j=0;
		while( i-- )
		{
			buf[j++] = s[i];
		}
		buf[j]=0;
		return buf;
	}
	return NULL;
}

inline const char* find_char(const char *s, const char c)
{
	while( (*s) && (*s)!=c )
	{
		s++;
	}
	return (*s)? s:NULL;
}
inline const char* find_char(const char* s, const char *end, char c)
{
	while( s<end && (*s)!=c )
	{
		s++;
	}
	return (s<end)? s:NULL;
}
inline const char* skip_char(const char* s, const char *end, const char* match)
{
	while( s<end && find_char(match,*s) )
	{
		s++;
	}
	return s;
}
inline const char* rskip_char(const char* s, const char *front, const char* match)
{
	while( s>front && find_char(match,*s) )
	{
		s--;
	}
	return s;
}

inline uint32_t string_hash(const char* s)
{
	uint32_t hash = 5381;
	for( int c; c=(*s++); )
	{
		hash = ( (hash<<5) + hash ) + c;
	}
	return hash;
}

/**
 *  获取当前时间
 *	nType == 1: YYYYMMDDhhmmss
 *	nType == 2: YYYY-MM-DD hh:mm:ss
 *	nType == 3: YYYYMMDD
 */
std::string get_cur_time(int nType);

/// 创建一个GUID
bool create_guid(std::string& new_guid);

/// 获取随机串
void get_random_string(char* str, const unsigned int len);

////////////////////////////////////////////////////////////////////
//	random	"生成随机数/随机串"
//	seed()	"更新随机因子"
////////////////////////////////////////////////////////////////////
struct random
{
	static int number(void);
	static const char* string(char* buf, int len);
	static void seed(int n);
};

////////////////////////////////////////////////////////////////////
//		String "字符串类"
////////////////////////////////////////////////////////////////////
class String
{
	char* m_buf;
	size_t m_capacity;
	size_t m_length;
	union {
		int zero_;
		char empty_[1];
	};

	bool grow_capacity(size_t need, bool copy)
	{
		size_t capacity = ( need*3/2 +63 ) & (~63);
		char *buf = new char[capacity];
		if( buf==NULL )
			return false;
		//		DPRINT("String::grow() new[%u]\n", capacity);
		if( m_capacity )
		{
			if( copy && m_length )
			{
				memcpy(buf, this->m_buf, this->m_length);
			}
			delete[] this->m_buf;
			//			DPRINT("String::grow() delete[]\n");
		}
		m_capacity = capacity;
		m_buf = buf;
		return true;
	}
public:
	String(void): m_buf(empty_), m_capacity(0), m_length(0), zero_(0)
	{
	}
	String(const char* begin, const char* end): m_buf(empty_), m_capacity(0), m_length(0), zero_(0)
	{
		this->assign(begin, end);
	}
	String(const char* s, size_t len): m_buf(empty_), m_capacity(0), m_length(0), zero_(0)
	{
		this->assign(s, len);
	}
	String(const char* s): m_buf(empty_), m_capacity(0), m_length(0), zero_(0)
	{
		this->assign(s);
	}
	String(const String& s): m_buf(empty_), m_capacity(0), m_length(0), zero_(0)
	{
		this->assign(s);
	}
	~String(void)
	{
		if( this->m_buf!=this->empty_ )
		{
			delete[] this->m_buf;
			//			DPRINT("~String() delete[]\n");
		}
	}
	String& operator=(const String& s)
	{
		return this->assign(s);
	}
	String& operator=(const char* s)
	{
		return this->assign(s);
	}
	String& operator=(const std::string& s)
	{
		return this->assign(s.c_str());
	}
	operator const char*(void)
	{
		return this? m_buf : "";
	}
	String& operator +=(const char* s)
	{
		return this->append(s);
	}
	String& operator +=(const String& s)
	{
		return this->append(s);
	}
	bool operator==(const char* s) const
	{
		return this->compare(s)==0;
	}
	bool operator==(const String& s) const
	{
		return this->compare(s.m_buf)==0;
	}
	bool operator<(const String& s) const
	{
		return this->compare(s.c_str()) < 0;
	}
	bool operator!=(const char* s) const
	{
		return this->compare(s)!=0;
	}
	bool operator!=(const String& s) const
	{
		return this->compare(s.m_buf)!=0;
	}
	int compare(const char* s) const
	{
		return strcmp(this->m_buf, s);
	}
	size_t length(void) const
	{
		return m_length;
	}
	void clear(void)
	{
		m_length=0;
		m_buf[0]=0;
	}
	bool empty(void) const
	{
		return m_length==0;
	}
	const char* c_str(void) const
	{
		return this? m_buf : "";
	}
	const char operator[](int index) const
	{
		return m_buf[index];
	}
	String& assign(const char* begin, const char* end)
	{
		if( end==NULL )
		{
			return this->assign(begin);
		}
		size_t len = end>begin? end-begin : 0;
		return this->assign( begin, len );
	}
	String& assign(const char* s, size_t len)
	{
		if( s==NULL || len==0 )
		{
			this->clear();
			return *this;
		}
		if( len >= m_capacity )
		{
			if( !grow_capacity(len+1,false) )
			{
				return *this;
			}
		}
		memcpy(m_buf, s, len);
		m_buf[len] = 0;
		m_length = len;
		return *this;
	}
	String& assign(const char* s)
	{
		size_t len = (s && s[0])? strlen(s) : 0;
		return this->assign(s, len);
	}
	String& assign(const String& s)
	{
		return this->assign( s.m_buf, s.m_length );
	}
	String& append(const char* begin, const char* end)
	{
		size_t len = end>begin? end-begin : 0;
		return this->append( begin, len );
	}
	String& append(const char* s, size_t len)
	{
		if( s==NULL || len==0 )
		{
			return *this;
		}
		if( m_length+len >= m_capacity )
		{
			if( !grow_capacity(m_length+len+1,true) )
			{
				return *this;
			}
		}
		memcpy(m_buf+m_length, s, len);
		m_length +=len;
		m_buf[m_length]=0;
		return *this;
	}
	String& append(const char* s)
	{
		size_t len = (s && s[0])? strlen(s) : 0;
		return this->append( s, len );
	}
	String& append(const String& s)
	{
		return this->append( s.m_buf, s.m_length );
	}
	String& trim(const char* match=" \t")
	{
		char *first = m_buf;
		char *end = first+m_length;
		while( first<end && find_char(match,*first) )
		{
			first++;
		}
		char *last = end;
		while( last>first && find_char(match, *(last-1)) )
		{
			last--;
		}
		if( first==m_buf )
		{
			if( last!=end )
			{
				*last=0;
				m_length=last-first;
			}
		}
		else
		{
			m_length = last-first;
			memmove(m_buf, first, m_length);
			m_buf[m_length]=0;
		}
		return *this;
	}
};
template<class T>
inline String& operator<<(String& s, T& s2)
{
	s.append(s2);
	return s;
}



}// sp_ext






#endif // __SP_EXT_API_TOOLS_H__
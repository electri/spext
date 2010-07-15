/** Copyright (c) 2008-2010
 * All rights reserved.
 * 
 * 文件名称:	aot_inet_define.h   
 * 摘	 要:	封装网络通讯的操作接口
 * 
 * 当前版本：	1.0
 * 作	 者:	陈学术
 * 操	 作:	新建
 * 完成日期:	2010年03月07日
 */
#ifndef __SP_EXT_DEFINE_H__
#define __SP_EXT_DEFINE_H__

#include <string>
#include <Windows.h>
#include <time.h>
#include <WinSock.h>

namespace sp_ext{ 
#ifdef WIN32
typedef UINT8	uint8_t;
typedef UINT16	uint16_t;
typedef UINT32	uint32_t;
typedef UINT64	uint64_t;
#else
#include <ace\basic_types.h>
typedef ACE_UINT8	uint8_t;
typedef ACE_UINT16	uint16_t;
typedef ACE_UINT32	uint32_t;
typedef ACE_UINT64	uint64_t;
#endif



#define __SP_EXT_INT32_B0(n)           ((uint8_t)((uint32_t)(n) & 0xff))
#define __SP_EXT_INT32_B1(n)          ((uint8_t) (((uint32_t)(n) & 0xff00) >> 8))
#define __SP_EXT_INT32_B2(n)          ((uint8_t) (((uint32_t)(n) & 0xff0000) >> 16))
#define __SP_EXT_INT32_B3(n)          ((uint8_t) (((uint32_t)(n) & 0xff000000) >> 24))

enum 
{
	e_ret_ok = 0, 
	e_ret_failed = -1, 
	e_ret_again = -2,
};

struct inet_addr_t
{
	/// host byte order
	unsigned long ip;
	/// host byte order
	unsigned short port;
};

struct inet_time_t
{
	long sec;  /// 秒
	long usec; /// 微秒
};

#pragma warning(disable:4996)

/// ip, port : 主机字节序
inline 
char* inet_addr_to_str(unsigned long ip, unsigned short port, char* buf)
{
	ip = ::htonl(ip);
	unsigned char* p = (unsigned char*)(&ip);
	sprintf(buf, "%d.%d.%d.%d:%d",  // 192.168.1.134
		p[0], p[1], p[2], p[3],
		port);
	return buf;
}

inline 
std::string inet_addr_to_str(unsigned long ip, unsigned short port)
{
	ip = ::htonl(ip);
	unsigned char* p = (unsigned char*)(&ip);
	char buf[64];
	sprintf(buf, "%d.%d.%d.%d:%d", 
		p[0], p[1], p[2], p[3],
		port);
	return buf;
}

inline 
std::string inet_addr_to_str(unsigned long ip)
{
	ip = ::htonl(ip);
	unsigned char* p = (unsigned char*)(&ip);
	char buf[64];
	sprintf(buf, "%d.%d.%d.%d", 
		p[0], p[1], p[2], p[3]);
	return buf;
}

inline 
std::string inet_addr_to_str(const inet_addr_t* addr)
{
	return inet_addr_to_str(addr->ip, addr->port);
}

inline 
char* inet_addr_to_str(unsigned long ip, char* buf)
{
	ip = ::htonl(ip);
	unsigned char* p = (unsigned char*)(&ip);
	sprintf(buf, "%d.%d.%d.%d", 
		p[0], p[1], p[2], p[3]);
	return buf;
}

////////////////////////////////////////////////////////////////////
//		inet_time_t 辅助函数
////////////////////////////////////////////////////////////////////
inline bool operator<(const inet_time_t& t1, const inet_time_t& t2) 
{
	return t1.sec==t2.sec? t1.usec<t2.usec : t1.sec<t2.sec;
}
inline inet_time_t operator-(const inet_time_t& t1, const inet_time_t& t2)
{
	inet_time_t t;
	t.sec = t1.sec - t2.sec;
	t.usec = t1.usec - t2.usec;
	if( t.usec<0 )
	{
		t.usec +=1000000;
		t.sec--;
	}
	return t;
}
inline inet_time_t& operator+=(inet_time_t& t1, const inet_time_t& t2)
{
	t1.sec += t2.sec;
	t1.usec += t2.usec;
	if( t1.usec>=1000000 )
	{
		t1.usec-=1000000;
		t1.sec++;
	}
	return t1;
}

/// 获取当前时间
inline int get_timeofday(inet_time_t* t, void* p = NULL)
{
#if 0
	if( t==NULL )
		return -1;

	FILETIME  ft;
	GetSystemTimeAsFileTime (&ft);

	uint64_t tim = filetime_to_unix_epoch (&ft);
	t->sec  = (long) (tim / 1000000L);
	t->tv_usec = (long) (tim % 1000000L);
	return 0;
#else
	if( t )
	{
		t->sec = (long)time(NULL);
		t->usec = (GetTickCount()%1000) *1000;
	}
	return -1;
#endif
}

inline char* get_datetime(char* buf, const time_t& tv)
{
	struct tm t;
	if( localtime_s(&t, &tv)==0 )
	{
		sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	}
	else
	{
		strcpy(buf, "0000/0000/0000 00:00:00");
	}
	return buf;
}


#pragma warning(default:4996)

} /// end namespace sp_ext

#endif /// __SP_EXT_DEFINE_H__
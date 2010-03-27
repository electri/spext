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
#include <WinSock2.h>


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
	long sec;
	long usec;
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


#pragma warning(default:4996)

} /// end namespace sp_ext

#endif /// __SP_EXT_DEFINE_H__
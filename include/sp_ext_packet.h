/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_packet.h    
* 摘	 要:	封装网络通讯包的操作接口
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:	2010年03月07日
*/
#ifndef __SP_EXT_PACKET_H__
#define __SP_EXT_PACKET_H__

#include "sp_ext_define.h"
#include <spbuffer.hpp>


namespace sp_ext
{ 

#define AIO_DEFAULT_HEARTBEAT_TM	(60000)
#define MAX_HEARTBEAT_TM_OUT		(AIO_DEFAULT_HEARTBEAT_TM * 3 + 10)/* sec */
#define SP_EXT_IOV_MAX					(14)

typedef enum 
{ 
    PKT_VERSION = 2,
    PKT_FLAG = 0x1122, 
    PKT_HEADER_LEN = sizeof(uint32_t)*3,
    PKT_MAX_EXHEADER_LEN = 256,
    PKT_MAX_LEN = 1024*32,
    PKT_SEND_HIGH_WATER = PKT_MAX_LEN + 1024,
}PKT_INFO;

#define SP_EXT_BUF_PAGE_SIZE				(4096 - 16)
#define SP_EXT_RECVBUF_DEFAULT_SIZE		SP_EXT_BUF_PAGE_SIZE
#define SP_EXT_SENDBUF_DEFAULT_SIZE		SP_EXT_BUF_PAGE_SIZE
#define SP_EXT_RECVBUF_MIN_FREE_SIZE		(PKT_MAX_EXHEADER_LEN + PKT_HEADER_LEN)

typedef enum
{
    /// 
    e_pkt_type_heartbeat = 0,
    e_pkt_type_echo,
    e_pkt_type_echo_reply,
    e_pkt_type_data,
    __e_pkt_type_user_define_begin__ = 1001,
}epkt_type;



/**   通讯协议数据包头定义:
*  
*    00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |    version(uint8)     |  T1       |     T2    |           packet type(uint_16)                |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                                   body  length (uint32)                                       |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |          extern header  length (uint16)       |                    T3                         |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   
*
*   字段说明:
*   version(1字节):		版本号(当前版本为1)
*   packet type(1字节):	数据包类型:
*								0: 心跳包
*								1: echo请求包(测试用)
*								2: echo应答包(测试用)
*								3: 业务数据包
*   header length(1字节):	包头长度
*   T1( 4 bit):			加密方式 0: 不加密
*   T2( 4 bit):			保留,必须置为全0
*   body length(4字节):	包体长度
*   extern header length(2字节):	扩展包头长度
*   T3(2字节):			保留,必须置为全0
*/
struct packet_header
{
	packet_header( char* p)
		: buf_((uint32_t*)p)
	{
		;
	}

	bool is_valid() { return version() == PKT_VERSION; }

	uint8_t version(){ return *((uint8_t*)buf_); }
	void version(uint32_t v) { *((uint8_t*)buf_) = v; }

    uint16_t type(){ return ntohs( ((uint16_t*)buf_)[1] ); }
    void type(uint16_t v) { ((uint16_t*)buf_)[1] = htons(v); }
   
    uint8_t encrypt_type(){ return ((uint8_t*)buf_)[1] & 0x0f;}
    void encrypt_type(uint8_t v) { ((uint8_t*)buf_)[1] = (v & 0x0f);} // 将低序字节存储在起始地址

    uint32_t data_len() { return ntohl( ((uint32_t*)buf_)[1] );}
    void data_len(uint32_t v) { ((uint32_t*)buf_)[1] = htonl(v);}

	uint16_t extern_header_len() { return ntohs( ((uint16_t*)buf_)[4] );}
    void extern_header_len(uint16_t v) { ((uint16_t*)buf_)[4] = htons((uint16_t)v);}

	uint32_t* buf_;
};

/** 
   网络通讯包封装
 */ 
class packet
{
public:
	packet(void);
	void destroy();
private:
	virtual ~packet(void);
public:
	/// 返回数据区首地址
	virtual char*  body_base();
	/// 剩余空间大小
	virtual size_t body_space();
	/// 有效数据长度(写指针 - 读指针)
	virtual size_t body_length();
	/// 缓冲区大小
	virtual size_t body_size();
	/// 获取读指针位置
	virtual char*  body_rd_ptr();
	/// 将当前读指针前进n个字节
	virtual void   body_rd_ptr(size_t n);
	/// 
	virtual void   body_rd_ptr(char* p);
	/// 获取写指针位置
	virtual char*  body_wr_ptr();
	/// 将当前写指针前进n个字节
	virtual void   body_wr_ptr(size_t n);
	/// 
	virtual void   body_wr_ptr(char* p);
	/// 
	virtual bool   body_copy_from(const void* data, size_t len);
	virtual void   body_reset();
	virtual void   token(void* p);
	virtual void*  token();
	virtual void*  body_impl();
	virtual void   body_impl(void* p);
	/// 分配内存, 如果当前已经分配过,则会自动先释放,再进行分配
	virtual bool   create_body(size_t body_size);
	virtual bool   create_header(size_t len = sp_ext::PKT_HEADER_LEN);

	/// 包头相关操作
	virtual void*  header_impl();
	virtual void   header_impl(void* p);
	virtual char*  header_base();
    /// 包头读写指针操作
	virtual char*  header_rd_ptr();
	virtual void   header_rd_ptr(size_t n);
	virtual void   header_rd_ptr(char* p);
	virtual char*  header_wr_ptr();
	virtual void   header_wr_ptr(size_t n);
	virtual void   header_wr_ptr(char* p);
	virtual void   header_reset();
    /// 组包头

    virtual bool   build_header(uint16_t pkt_type, uint8_t encrypt_type, 
        uint32_t data_len, uint16_t extern_header_len);

	/// 将整个数据包clone一份
	virtual packet* clone();
	virtual bool build_heartbeat_pkt();
public:
	static packet* create_packet();
public:
	void* token_;

	/// Pointer to beginning of next read.
	size_t rd_ptr_;
	/// Pointer to beginning of next write.
	size_t wr_ptr_;

	size_t h_rd_ptr_;
	size_t h_wr_ptr_;

	sp_evbuffer_t * body_;
	sp_evbuffer_t * header_;
};

class cdr_reader
{
public:
	cdr_reader();
	void destroy(); 
private:
	virtual ~cdr_reader();
public:
	virtual uint8_t read_1();
	virtual uint16_t read_2(bool to_host_order = true);
	virtual uint32_t read_4(bool to_host_order = true);
	virtual bool read_str(std::string& out); 

	virtual uint32_t read_array_elem_num(bool to_host_order = true);

	virtual bool read_1_array(void* buf, uint32_t elem_num);
	virtual bool read_2_array(void* buf, uint32_t elem_num, bool to_host_order = true);
	virtual bool read_4_array(void* buf, uint32_t elem_num, bool to_host_order = true);
public:
	virtual void recyle();
	virtual void set_buf(char* buf, size_t size);
	virtual char* get_buf();
	virtual size_t get_curr_pos();
	virtual void set_curr_pos(size_t v);
private:
	void recyle_i(char* buf, size_t size);
	bool read_array_i(void* buf, uint32_t elem_num, size_t char_len, bool to_host_order);
private:
	char* buf_;
	size_t buf_size_;
	size_t curr_pos_;
};

class cdr_writer
{
public:
	cdr_writer();
	void destroy(); 
private:
	virtual ~cdr_writer();
public:
	virtual bool write_1(uint8_t v);
	virtual bool write_2(uint16_t v, bool to_net_order = true);
	virtual bool write_4(uint32_t v, bool to_net_order = true);
	virtual bool write_str(const char* buf, uint32_t len); // len:字符串长度，不包结尾的\0, 实际发送字节有包括结尾的\0

	virtual bool write_array_elem_num(uint32_t elem_num, bool to_net_order = true);

	virtual bool write_1_array(const void* buf, uint32_t elem_num);
	virtual bool write_2_array(const void* buf, uint32_t elem_num, bool to_net_order = true);
	virtual bool write_4_array(const void* buf, uint32_t elem_num, bool to_net_order = true);
public:
	virtual void recyle();
	virtual void set_buf(char* buf, size_t size);
	virtual char* get_buf();
	virtual size_t get_curr_pos();
	virtual void set_curr_pos(size_t v);
private:
	void recyle_i(char* buf, size_t size);
	bool write_array_i(const void* buf, uint32_t elem_num, size_t elem_len, bool to_net_order);
private:
	char* buf_;
	size_t buf_size_;
	size_t curr_pos_;
};


}

#endif /// __SP_EXT_PACKET_H__
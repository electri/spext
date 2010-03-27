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

enum 
{ 
	PKT_VERSION = 1,
	PKT_FLAG = 0x1122, 
	PKT_HEADER_LEN = sizeof(uint32_t)*3,
	PKT_MAX_LEN = 1024*1024,
};

enum ectrl_flag
{
	/// 
	e_ctrl_flag_heartbeat = 0,
	e_ctrl_flag_data,
	e_ctrl_flag_echo,
	e_ctrl_flag_echo_reply,
};

/**   通讯协议数据包头定义:
*  
*    00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |       version         |      packet type      |             business  code                    |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                                   key (uint32)                                                |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                                   body  length (uint32)                                       |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |     header length     |   T1      |                      reserved                             |
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
*   business code(2字节):  业务类型
*   key(4字节):			原样返回
*   body length(4字节):	包体长度
*   header length(1字节):	包头长度
*   T1( 4 bit):			加密方式 0: 不加密
*   reserved:				必须置为全0
*/
struct packet_header
{
	packet_header( char* p)
		: buf_((uint32_t*)p)
	{
		;
	}

	bool is_valid() { return version() == PKT_VERSION; }

	uint32_t version(){ return (uint32_t)(((uint8_t*)buf_)[0]); }
	void version(uint32_t v) { ((uint8_t*)buf_)[0] = __SP_EXT_INT32_B0(v);}

	uint32_t conctrl_flag(){ return ((uint8_t*)buf_)[1]; }
	void conctrl_flag(uint32_t v) { ((uint8_t*)buf_)[1] = __SP_EXT_INT32_B0(v);}

	uint32_t conctrl_cmd(){ return ((uint8_t*)buf_)[2]; }
	void conctrl_cmd(uint32_t v) { ((uint8_t*)buf_)[2] = __SP_EXT_INT32_B0(v);}

	uint32_t seq(){ return ::ntohl(buf_[1]); }
	void seq(uint32_t v) { buf_[1] = ::htonl(v);}

	uint32_t data_len(){ return ::ntohl(buf_[2]); }
	void data_len(uint32_t v) { buf_[2] = ::htonl(v);}

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
	virtual bool   build_header(uint32_t body_len, uint32_t cmd, uint32_t flag = sp_ext::e_ctrl_flag_data, 
		uint32_t seq_num = 0, uint32_t vers = sp_ext::PKT_VERSION);

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
	virtual bool write_str(const char* buf, uint32_t len);

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
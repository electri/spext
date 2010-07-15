#include "StdAfx.h"
#include "../include/sp_ext_packet.h"
#include <spwin32buffer.hpp>

namespace sp_ext
{ 

packet::packet(void)
{
	body_ = NULL;
	header_ = NULL;

	rd_ptr_ = 0;
	wr_ptr_ = 0;

	h_rd_ptr_ = 0;
	h_wr_ptr_ = 0;

}
void 
packet::destroy()
{
	delete this;
}

packet::~packet(void)
{
	if( body_ )
	{
		sp_evbuffer_free( body_ );
		body_ = NULL;
	}

	if( header_ )
	{
		sp_evbuffer_free( header_ );
		header_ = NULL;
	}
}

char*  
packet::body_base()
{
	return (char *)EVBUFFER_DATA( body_ );
}

size_t 
packet::body_space()
{
	return this->body_base() + this->body_size() - this->body_wr_ptr();
}

size_t 
packet::body_length()
{
	return this->wr_ptr_ - this->rd_ptr_;
}

size_t   
packet::body_size()
{
	return EVBUFFER_LENGTH( this->body_ );
}

char*  
packet::body_rd_ptr()
{
	return this->body_base() + this->rd_ptr_;
}

void   
packet::body_rd_ptr(size_t n)
{
	this->rd_ptr_ += n;
}

void   
packet::body_rd_ptr(char* p)
{
	 this->rd_ptr_ = p - this->body_base();
}

char*  
packet::body_wr_ptr()
{
	return this->body_base() + this->wr_ptr_;
}

void   
packet::body_wr_ptr(size_t n)
{
	this->wr_ptr_ += n;
}

void   
packet::body_wr_ptr(char* p)
{
	this->wr_ptr_ = p - this->body_base();
}

bool   
packet::body_copy_from(const void* data, size_t len)
{
	if ( len <= this->body_space() )
	{
		memcpy(this->body_base() + this->wr_ptr_, data, len);
		this->wr_ptr_ += len;
		return true;
	}
	return false;
}

bool   
packet::create_body(size_t body_size)
{
	if ( body_ )
	{
		sp_evbuffer_free(body_);
		body_ = NULL;
	}
	body_ = sp_evbuffer_new();
	if ( body_ )
	{

		sp_evbuffer_expand( this->body_, body_size);
		EVBUFFER_LENGTH(this->body_) = body_size; // 有效长度直接设置为包体长度
		this->body_reset();
		return true;
	}
	return false;	
}

void   
packet::body_reset()
{
	this->rd_ptr_ = 0;
	this->wr_ptr_ = 0;
}

void   
packet::token(void* p)
{
	this->token_ = p;
}

void*  
packet::token()
{
	return this->token_;
}

void*  
packet::body_impl()
{
	return (void *)this->body_;
}

void   
packet::body_impl(void* p)
{
	this->body_ = (sp_evbuffer_t *)p;
}

bool   
packet::create_header(size_t len)
{
	if ( this->header_ )
	{
		sp_evbuffer_free(this->header_);
		this->header_ = NULL;
	}
	this->header_ = sp_evbuffer_new();
	if ( this->header_ )
	{
		sp_evbuffer_expand( this->header_, len);
		EVBUFFER_LENGTH(this->header_) = len; // 有效长度直接设置为头长度
		this->header_reset();
		return true;
	}
	return false;
}

void*  
packet::header_impl()
{
	return this->header_;
}

void   
packet::header_impl(void* p)
{
	this->header_ = (sp_evbuffer_t *)p;
}

char*  
packet::header_base()
{
	return (char *)EVBUFFER_DATA( this->header_ );
}

char*  
packet::header_rd_ptr()
{
	return this->header_base() + this->h_rd_ptr_;
}

void   
packet::header_rd_ptr(size_t n)
{
	this->h_rd_ptr_  += n;
}

void   
packet::header_rd_ptr(char* p)
{
	this->h_rd_ptr_ = p - this->header_base();
}

char*  
packet::header_wr_ptr()
{
	return this->header_base() + this->h_wr_ptr_;
}

void   
packet::header_wr_ptr(size_t n)
{
	this->h_wr_ptr_ += n;
}

void   
packet::header_wr_ptr(char* p)
{
	this->h_wr_ptr_ = p - this->header_base();
}

void   
packet::header_reset()
{
	this->h_rd_ptr_ = 0;
	this->h_wr_ptr_ = 0;
}



bool
packet::build_header(uint16_t pkt_type, uint8_t encrypt_type, 
                            uint32_t data_len, uint16_t extern_header_len)
{
	if( NULL == this->header_ )
	{
		if( !create_header() )
			return false;
	}
	else
	{
		this->header_reset();
	}

	sp_ext::packet_header h(this->header_base());


	h.version(PKT_VERSION);
    h.type(pkt_type);
    h.encrypt_type(encrypt_type);
    h.data_len(data_len);
    h.extern_header_len(extern_header_len);

	this->header_wr_ptr(sp_ext::PKT_HEADER_LEN);

	return true;
}

sp_ext::packet* 
packet::clone()
{
	packet* p = packet::create_packet();
	if( NULL == p )
		return NULL;

	if( this->header_ )
	{
		p->create_header(sp_ext::PKT_HEADER_LEN);
		
		if( NULL == p->header_ )
		{
			delete p;
			return NULL;
		}
		else
		{
			memcpy(EVBUFFER_DATA(p->header_), EVBUFFER_DATA(this->header_), EVBUFFER_LENGTH(this->header_));
			p->h_rd_ptr_ = this->h_rd_ptr_;
			p->h_wr_ptr_ = this->h_wr_ptr_;
		}
	}

	if( this->body_ )
	{
		p->create_body(this->body_size());

		if( NULL == p->body_ )
		{
			delete p;
			return NULL;
		}
		else
		{
			memcpy(EVBUFFER_DATA(p->body_), EVBUFFER_DATA(this->body_), EVBUFFER_LENGTH(this->body_));
			p->rd_ptr_ = this->rd_ptr_;
			p->wr_ptr_ = this->wr_ptr_;	
		}

		
	}
	p->token_ = this->token_;
	return p;
}

bool
packet::build_heartbeat_pkt()
{
	if( NULL == this->header_ )
	{
		if( !create_header() )
			return false;
	}

	sp_ext::packet_header h(header_base());
	
    h.version(PKT_VERSION);
    h.type(e_pkt_type_heartbeat);
    h.encrypt_type(0);
    h.data_len(0);
    h.extern_header_len(0);

	this->header_wr_ptr(sp_ext::PKT_HEADER_LEN);

	if( this->body_ )
	{
		sp_evbuffer_free( body_ );
		body_ = NULL;	
	}
	return true;
}

packet* 
packet::create_packet()
{
	packet* r = NULL;
	r = new packet();
	if( NULL == r)
	{
		//aot_log_error(AOT_LM_ERROR, "no memory! packet_impl::create_packet()");
	}
	return r;
}

cdr_reader::cdr_reader()
{
	recyle_i(NULL, 0);
}

void 
cdr_reader::destroy()
{
	delete this;
}

cdr_reader::~cdr_reader()
{
}

void 
cdr_reader::recyle_i(char* buf, size_t size)
{
	this->buf_ = buf;
	this->buf_size_ = size;
	this->curr_pos_ = 0;
}

void 
cdr_reader::recyle()
{
	recyle_i(NULL, 0);
}

uint8_t 
cdr_reader::read_1()
{
	bool b = ( this->buf_ && (this->curr_pos_ + 1 <= this->buf_size_) );
	//assert( b );
	if( !b )
	{
		//aot_log_error(AOT_LM_ALERT, "cdr_reader::read_1() out of range");
		return 0;
	}

	uint8_t r = static_cast<uint8_t>(this->buf_[this->curr_pos_]);
	++this->curr_pos_;
	return r;
}

uint16_t 
cdr_reader::read_2(bool to_host_order/* = true*/)
{
	bool b = (this->buf_ && (this->curr_pos_ + 2 <= this->buf_size_) );
	//assert( b );
	if( !b )
	{
		//aot_log_error(AOT_LM_ALERT, "cdr_reader::read_2() out of range");
		return 0;
	}

	uint16_t r = *(reinterpret_cast<uint16_t*>(this->buf_ + this->curr_pos_));
	this->curr_pos_ += 2;
	return to_host_order ? ::ntohs(r) : r;
}

uint32_t 
cdr_reader::read_4(bool to_host_order/* = true*/)
{
	bool b = ( this->buf_ && (this->curr_pos_ + 4 <= this->buf_size_) );
	//assert( b );
	if( !b )
	{
		//aot_log_error(AOT_LM_ALERT, "cdr_reader::read_4() out of range");
		return 0;
	}

	uint32_t r = *(reinterpret_cast<uint32_t*>(this->buf_ + this->curr_pos_));
	this->curr_pos_ += 4;
	return to_host_order ? ::ntohl(r) : r;
}

uint32_t 
cdr_reader::read_array_elem_num(bool to_host_order/* = true*/)
{
	return read_4(to_host_order);
}

bool 
cdr_reader::read_str(std::string& out)
{
	uint32_t len = read_array_elem_num(true);

	if( 0 == len )
	{
		out.clear();
	}
	else
	{
		bool b = (this->curr_pos_ + len <= this->buf_size_);
		//assert( b );
		if( !b )
		{
			//aot_log_error(AOT_LM_ALERT, "cdr_reader::read_str() out of range");
			return false;
		}

		out.assign(this->buf_ + this->curr_pos_, len - 1);
		this->curr_pos_ += len;
	}
	return true;
}

bool 
cdr_reader::read_1_array(void* buf, uint32_t elem_num)
{
	return read_array_i(buf, elem_num, 1, false);
}

bool 
cdr_reader::read_2_array(void* buf, uint32_t elem_num, bool to_host_order)
{
	return read_array_i(buf, elem_num, 2, to_host_order);
}

bool 
cdr_reader::read_4_array(void* buf, uint32_t elem_num, bool to_host_order)
{
	return read_array_i(buf, elem_num, 4, to_host_order);
}

bool 
cdr_reader::read_array_i(void* buf, uint32_t elem_num, size_t elem_len, bool to_host_order)
{
	if( elem_num == 0 )
		return true;

	uint32_t len = elem_num * elem_len;
	bool b = ( this->curr_pos_ + len <= this->buf_size_ );
	//assert( b );
	if( !b )
	{
		//aot_log_error(AOT_LM_ALERT, "cdr_reader::read_array_i() out of range");
		return false;
	}

	if( !to_host_order )
	{
		memcpy(buf, this->buf_ + this->curr_pos_, len);
		this->curr_pos_ += len;
		return true;
	}
	/// convert to host byte order
	switch( elem_len )
	{
	case 1:
		{
			memcpy(buf, this->buf_ + this->curr_pos_, len);
			this->curr_pos_ += len;
			return true;
		}
		break;
	case 2:
		{
			uint16_t v;
			uint16_t* d1 = reinterpret_cast<uint16_t*>(this->buf_ + this->curr_pos_);
			uint16_t* d2 = reinterpret_cast<uint16_t*>(buf);
			for( uint32_t i = 0; i < elem_num; ++i, ++d1, ++d2 )
			{
				v = *d1;
				v = ::ntohs(v);
				*d2 = v;
			}
			this->curr_pos_ += len;
			return true;
		}
		break;
	case 4:
		{
			uint32_t v;
			uint32_t* d1 = reinterpret_cast<uint32_t*>(this->buf_ + this->curr_pos_);
			uint32_t* d2 = reinterpret_cast<uint32_t*>(buf);
			for( uint32_t i = 0; i < elem_num; ++i, ++d1, ++d2 )
			{
				v = *d1;
				v = ::ntohl(v);
				*d2 = v;
			}
			this->curr_pos_ += len;
			return true;
		}
		break;
	default:
		//aot_log_error(AOT_LM_ALERT, "cdr_reader::read_array_i() no surpport: to_host_order = true and elem_len = %d", elem_len);
		return false;
	}

	return false;
}

void 
cdr_reader::set_buf(char* buf, size_t size)
{
	recyle_i(buf, size);
}

char* 
cdr_reader::get_buf()
{
	return this->buf_;
}

size_t 
cdr_reader::get_curr_pos()
{
	return this->curr_pos_;
}

void 
cdr_reader::set_curr_pos(size_t v)
{
	this->curr_pos_ = v;
}

/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */

cdr_writer::cdr_writer()
{
	recyle_i(NULL, 0);
}

cdr_writer::~cdr_writer()
{
	;
}

void 
cdr_writer::destroy()
{
	delete this;
}

bool 
cdr_writer::write_1(uint8_t v)
{
	bool b = ( this->buf_ && (this->curr_pos_ + 1 <= this->buf_size_) );
	//assert( b );
	if( !b )
	{
		//aot_log_error(AOT_LM_ALERT, "cdr_writer::write_1() out of range");
		return false;
	}

	*(reinterpret_cast<uint8_t*>(this->buf_ + this->curr_pos_)) = v;
	++this->curr_pos_;
	return true;
}

bool 
cdr_writer::write_2(uint16_t v, bool to_net_order/* = true*/)
{
	bool b = ( this->buf_ && (this->curr_pos_ + 2 <= this->buf_size_) );
	//assert( b );
	if( !b )
	{
		//aot_log_error(AOT_LM_ALERT, "cdr_writer::write_2() out of range");
		return false;
	}

	if( to_net_order )
	{
		v = ::htons(v);
	}

	*(reinterpret_cast<uint16_t*>(this->buf_ + this->curr_pos_)) = v;
	this->curr_pos_ += 2;
	return true;
}

bool 
cdr_writer::write_4(uint32_t v, bool to_net_order/* = true*/)
{
	bool b = ( this->buf_ && (this->curr_pos_ + 4 <= this->buf_size_) );
	//assert( b );
	if( !b )
	{
		//aot_log_error(AOT_LM_ALERT, "cdr_writer::write_4() out of range");
		return false;
	}

	if( to_net_order )
	{
		v = ::htonl(v);
	}

	*(reinterpret_cast<uint32_t*>(this->buf_ + this->curr_pos_)) = v;
	this->curr_pos_ += 4;
	return true;
}

bool 
cdr_writer::write_str(const char* buf, uint32_t len)
{
	if( 0 == len )
	{
		return write_array_elem_num(len);
	}

	if( !write_array_elem_num(len + 1) )
		return false;

	if( !write_array_i(buf, len, 1, false) )
		return false;
	return write_1(0);
}

bool 
cdr_writer::write_array_elem_num(uint32_t elem_num, bool to_net_order/* = true*/)
{
	return write_4(elem_num, to_net_order);
}

bool 
cdr_writer::write_1_array(const void* buf, uint32_t elem_num)
{
	return write_array_i(buf, elem_num, 1, false);
}

bool 
cdr_writer::write_2_array(const void* buf, uint32_t elem_num, bool to_net_order/* = true*/)
{
	return write_array_i(buf, elem_num, 2, to_net_order);
}

bool 
cdr_writer::write_4_array(const void* buf, uint32_t elem_num, bool to_net_order/* = true*/)
{
	return write_array_i(buf, elem_num, 4, to_net_order);
}

void 
cdr_writer::recyle()
{
	recyle_i(NULL, 0);
}

void 
cdr_writer::set_buf(char* buf, size_t size)
{
	recyle_i(buf, size);
}

char* 
cdr_writer::get_buf()
{
	return this->buf_;
}

size_t 
cdr_writer::get_curr_pos()
{
	return this->curr_pos_;
}

void 
cdr_writer::set_curr_pos(size_t v)
{
	this->curr_pos_ = v;
}

void 
cdr_writer::recyle_i(char* buf, size_t size)
{
	this->buf_ = buf;
	this->buf_size_ = size;
	this->curr_pos_ = 0;
}

bool 
cdr_writer::write_array_i(const void* buf, uint32_t elem_num, size_t elem_len, bool to_net_order)
{
	if( elem_num == 0 )
		return true;

	uint32_t len = elem_num * elem_len;
	bool b = ( this->curr_pos_ + len <= this->buf_size_ );
	//assert( b );
	if( !b )
	{
		//aot_log_error(AOT_LM_ALERT, "cdr_reader_impl::write_array_i() out of range");
		return false;
	}

	if( !to_net_order )
	{
		memcpy(this->buf_ + this->curr_pos_, buf, len);
		this->curr_pos_ += len;
		return true;
	}
	/// convert to network byte order
	switch( elem_len )
	{
	case 1:
		{
			memcpy(this->buf_ + this->curr_pos_, buf, len);
			this->curr_pos_ += len;
			return true;
		}
		break;
	case 2:
		{
			uint16_t v;
			uint16_t* d1 = reinterpret_cast<uint16_t*>(this->buf_ + this->curr_pos_);
			const uint16_t* d2 = reinterpret_cast<const uint16_t*>(buf);
			for( uint32_t i = 0; i < elem_num; ++i, ++d1, ++d2 )
			{
				v = *d2;
				v = ::htons(v);
				*d1 = v;
			}
			this->curr_pos_ += len;
			return true;
		}
		break;
	case 4:
		{
			uint32_t v;
			uint32_t* d1 = reinterpret_cast<uint32_t*>(this->buf_ + this->curr_pos_);
			const uint32_t* d2 = reinterpret_cast<const uint32_t*>(buf);
			for( uint32_t i = 0; i < elem_num; ++i, ++d1, ++d2 )
			{
				v = *d2;
				v = ::htonl(v);
				*d1 = v;
			}
			this->curr_pos_ += len;
			return true;
		}
		break;
	default:
		//aot_log_error(AOT_LM_ALERT, "cdr_reader_impl::write_array_i() no surpport: to_net_order = true and elem_len = %d", elem_len);
		return false;
	}

	return false;
}


}/// namespace sp_ext
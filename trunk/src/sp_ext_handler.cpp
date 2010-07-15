#include "StdAfx.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include "spporting.hpp"
#include "../include/sp_ext_handler.h"


#include "sprequest.hpp"
#include "spresponse.hpp"
#include "spbuffer.hpp"
#include "spmsgblock.hpp"

#ifdef WIN32
#include "spiocpdispatcher.hpp"
#else
#include "spdispatcher.hpp"
#endif

namespace sp_ext
{ 

/**
     packet_decoder 
 */
packet_decoder::packet_decoder()
{
	last_packet_ = NULL;
    recv_status_ = e_recv_status_head;
    body_len_ = 0;
    ext_head_len_ = 0;
}

packet_decoder::~packet_decoder()
{
	if ( last_packet_ )
	{
		last_packet_->destroy();
	}
	last_packet_ = NULL;
}

bool 
packet_decoder::decode_i(SP_Buffer * in_buffer, int& ret)
{
    bool continue_loop = false;
    size_t p_size = in_buffer->getSize();
    switch (  this->recv_status_ )
    {
    case packet_decoder::e_recv_status_head:
        {
            if ( p_size >= sp_ext::PKT_HEADER_LEN )
            {
                sp_ext::packet_header h((char *)in_buffer->getRawBuffer());

                if( !h.is_valid() )// 非法包重新收
                {
                    in_buffer->reset();
                }
                else
                {
                    if ( !last_packet_ )
                    {
                        last_packet_ = new packet();
                    }
                    last_packet_->create_header();
                    last_packet_->build_header(h.type(), h.encrypt_type(), h.data_len(), h.extern_header_len());
                    this->body_len_ = h.data_len();
                    this->ext_head_len_ = h.extern_header_len();
                    this->recv_status_ = packet_decoder::e_recv_status_body;

                    // 判断包是否已经完整
                    if ( p_size >= sp_ext::PKT_HEADER_LEN + this->body_len_ + this->ext_head_len_ )
                    {
                        continue_loop = true;
                    }
                }
            }
        }
        break;
    case packet_decoder::e_recv_status_body:
        {
            if ( p_size >= sp_ext::PKT_HEADER_LEN + this->body_len_ + this->ext_head_len_ )
            {
                last_packet_->create_body(this->body_len_);
                last_packet_->body_copy_from((char *)in_buffer->getRawBuffer() + sp_ext::PKT_HEADER_LEN + this->ext_head_len_, this->body_len_);

                in_buffer->erase(sp_ext::PKT_HEADER_LEN + this->ext_head_len_ + this->body_len_);
                ret = SP_MsgDecoder::eOK;
                this->recv_status_ = packet_decoder::e_recv_status_head;
            }
        }
        break;
    }
    return continue_loop;
}

int 
packet_decoder::decode( SP_Buffer * in_buffer )
{
	int ret = SP_MsgDecoder::eMoreData;
    size_t p_size = in_buffer->getSize();

	if( p_size <= 0 ) return  ret;

    while( decode_i(in_buffer, ret) );
	return ret;
}

packet * 
packet_decoder::get_packet()
{
	return this->last_packet_;
}

packet * 
packet_decoder::take_packet()
{
	packet * t = this->last_packet_;
	this->last_packet_ = NULL;
	return t;
}

/**
    tunnel_handler 
 */
tunnel_handler::tunnel_handler( SP_IocpDispatcher * dispatcher)
{
	dispatcher_ = dispatcher;
}

tunnel_handler::~tunnel_handler()
{

}

// return -1 : terminate session, 0 : continue
int 
tunnel_handler::start( SP_Request * request, SP_Response * response )
{
	request->setMsgDecoder(new packet_decoder());
	return 0;
}

// return -1 : terminate session, 0 : continue
int 
tunnel_handler::handle( SP_Request * request, SP_Response * response )
{
	packet_decoder * decoder = (packet_decoder*)request->getMsgDecoder();
	packet* p = decoder->take_packet();
	return on_incoming_pkt(request, response, p);
}

int 
tunnel_handler::on_incoming_pkt(SP_Request * request, SP_Response * response, packet* packet)
{
	if ( packet )
	{
		// 对包的处理
		packet->destroy();
	}
	return 0;
}
void 
tunnel_handler::error( SP_Response * response )
{

}

void 
tunnel_handler::timeout( SP_Response * response )
{

}

void 
tunnel_handler::close()
{
}

}
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
}

packet_decoder::~packet_decoder()
{
	if ( last_packet_ )
	{
		last_packet_->destroy();
	}
	last_packet_ = NULL;
}

int 
packet_decoder::decode( SP_Buffer * in_buffer )
{
	int ret = SP_MsgDecoder::eMoreData;
	if( in_buffer->getSize() > 0 ) 
	{
		sp_ext::packet_header h((char *)in_buffer->getRawBuffer());

		
		if( h.conctrl_flag() != sp_ext::e_ctrl_flag_data )// 非法包重新收
		{
			in_buffer->reset();
			ret = SP_MsgDecoder::eMoreData;
		}
		else// 合法包
		{
			if ( !last_packet_ )
			{
				last_packet_ = new packet();
			}
			last_packet_->create_header();
			last_packet_->build_header(h.data_len(), h.conctrl_cmd(), h.conctrl_flag(), h.seq(), h.version());
            
			last_packet_->create_body(h.data_len());
			last_packet_->body_copy_from((char *)in_buffer->getRawBuffer() + sp_ext::PKT_HEADER_LEN, h.data_len());

			in_buffer->reset();
			ret = SP_MsgDecoder::eOK;	

		}
	}
	
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
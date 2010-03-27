/** Copyright (c) 2008-2010
* All rights reserved.
* 
* �ļ�����:	sp_ext_handler.h     
* ժ	 Ҫ:	��װ����ͨ������
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:	2010��03��07��
*/
#ifndef __SP_EXT_HANDLER_H__
#define __SP_EXT_HANDLER_H__

#include <spthread.hpp>
#include <spmsgdecoder.hpp>
#include <sphandler.hpp>
#include <sprequest.hpp>
#include <spresponse.hpp>
#include <spiocpdispatcher.hpp>
#include "sp_ext_packet.h"

namespace sp_ext
{ 

class packet_decoder : public SP_MsgDecoder 
{
public:
	packet_decoder();
	virtual ~packet_decoder();

	virtual int decode( SP_Buffer * in_buffer );

	packet * get_packet();
	packet * take_packet();
private:
	packet * last_packet_;
};



class tunnel_handler : public SP_Handler 
{
public:
	tunnel_handler( SP_IocpDispatcher * dispatcher);

	virtual ~tunnel_handler();

	// return -1 : terminate session, 0 : continue
	virtual int start( SP_Request * request, SP_Response * response );

	// return -1 : terminate session, 0 : continue
	virtual int handle( SP_Request * request, SP_Response * response );
   
	// return -1 : terminate session, 0 : continue
	virtual int on_incoming_pkt(SP_Request * request, SP_Response * response, packet* packet);

	virtual void error( SP_Response * response );

	virtual void timeout( SP_Response * response );

	virtual void close();


private:
	SP_IocpDispatcher * dispatcher_;
};

}/// namespace sp_ext
#endif /// __SP_EXT_HANDLER_H__

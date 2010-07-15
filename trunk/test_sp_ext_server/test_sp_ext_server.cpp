/*
* Copyright 2007-2008 Stephen Liu
* For license terms, see the file COPYING along with this library.
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#include "spporting.hpp"
#include "spioutils.hpp"
//#include "sptunnelimpl.hpp"


/*
#ifdef OPENSSL
#include "spopenssl.hpp"
#else
#include "spmatrixssl.hpp"
#endif
*/

#include <sp_ext_handler.h>
#include <sp_ext_log.h>
#include <sp_ext_api_tools.h>

#include <spgetopt.h>
#include <spiocpdispatcher.hpp>
#include <spiochannel.hpp>

sp_ext::sp_ext_log g_log_obj;

class client_tunnel : public sp_ext::tunnel_handler
{
public:
    client_tunnel( SP_IocpDispatcher * dispatcher)
        :tunnel_handler(dispatcher)
    {
        
    }

    virtual ~client_tunnel()
    {

    }

    // 必须把packet销毁掉
    virtual int on_incoming_pkt(SP_Request * request, SP_Response * response, sp_ext::packet* packet)
    {
        sp_ext::packet_header ph(packet->header_base());
       
        g_log_obj.log_fmt(SP_LOG_DEBUG, "client addr = %s : %d\n", request->getClientIP(), request->getClientPort());
        g_log_obj.log_fmt(SP_LOG_DEBUG, "version = %d\n", ph.version());
        g_log_obj.log_fmt(SP_LOG_DEBUG, "type = %d\n", ph.type());
        g_log_obj.log_fmt(SP_LOG_DEBUG, "encrypt_type = %d\n", ph.encrypt_type());
        g_log_obj.log_fmt(SP_LOG_DEBUG, "data_len = %d\n", ph.data_len());
        g_log_obj.log_fmt(SP_LOG_DEBUG, "extern_header_len = %d\n", ph.extern_header_len());
        std::string t;
        t.assign(packet->body_base(), packet->body_length());
        g_log_obj.log_fmt(SP_LOG_DEBUG, "recv data = %s, strlen = %d\n", t.c_str(), t.length());
        
        SP_Message * msg = response->getReply();

        if ( sp_ext::e_pkt_type_echo == ph.type() )
        {
            ph.type(sp_ext::e_pkt_type_echo_reply);
            msg->getMsg()->append(packet->header_base(), sp_ext::PKT_HEADER_LEN);
            msg->getMsg()->append(packet->body_base(), ph.data_len());
        }
        else if( sp_ext::e_pkt_type_heartbeat == ph.type() )
        {
            g_log_obj.log_fmt(SP_LOG_DEBUG, "recv heart beat packet.");   
        }
        else if ( sp_ext::e_pkt_type_data == ph.type() )
        {
            // 处理业务逻辑 
        }
        
        if ( packet )
        {
            packet->destroy();
        }
        return 0;
    }

    virtual void error( SP_Response * response )
    {
        g_log_obj.log_fmt(SP_LOG_DEBUG, "error!\n");
    }

    virtual void timeout( SP_Response * response )
    {
        g_log_obj.log_fmt(SP_LOG_DEBUG, "timeout!\n");
    }

    virtual void close()
    {
        g_log_obj.log_fmt(SP_LOG_DEBUG, "close!\n");
    }
};


int main( int argc, char * argv[] )
{
    std::string exe_path = sp_ext::get_app_path();
    exe_path += "output.log";

    g_log_obj.init_log(SP_LOG_DEBUG, true, 10240, exe_path.c_str() );
	int port = 8080, maxThreads = 10;
	char * dstHost = "66.249.89.99";
	int dstPort = 80;

	extern char *optarg ;
	int c;

	while( ( c = getopt ( argc, argv, "p:t:r:v" )) != EOF ) 
	{
		switch ( c ) {
			case 'p' :
				port = atoi( optarg );
				break;
			case 't':
				maxThreads = atoi( optarg );
				break;
			case 'r':
				{
					dstHost = strdup( optarg );
					char * pos = strchr( dstHost, ':' );
					if( NULL != pos ) {
						dstPort = atoi( pos + 1 );
						*pos = '\0';
					}
					break;
				}
			case '?' :
			case 'v' :
				printf( "Usage: %s [-p <port>] [-t <threads>] [-r <backend>]\n", argv[0] );
				exit( 0 );
		}
	}

#ifdef LOG_PERROR
	sp_openlog( "sptunnel", LOG_CONS | LOG_PID | LOG_PERROR, LOG_USER );
#else
	sp_openlog( "sptunnel", LOG_CONS | LOG_PID, LOG_USER );
#endif

	if( 0 != sp_initsock() ) assert( 0 );

	g_log_obj.log_fmt(SP_LOG_DEBUG, "Backend server - %s:%d", dstHost, dstPort );

	int maxConnections = 50000, reqQueueSize = 50000;
	const char * refusedMsg = "System busy, try again later.";

	int listenFd = -1;
	if( 0 == SP_IOUtils::tcpListen( "", port, &listenFd ) ) {

		SP_IocpDispatcher dispatcher( new SP_DefaultCompletionHandler(), maxThreads );

		dispatcher.setTimeout( 60 );
		dispatcher.dispatch();

/*
#ifdef	OPENSSL
		SP_OpensslChannelFactory * sslFactory = new SP_OpensslChannelFactory();
#else
		SP_MatrixsslChannelFactory * sslFactory = new SP_MatrixsslChannelFactory();
#endif

#ifdef WIN32
		char basePath[ 256 ] = { 0 }, crtPath[ 256 ] = { 0 }, keyPath[ 256 ] = { 0 };
		spwin32_pwd( basePath, sizeof( basePath ) );
		snprintf( crtPath, sizeof( crtPath ), "%s\\..\\..\\..\\sptunnel\\demo.crt", basePath );
		snprintf( keyPath, sizeof( keyPath ), "%s\\..\\..\\..\\sptunnel\\demo.key", basePath );
		sslFactory->init( crtPath, keyPath );
#else
		sslFactory->init( "demo.crt", "demo.key" );
#endif
*/

		for( ; ; ) 
		{
			struct sockaddr_in addr;
			socklen_t socklen = sizeof( addr );
			int fd = accept( listenFd, (struct sockaddr*)&addr, &socklen );

			if( fd > 0 ) 
			{
				if( dispatcher.getSessionCount() >= maxConnections
					|| dispatcher.getReqQueueLength() >= reqQueueSize ) 
				{
						write( fd, refusedMsg, strlen( refusedMsg ) );
						closesocket( fd );
				} 
				else 
				{
					client_tunnel* handler  = new client_tunnel(&dispatcher );
					/*
					dispatcher.push( fd, handler, sslFactory->create() );
					*/

					// for non-ssl tunnel
					dispatcher.push( fd, handler, new SP_DefaultIOChannel() );
				}
			} 
			else 
			{
				break;
			}
		}
	}

	sp_closelog();

	return 0;
}


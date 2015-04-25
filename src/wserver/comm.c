#include "wserver.h"

int comm_AsyncConnectToRegisterServer( struct ServerEnv *penv , int skip_connect_flag )
{
	int			nret = 0 ;
	
	penv->connect_flag = 0 ;
	
	if( skip_connect_flag == 0 )
	{
		nret = AsyncConnectSocket( penv->param.rserver_ip , penv->param.rserver_port , & (penv->connect_session) ) ;
		if( nret == CONNECTING_IN_PROGRESS )
		{
			InfoLog( __FILE__ , __LINE__ , "CreateConnectSocket[%s:%d] done , but connecting is progressing" , penv->param.rserver_ip , penv->param.rserver_port );
			AddOutputSockToEpoll( penv->epoll_socks , & (penv->connect_session) );
			return nret;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "CreateConnectSocket[%s:%d] failed[%d]errno[%d]" , penv->param.rserver_ip , penv->param.rserver_port , nret , errno );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "CreateConnectSocket[%s:%d] ok" , penv->param.rserver_ip , penv->param.rserver_port );
		}
	}
	
	nret = proto_WorkerRegisterRequest( penv , & (penv->connect_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_WorkerRegisterRequest failed[%d]errno[%d]" , nret , errno );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "proto_WorkerRegisterRequest ok" );
	}
	
	AddOutputSockToEpoll( penv->epoll_socks , & (penv->connect_session) );
	
	penv->connect_flag = 1 ;
	
	return 0;
}

int comm_OnConnectedSocketInput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	nret = AsyncReceiveAndSendSocketData( psession , (void*) penv , & proto , 0 ) ;
	if( nret == PEER_CLOSED )
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData done" );
		DeleteSockFromEpoll( penv->epoll_socks , psession );
		close( psession->sock );
		ResetSocketSession( psession );
	}
	else if( nret == NO_SEND_RESPONSE )
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData done" );
	}
	else if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData failed[%d] , errno[%d]" , nret , errno );
		DeleteSockFromEpoll( penv->epoll_socks , psession );
		close( psession->sock );
		ResetSocketSession( psession );
		
		nret = comm_AsyncConnectToRegisterServer( penv , 0 ) ;
		if( nret == CONNECTING_IN_PROGRESS )
		{
			InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] done , but connecting is progressing" , penv->param.rserver_ip , penv->param.rserver_port );
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] failed[%d] , errno[%d]" , penv->param.rserver_ip , penv->param.rserver_port , nret , errno );
			return -1;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] ok" , penv->param.rserver_ip , penv->param.rserver_port );
		}
		
		return 0;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData ok" );
		ModifyOutputSockFromEpoll( penv->epoll_socks , psession );
	}
	
	return 0;
}

int comm_OnConnectedSocketOutput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	if( penv->connect_flag == 0 )
	{
#if ( defined __linux ) || ( defined __unix )
		int			error , code ;
#endif
		_SOCKLEN_T		addr_len ;

#if ( defined __linux ) || ( defined __unix )
		addr_len = sizeof(int) ;
		code = getsockopt( psession->sock , SOL_SOCKET , SO_ERROR , & error , & addr_len ) ;
		if( code < 0 && error )
#elif ( defined _WIN32 )
		addr_len = sizeof(struct sockaddr_in) ;
		nret = connect( p_forward_session_server->server_addr.sock , ( struct sockaddr *) & (p_forward_session_server->server_addr.netaddr.sockaddr) , addr_len ) ;
		if( ! ( nret == -1 && _ERRNO == _EISCONN ) )
#endif
		{
			DeleteSockFromEpoll( penv->epoll_socks , psession );
			close( psession->sock );
			ResetSocketSession( psession );
			
			nret = comm_AsyncConnectToRegisterServer( penv , 0 ) ;
			if( nret == CONNECTING_IN_PROGRESS )
			{
				InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] done , but connecting is progressing" , penv->param.rserver_ip , penv->param.rserver_port );
			}
			else if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] failed[%d] , errno[%d]" , penv->param.rserver_ip , penv->param.rserver_port , nret , errno );
				return -1;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] ok" , penv->param.rserver_ip , penv->param.rserver_port );
			}
			
			return 0;
		}
		else
		{
			nret = comm_AsyncConnectToRegisterServer( penv , 1 ) ;
			if( nret == CONNECTING_IN_PROGRESS )
			{
				InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] done , but connecting is progressing" , penv->param.rserver_ip , penv->param.rserver_port );
			}
			else if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] failed[%d] , errno[%d]" , penv->param.rserver_ip , penv->param.rserver_port , nret , errno );
				return -1;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] ok" , penv->param.rserver_ip , penv->param.rserver_port );
			}
		}
	}
	else
	{
		nret = AsyncSendSocketData( psession ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "AsyncSendSocketData failed[%d] , errno[%d]" , nret , errno );
			DeleteSockFromEpoll( penv->epoll_socks , psession );
			close( psession->sock );
			ResetSocketSession( psession );
			
			nret = comm_AsyncConnectToRegisterServer( penv , 0 ) ;
			if( nret == CONNECTING_IN_PROGRESS )
			{
				InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] done , but connecting is progressing" , penv->param.rserver_ip , penv->param.rserver_port );
			}
			else if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] failed[%d] , errno[%d]" , penv->param.rserver_ip , penv->param.rserver_port , nret , errno );
				return -1;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] ok" , penv->param.rserver_ip , penv->param.rserver_port );
			}
			
			return 0;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "AsyncSendSocketData ok" );
		}
		
		if( psession->send_len == psession->total_send_len )
		{
			ModifyInputSockFromEpoll( penv->epoll_socks , psession );
		}
	}
	
	return 0;
}

int comm_OnConnectedSocketError( struct ServerEnv *penv , struct SocketSession *psession )
{
	ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error , errno[%d]" , psession->sock , errno );
	DeleteSockFromEpoll( penv->epoll_socks , psession );
	close( psession->sock );
	
	return 0;
}

int comm_OnListenSocketInput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	if( penv->connect_flag != 2 )
	{
		nret = DiscardAcceptSocket( penv->epoll_socks , psession->sock ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DiscardAcceptSocket failed[%d] , errno[%d]" , nret , errno );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "DiscardAcceptSocket ok" );
		}
	}
	else
	{
		nret = AcceptSocket( penv->epoll_socks , psession->sock , & (penv->accepted_session) ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "AcceptSocket failed[%d] , errno[%d]" , nret , errno );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "AcceptSocket ok" );
		}
		
		SetNonBlocking( penv->accepted_session.sock );
		
		AddInputSockToEpoll( penv->epoll_socks , & (penv->accepted_session) );
		
		penv->accepted_flag = 1 ;
	}
	
	return 0;
}

int comm_OnAcceptedSocketInput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	nret = AsyncReceiveAndSendSocketData( psession , (void*) penv , & proto , 0 ) ;
	if( nret == PEER_CLOSED )
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData done" );
		DeleteSockFromEpoll( penv->epoll_socks , psession );
		close( psession->sock );
		ResetSocketSession( psession );
	}
	else if( nret == NO_SEND_RESPONSE )
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData done" );
	}
	else if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData failed[%d] , errno[%d]" , nret , errno );
		DeleteSockFromEpoll( penv->epoll_socks , psession );
		close( psession->sock );
		ResetSocketSession( psession );
		penv->accepted_flag = 0 ;
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData ok" );
		ModifyOutputSockFromEpoll( penv->epoll_socks , psession );
	}
	
	return 0;
}

int comm_OnAcceptedSocketOutput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	nret = AsyncSendSocketData( psession ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AsyncSendSocketData failed[%d] , errno[%d]" , nret , errno );
		DeleteSockFromEpoll( penv->epoll_socks , psession );
		close( psession->sock );
		ResetSocketSession( psession );
		penv->accepted_flag = 0 ;
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncSendSocketData ok" );
	}
	
	if( psession->send_len == psession->total_send_len )
	{
		ModifyInputSockFromEpoll( penv->epoll_socks , psession );
	}
	
	if( penv->accepted_flag == 3 )
	{
		InfoLog( __FILE__ , __LINE__ , "close sock[%d]" , psession->sock );
		penv->accepted_flag = 0 ;
	}
	
	return 0;
}

int comm_OnAcceptedSocketError( struct ServerEnv *penv , struct SocketSession *psession )
{
	ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error , errno[%d]" , psession->sock , errno );
	DeleteSockFromEpoll( penv->epoll_socks , psession );
	close( psession->sock );
	
	return 0;
}


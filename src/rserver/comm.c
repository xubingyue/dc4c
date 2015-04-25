#include "rserver.h"

int comm_OnListenSocketInput( struct ServerEnv *penv , struct SocketSession *psession )
{
	struct SocketSession	*p_new_session = NULL ;
	int			nret = 0 ;
	
	p_new_session = GetUnusedSocketSession( penv->accepted_session_array , MAXCOUNT_ACCEPTED_SESSION , & (penv->p_slibing_accepted_session) ) ;
	if( p_new_session == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "GetUnusedSocketSession failed , too many sessions" );
	}
	
	nret = AcceptSocket( penv->epoll_socks , psession->sock , p_new_session ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AcceptSocket failed[%d] , errno[%d]" , nret , errno );
		ResetSocketSession( p_new_session );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "AcceptSocket ok" );
	}
	
	SetNonBlocking( p_new_session->sock );
	
	AddInputSockToEpoll( penv->epoll_socks , p_new_session );
	
	return 0;
}

int comm_OnAcceptedSocketInput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	if( psession->comm_protocol_mode == COMMPROTO_LINE )
	{
		nret = AsyncReceiveCommandAndSendResponse( psession , (void*) penv , & proto , 0 ) ;
		if( nret == PEER_CLOSED )
		{
			DeleteSockFromEpoll( penv->epoll_socks , psession );
			close( psession->sock );
			ResetSocketSession( psession );
			return 0;
		}
		else if( nret == NO_SEND_RESPONSE )
		{
			ModifyOutputSockFromEpoll( penv->epoll_socks , psession );
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveCommandAndSendResponse failed[%d] , errno[%d]" , nret , errno );
			DeleteSockFromEpoll( penv->epoll_socks , psession );
			close( psession->sock );
			ResetSocketSession( psession );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "AsyncReceiveCommandAndSendResponse ok" );
		}
	}
	else
	{
		nret = AsyncReceiveAndSendSocketData( psession , (void*) penv , & proto , OPTION_ASYNC_CHANGE_MODE_FLAG ) ;
		if( nret == PEER_CLOSED )
		{
			InfoLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData done" );
			epoll_ctl( penv->epoll_socks , EPOLL_CTL_DEL , psession->sock , NULL );
			close( psession->sock );
			ResetSocketSession( psession );
		}
		else if( nret == CHANGE_COMM_PROTOCOL_MODE )
		{
			return AsyncReceiveCommandAndSendResponse( psession , penv , proto , OPTION_ASYNC_SKIP_RECV_FLAG );
		}
		else if( nret == NO_SEND_RESPONSE )
		{
			InfoLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData done" );
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData failed[%d] , errno[%d]" , nret , errno );
			epoll_ctl( penv->epoll_socks , EPOLL_CTL_DEL , psession->sock , NULL );
			close( psession->sock );
			ResetSocketSession( psession );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "AsyncReceiveAndSendSocketData ok" );
			ModifyOutputSockFromEpoll( penv->epoll_socks , psession );
		}
	}
	
	return 0;
}

int comm_OnAcceptedSocketOutput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int		nret = 0 ;
	
	nret = AsyncSendSocketData( psession ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AsyncSendSocketData failed[%d] , errno[%d]" , nret , errno );
		DeleteSockFromEpoll( penv->epoll_socks , psession );
		close( psession->sock );
		ResetSocketSession( psession );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncSendSocketData ok" );
	}
	
	if( psession->send_len == psession->total_send_len )
	{
		ModifyInputSockFromEpoll( penv->epoll_socks , psession );
	}
	
	return 0;
}

int comm_OnAcceptedSocketError( struct ServerEnv *penv , struct SocketSession *psession )
{
	ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error , errno[%d]" , psession->sock , errno );
	epoll_ctl( penv->epoll_socks , EPOLL_CTL_DEL , psession->sock , NULL );
	close( psession->sock );
	
	return 0;
}


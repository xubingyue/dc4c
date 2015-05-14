#include "rserver.h"

int comm_CloseAcceptedSocket( struct ServerEnv *penv , struct SocketSession *psession )
{
	app_WorkerUnregister( penv , psession );
	
	DeleteSockFromEpoll( penv->epoll_socks , psession );
	CloseSocket( psession );
	
	return 0;
}

int comm_OnListenSocketInput( struct ServerEnv *penv , struct SocketSession *psession )
{
	struct SocketSession	*p_new_session = NULL ;
	int			nret = 0 ;
	
	p_new_session = GetUnusedSocketSession( penv->accepted_session_array , MAXCOUNT_ACCEPTED_SESSION , & (penv->p_slibing_accepted_session) ) ;
	if( p_new_session == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "GetUnusedSocketSession failed , too many sessions" );
	}
	
	nret = AcceptSocket( psession->sock , p_new_session ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AcceptSocket failed[%d] , errno[%d]" , nret , errno );
		return nret;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "AcceptSocket ok" );
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
		nret = AsyncReceiveCommand( psession , 0 ) ;
_GOTO_CHECK_ASYNC_RECEIVE_COMMAND :
		if( nret == RETURN_RECEIVING_IN_PROGRESS )
		{
			DebugLog( __FILE__ , __LINE__ , "AsyncReceiveCommand done" );
		}
		else if( nret == RETURN_PEER_CLOSED )
		{
			comm_CloseAcceptedSocket( penv , psession );
			return 0;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveCommand failed[%d] , errno[%d]" , nret , errno );
			comm_CloseAcceptedSocket( penv , psession );
			return nret;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "AsyncReceiveCommand ok , call proto" );
			
			nret = proto( penv , psession ) ;
			if( nret == RETURN_QUIT )
			{
				InfoLog( __FILE__ , __LINE__ , "proto return ok" );
				comm_CloseAcceptedSocket( penv , psession );
				return 0;
			}
			else if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "proto return failed[%d]" , nret );
				comm_CloseAcceptedSocket( penv , psession );
				return 0;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "proto ok" );
			}
			
			nret = AfterDoCommandProtocol( psession ) ;
			if( nret == RETURN_NO_SEND_RESPONSE )
			{
				DebugLog( __FILE__ , __LINE__ , "AfterDoCommandProtocol done" );
				return 0;
			}
			else if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "AfterDoCommandProtocol return failed[%d]" , nret );
				comm_CloseAcceptedSocket( penv , psession );
				return 0;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "AfterDoCommandProtocol ok" );
				ModifyOutputSockFromEpoll( penv->epoll_socks , psession );
				nret = AsyncReceiveCommand( psession , OPTION_ASYNC_SKIP_RECV_FLAG ) ;
				goto _GOTO_CHECK_ASYNC_RECEIVE_COMMAND;
			}
		}
	}
	else
	{
		nret = AsyncReceiveSocketData( psession , OPTION_ASYNC_CHANGE_MODE_FLAG ) ;
		if( nret == RETURN_RECEIVING_IN_PROGRESS )
		{
			DebugLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData ok" );
			return 0;
		}
		else if( nret == RETURN_PEER_CLOSED )
		{
			DebugLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData done" );
			comm_CloseAcceptedSocket( penv , psession );
			return 0;
		}
		else if( nret == RETURN_CHANGE_COMM_PROTOCOL_MODE )
		{
			nret = AsyncReceiveCommand( psession , OPTION_ASYNC_SKIP_RECV_FLAG ) ;
			goto _GOTO_CHECK_ASYNC_RECEIVE_COMMAND;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData failed[%d] , errno[%d]" , nret , errno );
			comm_CloseAcceptedSocket( penv , psession );
			return nret;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData ok , call proto" );
			
			nret = proto( penv , psession ) ;
			if( nret == RETURN_QUIT )
			{
				InfoLog( __FILE__ , __LINE__ , "proto return ok" );
				comm_CloseAcceptedSocket( penv , psession );
				return 0;
			}
			else if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "proto return failed[%d]" , nret );
				comm_CloseAcceptedSocket( penv , psession );
				return 0;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "proto ok" );
			}
			
			nret = AfterDoProtocol( psession ) ;
			if( nret == RETURN_NO_SEND_RESPONSE )
			{
				DebugLog( __FILE__ , __LINE__ , "AfterDoProtocol done" );
				return 0;
			}
			else if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "AfterDoProtocol return failed[%d]" , nret );
				comm_CloseAcceptedSocket( penv , psession );
				return 0;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "AfterDoProtocol ok" );
				ModifyOutputSockFromEpoll( penv->epoll_socks , psession );
			}
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
		comm_CloseAcceptedSocket( penv , psession );
		return 0;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "AsyncSendSocketData ok" );
	}
	
	if( psession->send_len == psession->total_send_len )
	{
		CleanSendBuffer( psession );
		ModifyInputSockFromEpoll( penv->epoll_socks , psession );
	}
	
	return 0;
}

int comm_OnAcceptedSocketError( struct ServerEnv *penv , struct SocketSession *psession )
{
	ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error , errno[%d]" , psession->sock , errno );
	comm_CloseAcceptedSocket( penv , psession );
	return 0;
}


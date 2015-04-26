#include "rserver.h"

int app_WorkerUnregister( struct ServerEnv *penv , struct SocketSession *psession );

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
		nret = AsyncReceiveCommand( penv->epoll_socks , psession , 0 ) ;
_GOTO_CHECK_ASYNC_RECEIVE_COMMAND :
		if( nret == RETURN_RECEIVING_IN_PROGRESS )
		{
			InfoLog( __FILE__ , __LINE__ , "AsyncReceiveCommand done" );
		}
		else if( nret == RETURN_PEER_CLOSED )
		{
			DeleteSockFromEpoll( penv->epoll_socks , psession );
			close( psession->sock );
			ResetSocketSession( psession );
			return 0;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveCommand failed[%d] , errno[%d]" , nret , errno );
			DeleteSockFromEpoll( penv->epoll_socks , psession );
			close( psession->sock );
			ResetSocketSession( psession );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "AsyncReceiveCommand ok , call proto" );
			
			nret = proto( penv , psession ) ;
			if( nret == RETURN_CLOSE_PEER )
			{
				InfoLog( __FILE__ , __LINE__ , "proto done , disconnect socket" );
				DeleteSockFromEpoll( penv->epoll_socks , psession );
				close( psession->sock );
				ResetSocketSession( psession );
				return 0;
			}
			else if( nret < 0 )
			{
				ErrorLog( __FILE__ , __LINE__ , "proto return failed[%d]" , nret );
				return -1;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "proto ok" );
			}
			
			nret = AfterDoCommandProtocol( psession ) ;
			if( nret == RETURN_NO_SEND_RESPONSE )
			{
				InfoLog( __FILE__ , __LINE__ , "AfterDoCommandProtocol done" );
				return 0;
			}
			else if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "AfterDoCommandProtocol return failed[%d]" , nret );
				return -1;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "AfterDoCommandProtocol ok" );
				ModifyOutputSockFromEpoll( penv->epoll_socks , psession );
				nret = AsyncReceiveCommand( penv->epoll_socks , psession , OPTION_ASYNC_SKIP_RECV_FLAG ) ;
				goto _GOTO_CHECK_ASYNC_RECEIVE_COMMAND;
			}
		}
	}
	else
	{
		nret = AsyncReceiveSocketData( penv->epoll_socks , psession , OPTION_ASYNC_CHANGE_MODE_FLAG ) ;
		if( nret == RETURN_RECEIVING_IN_PROGRESS )
		{
			InfoLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData ok" );
			return 0;
		}
		else if( nret == RETURN_PEER_CLOSED )
		{
			InfoLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData done" );
			app_WorkerUnregister( penv , psession );
			epoll_ctl( penv->epoll_socks , EPOLL_CTL_DEL , psession->sock , NULL );
			close( psession->sock );
			ResetSocketSession( psession );
			return 0;
		}
		else if( nret == RETURN_CHANGE_COMM_PROTOCOL_MODE )
		{
			nret = AsyncReceiveCommand( penv->epoll_socks , psession , OPTION_ASYNC_SKIP_RECV_FLAG ) ;
			goto _GOTO_CHECK_ASYNC_RECEIVE_COMMAND;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData failed[%d] , errno[%d]" , nret , errno );
			epoll_ctl( penv->epoll_socks , EPOLL_CTL_DEL , psession->sock , NULL );
			close( psession->sock );
			ResetSocketSession( psession );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData ok , call proto" );
			
			nret = proto( penv , psession ) ;
			if( nret == RETURN_CLOSE_PEER )
			{
				InfoLog( __FILE__ , __LINE__ , "proto done , disconnect socket" );
				app_WorkerUnregister( penv , psession );
				epoll_ctl( penv->epoll_socks , EPOLL_CTL_DEL , psession->sock , NULL );
				close( psession->sock );
				ResetSocketSession( psession );
				
				return 0;
			}
			else if( nret < 0 )
			{
				ErrorLog( __FILE__ , __LINE__ , "proto return failed[%d]" , nret );
				return -1;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "proto ok" );
			}
			
			nret = AfterDoProtocol( psession ) ;
			if( nret == RETURN_NO_SEND_RESPONSE )
			{
				InfoLog( __FILE__ , __LINE__ , "AfterDoProtocol done" );
				return 0;
			}
			else if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "AfterDoProtocol return failed[%d]" , nret );
				return -1;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "AfterDoProtocol ok" );
				ModifyOutputSockFromEpoll( penv->epoll_socks , psession );
			}
		}
	}
	
	return 0;
}

int comm_OnAcceptedSocketOutput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int		nret = 0 ;
	
	nret = AsyncSendSocketData( penv->epoll_socks , psession ) ;
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
		CleanSendBuffer( psession );
		ModifyInputSockFromEpoll( penv->epoll_socks , psession );
	}
	
	return 0;
}

int comm_OnAcceptedSocketError( struct ServerEnv *penv , struct SocketSession *psession )
{
	ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error , errno[%d]" , psession->sock , errno );
	app_WorkerUnregister( penv , psession );
	epoll_ctl( penv->epoll_socks , EPOLL_CTL_DEL , psession->sock , NULL );
	close( psession->sock );
	
	return 0;
}


/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 * LastVersion	: v1.0.0
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "server.h"

int comm_AsyncConnectToRegisterServer( struct ServerEnv *penv , struct SocketSession *psession , int skip_connect_flag )
{
	int			rserver_index ;
	
	int			nret = 0 ;
	
	psession->progress = CONNECT_SESSION_PROGRESS_CLOSED ;
	
	if( skip_connect_flag == 0 )
	{
		rserver_index = psession - &(penv->connect_session[0]) ;
		
		nret = AsyncConnectSocket( penv->param.rserver_ip[rserver_index] , penv->param.rserver_port[rserver_index] , psession ) ;
		if( nret == RETURN_CONNECTING_IN_PROGRESS )
		{
			InfoLog( __FILE__ , __LINE__ , "CreateConnectSocket[%s:%d] done , but connecting is progressing" , penv->param.rserver_ip[rserver_index] , penv->param.rserver_port[rserver_index] );
			AddOutputSockToEpoll( penv->epoll_socks , psession );
			return nret;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "CreateConnectSocket[%s:%d] failed[%d]errno[%d]" , penv->param.rserver_ip[rserver_index] , penv->param.rserver_port[rserver_index] , nret , errno );
			return 0;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "CreateConnectSocket[%s:%d] ok" , penv->param.rserver_ip[rserver_index] , penv->param.rserver_port[rserver_index] );
			AddOutputSockToEpoll( penv->epoll_socks , psession );
		}
	}
	
	nret = proto_WorkerRegisterRequest( penv , psession ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_WorkerRegisterRequest failed[%d]errno[%d]" , nret , errno );
		return nret;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "proto_WorkerRegisterRequest ok" );
	}
	
	psession->progress = CONNECT_SESSION_PROGRESS_CONNECTED ;
	
	return 0;
}

int comm_CloseConnectedSocket( struct ServerEnv *penv , struct SocketSession *psession )
{
	DeleteSockFromEpoll( penv->epoll_socks , psession );
	CloseSocket( psession );
	
	psession->progress = CONNECT_SESSION_PROGRESS_CLOSED ;
	
	return 0;
}

static void KillChildProcess( struct ServerEnv *penv , struct SocketSession *psession )
{
	int		nret = 0 ;
	
	if( psession->p1 == & (penv->alive_session) && IsSocketEstablished( & (penv->alive_session) ) )
	{
		nret = kill( penv->pid , SIGTERM ) ;
		InfoLog( __FILE__ , __LINE__ , "kill [%ld] return errno[%d]" , penv->pid , errno );
		
		sleep(2);
		
		nret = kill( penv->pid , SIGKILL ) ;
		InfoLog( __FILE__ , __LINE__ , "kill -9 [%ld] return errno[%d]" , penv->pid , errno );
		
		penv->pid = 0 ;
	}
	
	return;
}

int comm_CloseAcceptedSocket( struct ServerEnv *penv , struct SocketSession *psession )
{
	KillChildProcess( penv , psession );
	
	DeleteSockFromEpoll( penv->epoll_socks , psession );
	CloseSocket( psession );
	
	return 0;
}

int comm_OnConnectedSocketInput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	nret = AsyncReceiveSocketData( psession , 0 ) ;
	if( nret == RETURN_RECEIVING_IN_PROGRESS )
	{
		DebugLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData ok" );
		return 0;
	}
	else if( nret == RETURN_PEER_CLOSED )
	{
		DebugLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData done" );
		comm_CloseConnectedSocket( penv , psession );
		return 0;
	}
	else if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData failed[%d] , errno[%d]" , nret , errno );
		comm_CloseConnectedSocket( penv , psession );
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData ok , call proto" );
		
		nret = proto( penv , psession ) ;
		if( nret < 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "proto return failed[%d]" , nret );
			comm_CloseConnectedSocket( penv , psession );
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
			comm_CloseConnectedSocket( penv , psession );
			return 0;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "AfterDoProtocol ok" );
			ModifyOutputSockFromEpoll( penv->epoll_socks , psession );
		}
	}
	
	return 0;
}

int comm_OnConnectedSocketOutput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	if( psession->progress == CONNECT_SESSION_PROGRESS_CLOSED )
	{
		nret = AsyncCompleteConnectedSocket( psession ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "AsyncCompleteConnectedSocket failed[%d]" , nret );
			comm_CloseConnectedSocket( penv , psession );
			return 0;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "AsyncCompleteConnectedSocket ok" );
			nret = comm_AsyncConnectToRegisterServer( penv , psession , 1 );
			if( nret == RETURN_CONNECTING_IN_PROGRESS )
			{
				InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer done" );
			}
			else if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer failed[%d]" , nret );
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer ok" );
			}
		}
	}
	else
	{
		nret = AsyncSendSocketData( psession ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "AsyncSendSocketData failed[%d] , errno[%d]" , nret , errno );
			comm_CloseConnectedSocket( penv , psession );
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
	}
	
	return 0;
}

int comm_OnConnectedSocketError( struct ServerEnv *penv , struct SocketSession *psession )
{
	ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error , errno[%d]" , psession->sock , errno );
	comm_CloseConnectedSocket( penv , psession );
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
	else if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData failed[%d] , errno[%d]" , nret , errno );
		comm_CloseAcceptedSocket( penv , psession );
		return 0;
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
	
	return 0;
}

int comm_OnAcceptedSocketOutput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
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


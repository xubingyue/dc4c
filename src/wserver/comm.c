#include "wserver.h"

int proto_WorkerNoticeRequest( struct ServerEnv *penv , struct SocketSession *psession );

static void KillChildProcess( struct ServerEnv *penv )
{
	int		nret = 0 ;
	
	if( penv->accepted_progress == 2 )
	{
		nret = kill( penv->pid , SIGTERM ) ;
		InfoLog( __FILE__ , __LINE__ , "kill [%ld] return[%d]errno[%d]" , penv->pid , errno );
		
		sleep(2);
		
		nret = kill( penv->pid , SIGKILL ) ;
		InfoLog( __FILE__ , __LINE__ , "kill -9 [%ld] return[%d]errno[%d]" , penv->pid , errno );
		
		penv->pid = 0 ;
	}
	
	return;
}

static int SendWorkerNotice( struct ServerEnv *penv )
{
	int		nret = 0 ;
	
	if( penv->connect_progress == 2 )
	{
		nret = proto_WorkerNoticeRequest( penv , & (penv->connect_session) ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "proto_WorkerNoticeRequest failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "proto_WorkerNoticeRequest ok" );
			ModifyOutputSockFromEpoll( penv->epoll_socks , & (penv->connect_session) );
		}
	}
	else
	{
		return -2;
	}
	
	return 0;
}

int comm_AsyncConnectToRegisterServer( struct ServerEnv *penv , int skip_connect_flag )
{
	int			nret = 0 ;
	
	penv->connect_progress = 0 ;
	
	if( skip_connect_flag == 0 )
	{
		nret = AsyncConnectSocket( penv->param.rserver_ip , penv->param.rserver_port , & (penv->connect_session) ) ;
		if( nret == RETURN_CONNECTING_IN_PROGRESS )
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
	
	penv->connect_session.established_flag = 1 ;
	
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
	
	penv->connect_progress = 1 ;
	
	return 0;
}

int comm_OnConnectedSocketInput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	nret = AsyncReceiveSocketData( psession , OPTION_ASYNC_CHANGE_MODE_FLAG ) ;
	if( nret == RETURN_RECEIVING_IN_PROGRESS )
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData ok" );
		return 0;
	}
	else if( nret == RETURN_PEER_CLOSED )
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData done" );
_GOTO_RECONNECTING :
		DeleteSockFromEpoll( penv->epoll_socks , psession );
		CloseSocket( psession );
		ResetSocketSession( psession );
		
		penv->connect_progress = 0 ;
		
		nret = comm_AsyncConnectToRegisterServer( penv , 0 ) ;
		if( nret == RETURN_CONNECTING_IN_PROGRESS )
		{
			InfoLog( __FILE__ , __LINE__ , "comm_AsyncConnectToRegisterServer[%s:%d] done , but connecting is progressing" , penv->param.rserver_ip , penv->param.rserver_port );
			return 0;
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
	else if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData failed[%d] , errno[%d]" , nret , errno );
		goto _GOTO_RECONNECTING;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData ok , call proto" );
		
		nret = proto( penv , psession ) ;
		if( nret == RETURN_CLOSE_PEER )
		{
			InfoLog( __FILE__ , __LINE__ , "proto done , disconnect socket" );
			DeleteSockFromEpoll( penv->epoll_socks , psession );
			CloseSocket( psession );
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
	
	return 0;
}

int comm_OnConnectedSocketOutput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	if( penv->connect_progress == 0 )
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
			CloseSocket( psession );
			ResetSocketSession( psession );
			
			sleep(1);
			
			nret = comm_AsyncConnectToRegisterServer( penv , 0 ) ;
			if( nret == RETURN_CONNECTING_IN_PROGRESS )
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
			if( nret == RETURN_CONNECTING_IN_PROGRESS )
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
			CloseSocket( psession );
			ResetSocketSession( psession );
			
			nret = comm_AsyncConnectToRegisterServer( penv , 0 ) ;
			if( nret == RETURN_CONNECTING_IN_PROGRESS )
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
			CleanSendBuffer( psession );
			ModifyInputSockFromEpoll( penv->epoll_socks , psession );
		}
	}
	
	return 0;
}

int comm_OnConnectedSocketError( struct ServerEnv *penv , struct SocketSession *psession )
{
	int		nret = 0 ;
	
	ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error , errno[%d]" , psession->sock , errno );
	DeleteSockFromEpoll( penv->epoll_socks , psession );
	CloseSocket( psession );
	
	penv->connect_progress = 0 ;
	
	sleep(1);
	
	nret = comm_AsyncConnectToRegisterServer( penv , 0 ) ;
	if( nret == RETURN_CONNECTING_IN_PROGRESS )
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

int comm_OnListenSocketInput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	if( penv->accepted_progress != 0 )
	{
		nret = DiscardAcceptSocket( psession->sock ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DiscardAcceptSocket failed[%d] , errno[%d]" , nret , errno );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "DiscardAcceptSocket ok" );
		}
		
		return 0;
	}
	
	nret = AcceptSocket( psession->sock , & (penv->accepted_session) ) ;
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
	
	penv->accepted_progress = 1 ;
	
	SendWorkerNotice( penv );
	
	return 0;
}

int comm_OnAcceptedSocketInput( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	nret = AsyncReceiveSocketData( psession , OPTION_ASYNC_CHANGE_MODE_FLAG ) ;
	if( nret == RETURN_RECEIVING_IN_PROGRESS )
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData ok" );
		return 0;
	}
	else if( nret == RETURN_PEER_CLOSED )
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData done" );
		DeleteSockFromEpoll( penv->epoll_socks , psession );
		CloseSocket( psession );
		ResetSocketSession( psession );
		
		penv->accepted_progress = 0 ;
		
		KillChildProcess( penv );
		SendWorkerNotice( penv );
		
		return 0;
	}
	else if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData failed[%d] , errno[%d]" , nret , errno );
		DeleteSockFromEpoll( penv->epoll_socks , psession );
		CloseSocket( psession );
		ResetSocketSession( psession );
		
		penv->accepted_progress = 0 ;
		
		KillChildProcess( penv );
		SendWorkerNotice( penv );
		
		return 0;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "AsyncReceiveSocketData ok , call proto" );
		
		nret = proto( penv , psession ) ;
		if( nret == RETURN_CLOSE_PEER )
		{
			InfoLog( __FILE__ , __LINE__ , "proto done , disconnect socket" );
			DeleteSockFromEpoll( penv->epoll_socks , psession );
			CloseSocket( psession );
			ResetSocketSession( psession );
			
			penv->accepted_progress = 0 ;
			
			KillChildProcess( penv );
			SendWorkerNotice( penv );
			
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
			DeleteSockFromEpoll( penv->epoll_socks , psession );
			CloseSocket( psession );
			ResetSocketSession( psession );
			
			penv->accepted_progress = 0 ;
			
			KillChildProcess( penv );
			SendWorkerNotice( penv );
			
			return 0;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "AfterDoProtocol ok" );
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
		DeleteSockFromEpoll( penv->epoll_socks , psession );
		CloseSocket( psession );
		ResetSocketSession( psession );
		
		penv->accepted_progress = 0 ;
		
		SendWorkerNotice( penv );
		
		return -1;
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
	DeleteSockFromEpoll( penv->epoll_socks , psession );
	CloseSocket( psession );
	
	penv->accepted_progress = 0 ;
	
	SendWorkerNotice( penv );
	
	return 0;
}


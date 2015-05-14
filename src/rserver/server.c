#include "rserver.h"

int		g_server_exit_flag = 0 ;

static void server_signal_proc( int signum )
{
	if( signum == SIGTERM )
		g_server_exit_flag = 1 ;
	return;
}

int server( struct ServerEnv *penv )
{
	int			i ;
	
	time_t			now ;
	long			epoll_timeout ;
	struct epoll_event	events[ WAIT_EVENTS_COUNT ] ;
	int			epoll_ready_count ;
	int			epoll_ready_index ;
	struct epoll_event	*pevent ;
	struct SocketSession	*psession = NULL ;
	
	int			nret = 0 ;
	
	SetLogFile( "%s/log/rserver_1_%s:%d.log" , getenv("HOME") , penv->param.rserver_ip , penv->param.rserver_port );
	if( penv->param.loglevel_debug == 1 )
		SetLogLevel( LOGLEVEL_DEBUG );
	else
		SetLogLevel( LOGLEVEL_INFO );
	
	penv->epoll_socks = epoll_create( EPOLL_FD_COUNT ) ;
	if( penv->epoll_socks < 0 )
	{
		ErrorLog( __FILE__ , __LINE__ , "epoll_create failed[%d]errno[%d]" , penv->epoll_socks , errno );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "epoll_create ok , sock[%d]" , penv->epoll_socks );
	}
	
	nret = InitSocketSession( & (penv->listen_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
		return -1;
	}
	
	nret = BindListenSocket( penv->param.rserver_ip , penv->param.rserver_port , & (penv->listen_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "BindListenSocket[%s:%d] failed[%d]errno[%d]" , penv->param.rserver_ip , penv->param.rserver_port , nret , errno );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "BindListenSocket[%s:%d] ok" , penv->param.rserver_ip , penv->param.rserver_port );
	}
	
	AddInputSockToEpoll( penv->epoll_socks , & (penv->listen_session) );
	
	penv->accepted_session_array = (struct SocketSession *)malloc( sizeof(struct SocketSession) * MAXCOUNT_ACCEPTED_SESSION ) ;
	if( penv->accepted_session_array == NULL )
	{
		FatalLog( __FILE__ , __LINE__ , "malloc failed[%d]errno[%d]" , nret , errno );
		return -1;
	}
	memset( penv->accepted_session_array , 0x00 , sizeof(struct SocketSession) * MAXCOUNT_ACCEPTED_SESSION );
	for( i = 0 ; i < MAXCOUNT_ACCEPTED_SESSION ; i++ )
	{
		nret = InitSocketSession( penv->accepted_session_array+i ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
			return -1;
		}
	}
	
	signal( SIGTERM , & server_signal_proc );
	
	epoll_timeout = 1 ;
	while( ! g_server_exit_flag )
	{
		memset( events , 0x00 , sizeof(events) );
		epoll_ready_count = epoll_wait( penv->epoll_socks , events , WAIT_EVENTS_COUNT , epoll_timeout * 1000 ) ;
		if( g_server_exit_flag )
			break;
		
		DebugLog( __FILE__ , __LINE__ , "epoll_wait [%d]events reached" , epoll_ready_count );
		
		time( & now );
		
		for( epoll_ready_index = 0 , pevent = & (events[0]) ; epoll_ready_index < epoll_ready_count ; epoll_ready_index++ , pevent++ )
		{
			psession = pevent->data.ptr ;
			psession->alive_timestamp = now ;
			
			if( psession == & (penv->listen_session) )
			{
				if( pevent->events & EPOLLIN )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLIN on listen sock[%d]" , psession->sock );
					
					nret = comm_OnListenSocketInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLOUT )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLOUT on listen sock[%d]" , psession->sock );
					g_server_exit_flag = 1 ;
					break;
				}
				else if( pevent->events & EPOLLERR )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLERR on listen sock[%d]" , psession->sock );
					g_server_exit_flag = 1 ;
					break;
				}
			}
			else
			{
				if( pevent->events & EPOLLIN )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLIN on accepted sock[%d]" , psession->sock );
					
					nret = comm_OnAcceptedSocketInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLOUT )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLOUT on accepted sock[%d]" , psession->sock );
					
					nret = comm_OnAcceptedSocketOutput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLERR )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLERR on accepted sock[%d]" , psession->sock );
					
					nret = comm_OnAcceptedSocketError( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
			}
		}
		
		app_HeartBeatRequest( penv , & now , & epoll_timeout );
	}
	
	close( penv->epoll_socks );
	InfoLog( __FILE__ , __LINE__ , "close all socks and epoll_socks" );
	
	for( i = 0 ; i < MAXCOUNT_ACCEPTED_SESSION ; i++ )
	{
		CleanSocketSession( penv->accepted_session_array+i );
	}
	CleanSocketSession( & (penv->listen_session) );
	
	free( penv->accepted_session_array );
	
	return 0;
}

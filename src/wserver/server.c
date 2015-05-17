/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 * LastVersion	: v1.0.0
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "server.h"

int		g_server_exit_flag = 0 ;

static void server_signal_proc( int signum )
{
	if( signum == SIGTERM )
		g_server_exit_flag = 1 ;
	return;
}

int server( struct ServerEnv *penv )
{
	time_t			now ;
	long			epoll_timeout ;
	struct epoll_event	events[ WAIT_EVENTS_COUNT ] ;
	int			epoll_ready_count ;
	int			epoll_ready_index ;
	struct epoll_event	*pevent ;
	struct SocketSession	*psession = NULL ;
	int			accepted_session_index ;
	int			rserver_index ;
	
	int			nret = 0 ;
	
	SetLogFile( "%s/log/dc4c_wserver_%d_%s:%d.log" , getenv("HOME") , penv->wserver_index+1 , penv->param.wserver_ip , penv->param.wserver_port );
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
	
	nret = BindListenSocket( penv->param.wserver_ip , penv->param.wserver_port , & (penv->listen_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "BindListenSocket[%s:%d] failed[%d]errno[%d]" , penv->param.wserver_ip , penv->param.wserver_port , nret , errno );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "BindListenSocket[%s:%d] ok" , penv->param.wserver_ip , penv->param.wserver_port );
	}
	
	AddInputSockToEpoll( penv->epoll_socks , & (penv->listen_session) );
	
	penv->accepted_session_array = (struct SocketSession *)malloc( sizeof(struct SocketSession) * MAXCOUNT_ACCEPTED_SESSION ) ;
	if( penv->accepted_session_array == NULL )
	{
		FatalLog( __FILE__ , __LINE__ , "malloc failed[%d]errno[%d]" , nret , errno );
		return -1;
	}
	memset( penv->accepted_session_array , 0x00 , sizeof(struct SocketSession) * MAXCOUNT_ACCEPTED_SESSION );
	for( accepted_session_index = 0 ; accepted_session_index < MAXCOUNT_ACCEPTED_SESSION ; accepted_session_index++ )
	{
		nret = InitSocketSession( & (penv->accepted_session_array[accepted_session_index]) ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
			return -1;
		}
	}
	
	for( rserver_index = 0 ; rserver_index < penv->rserver_count ; rserver_index++ )
	{
		nret = InitSocketSession( & (penv->connect_session[rserver_index]) ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
			return -1;
		}
		
		nret = comm_AsyncConnectToRegisterServer( penv , & (penv->connect_session[rserver_index]) , 0 ) ;
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
	
	signal( SIGTERM , & server_signal_proc );
	
	epoll_timeout = 1 ;
	while( ! g_server_exit_flag )
	{
		memset( events , 0x00 , sizeof(events) );
		epoll_ready_count = epoll_wait( penv->epoll_socks , events , WAIT_EVENTS_COUNT , epoll_timeout*1000 ) ;
		if( g_server_exit_flag )
			break;
		
		for( rserver_index = 0 ; rserver_index < penv->rserver_count ; rserver_index++ )
		{
			if( IsSocketClosed( & (penv->connect_session[rserver_index]) ) )
			{
				nret = comm_AsyncConnectToRegisterServer( penv , & (penv->connect_session[rserver_index]) , 0 ) ;
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
		
		DebugLog( __FILE__ , __LINE__ , "epoll_wait [%d]events reached" , epoll_ready_count );
		
		time( & now );
		
		for( epoll_ready_index = 0 , pevent = & (events[0]) ; epoll_ready_index < epoll_ready_count ; epoll_ready_index++ , pevent++ )
		{
			psession = pevent->data.ptr ;
			DebugLog( __FILE__ , __LINE__ , "psession[%p] pevent->events[%d]" , psession , pevent->events );
			
			if( psession == & (penv->listen_session) )
			{
				if( pevent->events & EPOLLIN || pevent->events & EPOLLHUP )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLIN on listen sock[%d]" , psession->sock );
					
					nret = comm_OnListenSocketInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
					break;
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
			else if( & (penv->connect_session[0]) <= psession && psession <= & (penv->connect_session[RSERVER_ARRAYSIZE-1]) )
			{
				if( pevent->events & EPOLLIN || pevent->events & EPOLLHUP )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLIN on connected sock[%d]" , psession->sock );
					
					nret = comm_OnConnectedSocketInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLOUT )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLOUT on connected sock[%d]" , psession->sock );
					
					nret = comm_OnConnectedSocketOutput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLERR )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLERR on connected sock[%d]" , psession->sock );
					
					nret = comm_OnConnectedSocketError( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
			}
			else if( psession == & (penv->info_session) )
			{
				if( pevent->events & EPOLLIN || pevent->events & EPOLLHUP )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLIN on info pipe[%d]" , psession->sock );
					
					nret = comm_OnInfoPipeInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLOUT )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLOUT on info pipe[%d]" , psession->sock );
					
					nret = comm_OnInfoPipeError( penv , psession ) ;
					if( nret < 0 )
						return nret;
				}
				else if( pevent->events & EPOLLERR )
				{
					DebugLog( __FILE__ , __LINE__ , "EPOLLERR on info pipe[%d]" , psession->sock );
					
					nret = comm_OnInfoPipeError( penv , psession ) ;
					if( nret < 0 )
						return nret;
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
				else if( pevent->events & EPOLLERR || pevent->events & EPOLLHUP )
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
	
	DeleteSockFromEpoll( penv->epoll_socks , & (penv->listen_session) );
	CloseSocket( & (penv->listen_session) );
	CleanSocketSession( & (penv->listen_session) );
	
	for( rserver_index = 0 ; rserver_index < penv->rserver_count ; rserver_index++ )
	{
		DeleteSockFromEpoll( penv->epoll_socks , & (penv->connect_session[rserver_index]) );
		CloseSocket( & (penv->connect_session[rserver_index]) );
		CleanSocketSession( & (penv->connect_session[rserver_index]) );
	}
	
	close( penv->epoll_socks );
	InfoLog( __FILE__ , __LINE__ , "close all socks and epoll_socks" );
	
	return 0;
}

#include "wserver.h"

int proto_ExecuteProgramResponse( struct ServerEnv *penv , struct SocketSession *psession , int response_code , int status );
int app_WaitProgramExiting( struct ServerEnv *penv , struct SocketSession *psession );

int		g_exit_flag = 0 ;

static void signal_proc( int signum )
{
	if( signum == SIGTERM )
		g_exit_flag = 1 ;
	return;
}

int server( struct ServerEnv *penv )
{
	struct epoll_event	events[ WAIT_EVENTS_COUNT ] ;
	int			epoll_ready_count ;
	int			epoll_ready_index ;
	struct epoll_event	*pevent ;
	struct SocketSession	*psession = NULL ;
	
	int			nret = 0 ;
	
	signal( SIGTERM , & signal_proc );
	signal( SIGCLD , SIG_DFL );
	signal( SIGCHLD , SIG_DFL );
	signal( SIGPIPE , SIG_IGN );
	
	while( ! g_exit_flag )
	{
		memset( events , 0x00 , sizeof(events) );
		epoll_ready_count = epoll_wait( penv->epoll_socks , events , WAIT_EVENTS_COUNT , -1 ) ;
		if( g_exit_flag )
			break;
		
		InfoLog( __FILE__ , __LINE__ , "epoll_wait [%d]events reached" , epoll_ready_count );
		
		for( epoll_ready_index = 0 , pevent = & (events[0]) ; epoll_ready_index < epoll_ready_count ; epoll_ready_index++ , pevent++ )
		{
			psession = pevent->data.ptr ;
DebugLog( __FILE__ , __LINE__ , "psession[%p] pevent->events[%X] EPOLLIN[%X] EPOLLOUT[%X] EPOLLERR[%X]" , psession , pevent->events , EPOLLIN , EPOLLOUT , EPOLLERR );
			
			if( psession == & (penv->listen_session) )
			{
				if( pevent->events & EPOLLIN )
				{
					InfoLog( __FILE__ , __LINE__ , "EPOLLIN on listen sock[%d]" , psession->sock );
					
					nret = comm_OnListenSocketInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
					else if( nret > 0 )
						continue;
				}
				else if( pevent->events & EPOLLOUT )
				{
					InfoLog( __FILE__ , __LINE__ , "EPOLLOUT on listen sock[%d]" , psession->sock );
				}
				else if( pevent->events & EPOLLERR )
				{
					InfoLog( __FILE__ , __LINE__ , "EPOLLERR on listen sock[%d]" , psession->sock );
					
					g_exit_flag = 1 ;
					continue;
				}
			}
			else if( psession == & (penv->connect_session) )
			{
				if( pevent->events & EPOLLIN )
				{
					InfoLog( __FILE__ , __LINE__ , "EPOLLIN on connected sock[%d]" , psession->sock );
					
					nret = comm_OnConnectedSocketInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
					else if( nret > 0 )
						continue;
				}
				else if( pevent->events & EPOLLOUT )
				{
					InfoLog( __FILE__ , __LINE__ , "EPOLLOUT on connected sock[%d]" , psession->sock );
					
					nret = comm_OnConnectedSocketOutput( penv , psession ) ;
					if( nret < 0 )
						return nret;
					else if( nret > 0 )
						continue;
				}
				else if( pevent->events & EPOLLERR )
				{
					InfoLog( __FILE__ , __LINE__ , "EPOLLERR on connected sock[%d]" , psession->sock );
					
					nret = comm_OnConnectedSocketError( penv , psession ) ;
					if( nret < 0 )
						return nret;
					else if( nret > 0 )
						continue;
				}
			}
			else if( psession == & (penv->exit_session) )
			{
				/* 为什么这时候的pevent->events是16(0x10)？ */
				InfoLog( __FILE__ , __LINE__ , "EPOLL?? on exit sock[%d]" , psession->sock );
				
				nret = app_WaitProgramExiting( penv , psession ) ;
				if( nret < 0 )
					return nret;
				else if( nret > 0 )
					continue;
			}
			else
			{
				if( pevent->events & EPOLLIN )
				{
					InfoLog( __FILE__ , __LINE__ , "EPOLLIN on accepted sock[%d]" , psession->sock );
					
					nret = comm_OnAcceptedSocketInput( penv , psession ) ;
					if( nret < 0 )
						return nret;
					else if( nret > 0 )
						continue;
				}
				else if( pevent->events & EPOLLOUT )
				{
					InfoLog( __FILE__ , __LINE__ , "EPOLLOUT on accepted sock[%d]" , psession->sock );
					
					nret = comm_OnAcceptedSocketOutput( penv , psession ) ;
					if( nret < 0 )
						return nret;
					else if( nret > 0 )
						continue;
				}
				else if( pevent->events & EPOLLERR )
				{
					InfoLog( __FILE__ , __LINE__ , "EPOLLERR on accepted sock[%d]" , psession->sock );
					
					nret = comm_OnAcceptedSocketError( penv , psession ) ;
					if( nret < 0 )
						return nret;
					else if( nret > 0 )
						continue;
				}
			}
		}
	}
	
	return 0;
}

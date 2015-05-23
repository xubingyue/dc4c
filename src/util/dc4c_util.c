/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "dc4c_util.h"

char __DC4C_UTIL_VERSION_1_0_0[] = "1.0.0" ;
char *__DC4C_UTIL_VERSION = __DC4C_UTIL_VERSION_1_0_0 ;

int ConvertToDaemonServer()
{
	int pid;
	int sig,fd;
	
	pid=fork();
	
	switch( pid )
	{
		case -1:
			return -1;
		case 0:
			break;
		default		:
			exit( 0 );	
			break;
	}

	setsid() ;
	signal( SIGHUP,SIG_IGN );

	pid=fork();

	switch( pid )
	{
		case -1:
			return -1;
		case 0:
			break ;
		default:
			exit( 0 );
			break;
	}
	
	setuid( getpid() ) ;
	
	chdir("/tmp");
	
	umask( 0 ) ;
	
	for( sig=0 ; sig<30 ; sig++ )
		signal( sig , SIG_IGN );
	
	signal( SIGTERM , SIG_DFL );
	
	for( fd=0 ; fd<=2 ; fd++ )
		close( fd );
	
	return 0;
}

int ns2i( char *str , int len )
{
	char	buf[ 10 + 1 ] ;
	
	memset( buf , 0x00 , sizeof(buf) );
	strncpy( buf , str , len );
	
	return atoi( buf ) ;
}

void CleanSocketSession( struct SocketSession *psession )
{
	if( psession )
	{
		CloseSocket( psession );
		
		if( psession->recv_buffer )
		{
			free( psession->recv_buffer );
			psession->recv_buffer = NULL ;
		}
		
		if( psession->send_buffer )
		{
			free( psession->send_buffer );
			psession->send_buffer = NULL ;
		}
	}
	
	return;
}

int InitSocketSession( struct SocketSession *psession )
{
	memset( psession , 0x00 , sizeof(struct SocketSession) );
	
	psession->recv_buffer_size = 1024+1 ;
	psession->recv_buffer = (char*) malloc( psession->recv_buffer_size ) ;
	if( psession->recv_buffer == NULL )
	{
		CleanSocketSession( psession );
		return -1;
	}
	CleanRecvBuffer( psession );
	
	psession->send_buffer_size = 1024+1 ;
	psession->send_buffer = (char*) malloc( psession->send_buffer_size ) ;
	if( psession->send_buffer == NULL )
	{
		CleanSocketSession( psession );
		return -1;
	}
	CleanSendBuffer( psession );
	
	return 0;
}

void FreeSocketSession( struct SocketSession *psession )
{
	CleanSocketSession( psession );
	
	free( psession );
	
	return;
}

struct SocketSession *AllocSocketSession()
{
	struct SocketSession	*psession = NULL ;
	
	int			nret = 0 ;
	
	psession = (struct SocketSession *)malloc( sizeof(struct SocketSession) ) ;
	if( psession == NULL )
		return NULL;
	
	nret = InitSocketSession( psession ) ;
	if( nret )
	{
		FreeSocketSession( psession );
		return NULL;
	}
	
	return psession;
}

void CleanSendBuffer( struct SocketSession *psession )
{
	memset( psession->send_buffer , 0x00 , psession->send_buffer_size );
	psession->total_send_len = 0 ;
	psession->send_len = 0 ;
	
	return;
}

void CleanRecvBuffer( struct SocketSession *psession )
{
	memset( psession->recv_buffer , 0x00 , psession->recv_buffer_size );
	psession->total_recv_len = 0 ;
	psession->recv_body_len = 0 ;
	
	return;
}

int ReallocSendBuffer( struct SocketSession *psession , int new_send_buffer_size )
{
	char		*new_send_buffer = NULL ;
	
	if( new_send_buffer_size <= psession->send_buffer_size )
		return 0;
	
	new_send_buffer = (char*) realloc( psession->send_buffer , new_send_buffer_size ) ;
	if( new_send_buffer == NULL )
		return -1;
	// memset( psession->send_buffer + psession->total_send_len , 0x00 , new_send_buffer_size - psession->total_send_len );
	
	psession->send_buffer = new_send_buffer ;
	psession->send_buffer_size = new_send_buffer_size ;
	
	return 0;
}

int ReallocRecvBuffer( struct SocketSession *psession , int new_recv_buffer_size )
{
	char		*new_recv_buffer = NULL ;
	
	if( new_recv_buffer_size <= psession->recv_buffer_size )
		return 0;
	
	new_recv_buffer = (char*) realloc( psession->recv_buffer , new_recv_buffer_size ) ;
	if( new_recv_buffer == NULL )
		return -1;
	
	psession->recv_buffer = new_recv_buffer ;
	psession->recv_buffer_size = new_recv_buffer_size ;
	
	return 0;
}

struct SocketSession *GetUnusedSocketSession( struct SocketSession *p_session_array , int count , struct SocketSession **pp_slibing_session )
{
	struct SocketSession	*p_last_session = NULL ;
	int			i ;
	
	p_last_session = p_session_array + count - 1 ;
	
	for( i = 0 ; i < count ; i++ )
	{
		if( (*pp_slibing_session) == NULL )
		{
			(*pp_slibing_session) = p_session_array+0 ;
		}
		else
		{
			(*pp_slibing_session)++;
			if( (*pp_slibing_session) > p_last_session )
				(*pp_slibing_session) = p_session_array+0 ;
		}
		
		if( (*pp_slibing_session)->ip[0] == 0 )
		{
			return (*pp_slibing_session);
		}
	}
	
	return NULL;
}

int SetAddrReused( int sock )
{
	int	on ;
	
	on = 1 ;
	setsockopt( sock , SOL_SOCKET , SO_REUSEADDR , (void *) & on, sizeof(on) );
	DebugLog( __FILE__ , __LINE__ , "set sock[%d] addr reused" , sock );
	
	return 0;
}

int SetNonBlocking( int sock )
{
	int	opts;
	
	opts = fcntl( sock , F_GETFL ) ;
	if( opts < 0 )
	{
		return -1;
	}
	
	opts = opts | O_NONBLOCK;
	if( fcntl( sock , F_SETFL , opts ) < 0 )
	{
		return -2;
	}
	
	DebugLog( __FILE__ , __LINE__ , "set sock[%d] nonblock" , sock );
	
	return 0;
}

void SetSocketAddr( struct sockaddr_in *p_sockaddr , char *ip , long port )
{
	memset( p_sockaddr , 0x00 , sizeof(struct sockaddr_in) );
	p_sockaddr->sin_family = AF_INET ;
	p_sockaddr->sin_addr.s_addr = inet_addr( ip ) ;
	p_sockaddr->sin_port = htons( (unsigned short)port );
	return;
}

void GetSocketAddr( struct sockaddr_in *addr , char *ip , long *port )
{
	if( ip )
	{
#ifdef inet_ntop
		inet_ntop( AF_INET , addr->sin_addr , ip , MAXLEN_SERVER_IP + 1 );
#else
		strcpy( ip , inet_ntoa( addr->sin_addr ) );
#endif
	}
	
	if( port )
	{
		(*port) = (long)ntohs( addr->sin_port ) ;
	}
	
	return;
}

int BindListenSocket( char *ip , long port , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	psession->sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( psession->sock == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "socket failed[%d]errno[%d]" , psession->sock , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "socket ok , sock[%d]" , psession->sock );
	}
	
	SetAddrReused( psession->sock );
	
	strcpy( psession->ip , ip );
	psession->port = port ;
	SetSocketAddr( & (psession->addr) , ip , port );
	nret = bind( psession->sock , (struct sockaddr *) & (psession->addr) , sizeof(struct sockaddr) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "bind[%s:%d] failed[%d]errno[%d]" , ip , port , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "bind[%s:%d] ok" , ip , port );
	}
	
	nret = listen( psession->sock , MAXCNT_LISTEN_BAKLOG ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "listen[%s:%d] failed[%d]errno[%d]" , ip , port , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "listen[%s:%d] ok" , ip , port );
	}
	
	SetSocketEstablished( psession );
	
	time( & (psession->alive_timestamp) );
	time( & (psession->active_timestamp) );
	
	return 0;
}

int AcceptSocket( int listen_sock , struct SocketSession *psession )
{
	socklen_t		addr_len ;
	
	memset( & (psession->addr) , 0x00 , sizeof(struct sockaddr_in) );
	addr_len = sizeof(struct sockaddr_in) ;
	psession->sock = accept( listen_sock , (struct sockaddr *) & (psession->addr) , & addr_len ) ;
	if( psession->sock == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "[%d]accept failed , errno[%d]" , listen_sock , errno );
		return -1;
	}
	else
	{
		GetSocketAddr( & (psession->addr) , psession->ip , & (psession->port) );
		InfoLog( __FILE__ , __LINE__ , "[%d]accept[%d] ok , from[%s:%d]" , listen_sock , psession->sock , psession->ip , psession->port );
	}
	
	SetSocketEstablished( psession );
	
	time( & (psession->alive_timestamp) );
	time( & (psession->active_timestamp) );
	
	return 0;
}

int AsyncConnectSocket( char *ip , long port , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	psession->sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( psession->sock == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "socket failed[%d]errno[%d]" , psession->sock , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "socket ok , sock[%d]" , psession->sock );
	}
	
	SetNonBlocking( psession->sock );
	
	strcpy( psession->ip , ip );
	psession->port = port ;
	SetSocketAddr( & (psession->addr) , psession->ip , psession->port );
	SetSocketOpened( psession );
	nret = connect( psession->sock , (struct sockaddr *) & (psession->addr) , sizeof(struct sockaddr) ) ;
	if( nret )
	{
		if( errno == EINPROGRESS )
		{
			DebugLog( __FILE__ , __LINE__ , "connect2[%s:%d] ok" , psession->ip , psession->port );
			return RETURN_CONNECTING_IN_PROGRESS;
		}
		else
		{
			ErrorLog( __FILE__ , __LINE__ , "connect[%s:%d] failed[%d]errno[%d]" , psession->ip , psession->port , psession->sock , errno );
			CloseSocket( psession );
			return -1;
		}
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "connect[%s:%d] ok" , psession->ip , psession->port );
	}
	
	SetSocketEstablished( psession );
	
	time( & (psession->alive_timestamp) );
	time( & (psession->active_timestamp) );
	
	return 0;
}

int AsyncCompleteConnectedSocket( struct SocketSession *psession )
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
		return -1;
	
	time( & (psession->alive_timestamp) );
	time( & (psession->active_timestamp) );
	
	return 0;
}

int DiscardAcceptSocket( int listen_sock )
{
	struct SocketSession	session ;
	socklen_t		addr_len ;
	
	memset( & session , 0x00 , sizeof(struct SocketSession) );
	addr_len = sizeof(struct sockaddr_in) ;
	session.sock = accept( listen_sock , (struct sockaddr *) & (session.addr) , & addr_len ) ;
	if( session.sock == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "[%d]accept failed , errno[%d]" , listen_sock , errno );
		return -1;
	}
	else
	{
		GetSocketAddr( & (session.addr) , session.ip , & (session.port) );
		InfoLog( __FILE__ , __LINE__ , "[%d]accept[%d] ok , from[%s:%d]" , listen_sock , session.sock , session.ip , session.port );
	}
	
	SetSocketEstablished( & session );
	
	InfoLog( __FILE__ , __LINE__ , "but discard accepted sock[%d]" , session.sock );
	CloseSocket( & session );
	
	return 0;
}

void CloseSocket( struct SocketSession *psession )
{
	if( IsSocketOpened(psession) )
	{
		memset( psession->ip , 0x00 , sizeof(psession->ip) );
		psession->port = 0 ;
		InfoLog( __FILE__ , __LINE__ , "close sock[%d]" , psession->sock );
		close( psession->sock );
		memset( & (psession->addr) , 0x00 , sizeof(psession->addr) );
		SetSocketClosed( psession );
		
		psession->type = 0 ;
		psession->comm_protocol_mode = 0 ;
		psession->progress = 0 ;
		
		psession->alive_timestamp = 0 ;
		psession->active_timestamp = 0 ;
		psession->alive_timeout = 0 ;
		psession->heartbeat_lost_count = 0 ;
		
		psession->ch = '\0' ;
		psession->p1 = NULL ;
		psession->p2 = NULL ;
		psession->p3 = NULL ;
	}
	
	return;
}

void SetSocketClosed( struct SocketSession *psession )
{
	psession->established_flag = 0 ;
	return;
}

void SetSocketOpened( struct SocketSession *psession )
{
	psession->established_flag = 2 ;
	return;
}

void SetSocketEstablished( struct SocketSession *psession )
{
	psession->established_flag = 1 ;
	return;
}

int IsSocketClosed( struct SocketSession *psession )
{
	return psession->established_flag==0?1:0;
}

int IsSocketOpened( struct SocketSession *psession )
{
	return psession->established_flag?1:0;
}

int IsSocketEstablished( struct SocketSession *psession )
{
	return psession->established_flag==1?1:0;
}

int AsyncReceiveSocketData( struct SocketSession *psession , int change_mode_flag )
{
	int		len ;
	
	int		nret = 0 ;
	
	if( psession->recv_body_len == 0 )
	{
		len = (int)recv( psession->sock , psession->recv_buffer + psession->total_recv_len , LEN_COMMHEAD , 0 ) ;
	}
	else
	{
		len = (int)recv( psession->sock , psession->recv_buffer + psession->total_recv_len , LEN_COMMHEAD + psession->recv_body_len - psession->total_recv_len , 0 ) ;
	}
	time( & (psession->active_timestamp) );
	if( len < 0 )
	{
		if( _ERRNO == _EWOULDBLOCK )
			return RETURN_RECEIVING_IN_PROGRESS;
		
		ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on receiving data" , psession->sock );
		return -1;
	}
	else if( len == 0 )
	{
		if( psession->total_recv_len == 0 )
		{
			InfoLog( __FILE__ , __LINE__ , "detected sock[%d] closed on waiting for first byte" , psession->sock );
			return RETURN_PEER_CLOSED;
		}
		else
		{
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] closed on receiving data" , psession->sock );
			return -1;
		}
	}
	else
	{
		int	short_len ;
		
		short_len = len ;
		if( short_len > 4096 )
			short_len = 4096 ;
		DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer + psession->total_recv_len , short_len , "received sock[%d] [%d]bytes" , psession->sock , len );
		
		psession->total_recv_len += len ;
	}
	
	if( change_mode_flag == OPTION_ASYNC_CHANGE_MODE_FLAG && psession->comm_protocol_mode == 0 )
	{
		if( '0' <= psession->recv_buffer[0] && psession->recv_buffer[0] <= '9' )
		{
			psession->comm_protocol_mode = COMMPROTO_PRELEN ;
		}
		else
		{
			psession->comm_protocol_mode = COMMPROTO_LINE ;
			return RETURN_CHANGE_COMM_PROTOCOL_MODE;
			
		}
	}
	
	if( psession->recv_body_len == 0 )
	{
		if( psession->total_recv_len >= LEN_COMMHEAD )
		{
			psession->recv_body_len = ns2i( psession->recv_buffer , LEN_COMMHEAD ) ;
			DebugLog( __FILE__ , __LINE__ , "calc body len[%d]" , psession->recv_body_len );
			if( psession->recv_body_len <= 0 )
			{
				ErrorLog( __FILE__ , __LINE__ , "sock[%d] body len invalid[%d]" , psession->sock , psession->recv_body_len );
				return -1;
			}
			
			if( LEN_COMMHEAD + psession->recv_body_len > psession->recv_buffer_size-1 )
			{
				nret = ReallocRecvBuffer( psession , LEN_COMMHEAD + psession->recv_body_len + 1 ) ;
				if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "ReallocRecvBuffer ->[%d]bytes failed[%ld] , errno[%d]" , LEN_COMMHEAD + psession->recv_body_len + 1 , nret , errno );
					return -1;
				}
				else
				{
					DebugLog( __FILE__ , __LINE__ , "ReallocRecvBuffer ->[%d]bytes ok" , psession->recv_buffer_size );
				}
			}
		}
	}
	
	if( psession->recv_body_len > 0 )
	{
		if( psession->total_recv_len == LEN_COMMHEAD + psession->recv_body_len )
		{
			psession->ch = psession->recv_buffer[ LEN_COMMHEAD + psession->recv_body_len ] ;
			psession->recv_buffer[ LEN_COMMHEAD + psession->recv_body_len ] = '\0' ;
			return 0;
		}
	}
	
	return RETURN_RECEIVING_IN_PROGRESS;
}

int AfterDoProtocol( struct SocketSession *psession )
{
	if( psession->total_recv_len == LEN_COMMHEAD + psession->recv_body_len )
	{
		CleanRecvBuffer( psession );
		DebugLog( __FILE__ , __LINE__ , "clean recv buffer" );
	}
	else
	{
		memmove( psession->recv_buffer , psession->recv_buffer + LEN_COMMHEAD + psession->recv_body_len , psession->total_recv_len - LEN_COMMHEAD - psession->recv_body_len );
		psession->recv_buffer[0] = psession->ch ;
		psession->total_recv_len -= LEN_COMMHEAD + psession->recv_body_len ;
		psession->recv_body_len = 0 ;
		DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer , psession->total_recv_len , "recv buffer remain [%d]bytes" , psession->total_recv_len );
	}
	
	if( psession->total_send_len == 0 )
	{
		DebugLog( __FILE__ , __LINE__ , "proto return ok , but no data need sending" );
		return RETURN_NO_SEND_RESPONSE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "proto return ok , sock[%d] convert to wait EPOLLOUT event" , psession->sock );
		return 0;
	}
}

int AsyncReceiveCommand( struct SocketSession *psession , int skip_recv_flag )
{
	int		len ;
	
	if( ! ( skip_recv_flag == OPTION_ASYNC_SKIP_RECV_FLAG ) )
	{
		len = (int)recv( psession->sock , psession->recv_buffer + psession->total_recv_len , psession->recv_buffer_size-1 - psession->total_recv_len , 0 ) ;
		time( & (psession->active_timestamp) );
		if( len < 0 )
		{
			if( _ERRNO == _EWOULDBLOCK )
				return RETURN_RECEIVING_IN_PROGRESS;
			
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on receiving data" , psession->sock );
			return -1;
		}
		else if( len == 0 )
		{
			if( psession->total_recv_len == 0 )
			{
				InfoLog( __FILE__ , __LINE__ , "detected sock[%d] closed on waiting for first byte" , psession->sock );
				return RETURN_PEER_CLOSED;
			}
			else
			{
				ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] closed on receiving data" , psession->sock );
				return -1;
			}
		}
		else
		{
			int	short_len ;
			
			short_len = len ;
			if( short_len > 4096 )
				short_len = 4096 ;
			DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer + psession->total_recv_len , short_len , "received sock[%d] [%d]bytes" , psession->sock , len );
			
			psession->total_recv_len += len ;
		}
	}
	
	psession->p1 = strchr( psession->recv_buffer , '\n' ) ;
	if( psession->p1 == NULL )
		return RETURN_RECEIVING_IN_PROGRESS;
	
	*(char*)(psession->p1) = '\0' ;
	
	return 0;
}
	
int AfterDoCommandProtocol( struct SocketSession *psession )
{
	if( *((char*)(psession->p1)+1) == '\0' )
	{
		CleanRecvBuffer( psession );
		DebugLog( __FILE__ , __LINE__ , "clean recv buffer" );
	}
	else
	{
		/* "command1\ncommand2\n" */
		int	len ;
		len = strlen((char*)(psession->p1)+1) ;
		strcpy( psession->recv_buffer , (char*)(psession->p1)+1 );
		psession->total_recv_len = len ;
		DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer , psession->total_recv_len , "recv buffer remain [%d]bytes" , psession->total_recv_len );
	}
	
	if( psession->total_send_len == 0 )
	{
		DebugLog( __FILE__ , __LINE__ , "proto return ok , but no data need send" );
		return RETURN_NO_SEND_RESPONSE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "proto return ok , sock[%d] convert to wait EPOLLOUT event" , psession->sock );
		return 0;
	}
}

int AsyncSendSocketData( struct SocketSession *psession )
{
	int			len ;
	
	len = (int)send( psession->sock , psession->send_buffer + psession->send_len , psession->total_send_len - psession->send_len , 0 ) ;
	time( & (psession->active_timestamp) );
	if( len < 0 )
	{
		if( _ERRNO == _EWOULDBLOCK )
			return 0;
		
		ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on sending data" , psession->sock );
		return -1;
	}
	else
	{
		int	short_len ;
		
		short_len = len ;
		if( short_len > 4096 )
			short_len = 4096 ;
		DebugHexLog( __FILE__ , __LINE__ , psession->send_buffer + psession->send_len , short_len , "sended sock[%d] [%d]bytes" , psession->sock , len );
		
		psession->send_len += len ;
	}
	
	return 0;
}

int SyncConnectSocket( char *ip , long port , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	psession->sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( psession->sock == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "socket failed[%d]errno[%d]" , psession->sock , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "socket ok , sock[%d]" , psession->sock );
	}
	
	strcpy( psession->ip , ip );
	psession->port = port ;
	SetSocketAddr( & (psession->addr) , psession->ip , psession->port );
	SetSocketOpened( psession );
	nret = connect( psession->sock , (struct sockaddr *) & (psession->addr) , sizeof(struct sockaddr) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "connect[%s:%d] failed[%d]errno[%d]" , psession->ip , psession->port , psession->sock , errno );
		CloseSocket( psession );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "connect[%s:%d] ok" , psession->ip , psession->port );
	}
	
	SetSocketEstablished( psession );
	
	time( & (psession->alive_timestamp) );
	time( & (psession->active_timestamp) );
	
	return 0;
}

int SyncReceiveSocketData( struct SocketSession *psession , int *p_timeout )
{
	fd_set			read_fds , expt_fds ;
	struct timeval		tv ;
	time_t			tt1 , tt2 ;
	int			len ;
	
	int			nret = 0 ;
	
	CleanRecvBuffer( psession );
	
	while( psession->total_recv_len < 8 )
	{
		time( & tt1 );
		
		FD_ZERO( & read_fds );
		FD_ZERO( & expt_fds );
		FD_SET( psession->sock , & read_fds );
		FD_SET( psession->sock , & expt_fds );
		tv.tv_sec = (*p_timeout) ;
		tv.tv_usec = 0 ;
		nret = select( psession->sock+1 , & read_fds , NULL , & expt_fds , & tv ) ;
		if( nret == 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "send timeout when selecting" );
			return DC4C_ERROR_TIMETOUT;
		}
		
		if( FD_ISSET( psession->sock , & expt_fds ) )
		{
			ErrorLog( __FILE__ , __LINE__ , "expection when selecting" );
			return DC4C_ERROR_SOCKET_EXPECTION;
		}
		
		time( & tt2 );
		(*p_timeout) -= ( tt2 - tt1 ) ;
		if( (*p_timeout) <= 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "send timeout after select" );
			return DC4C_ERROR_TIMETOUT;
		}
		
		len = (int)recv( psession->sock , psession->recv_buffer + psession->total_recv_len , 8 - psession->total_recv_len , 0 ) ;
		time( & (psession->active_timestamp) );
		if( len < 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on receiving data" , psession->sock );
			return -1;
		}
		else if( len == 0 )
		{
			if( psession->total_recv_len == 0 )
			{
				InfoLog( __FILE__ , __LINE__ , "detected sock[%d] close on waiting for first byte" , psession->sock );
				return RETURN_PEER_CLOSED;
			}
			else
			{
				ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] close on receiving data" , psession->sock );
				return -1;
			}
		}
		else
		{
			int	short_len ;
			
			short_len = len ;
			if( short_len > 4096 )
				short_len = 4096 ;
			DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer + psession->total_recv_len , short_len , "received sock[%d] [%d]bytes" , psession->sock , len );
			
			psession->total_recv_len += len ;
		}
	}
	
	psession->recv_body_len = ns2i( psession->recv_buffer , LEN_COMMHEAD ) ;
	
	while( psession->total_recv_len < 8 + psession->recv_body_len )
	{
		time( & tt1 );
		
		FD_ZERO( & read_fds );
		FD_ZERO( & expt_fds );
		FD_SET( psession->sock , & read_fds );
		FD_SET( psession->sock , & expt_fds );
		tv.tv_sec = (*p_timeout) ;
		tv.tv_usec = 0 ;
		nret = select( psession->sock+1 , & read_fds , NULL , & expt_fds , & tv ) ;
		if( nret == 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "send timeout when selecting" );
			return DC4C_ERROR_TIMETOUT;
		}
		
		if( FD_ISSET( psession->sock , & expt_fds ) )
		{
			ErrorLog( __FILE__ , __LINE__ , "expection when selecting" );
			return DC4C_ERROR_SOCKET_EXPECTION;
		}
		
		time( & tt2 );
		(*p_timeout) -= ( tt2 - tt1 ) ;
		if( (*p_timeout) <= 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "send timeout after select" );
			return DC4C_ERROR_TIMETOUT;
		}
		
		len = (int)recv( psession->sock , psession->recv_buffer + psession->total_recv_len , 8 + psession->recv_body_len - psession->total_recv_len , 0 ) ;
		time( & (psession->active_timestamp) );
		if( len < 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on receiving data" , psession->sock );
			return -1;
		}
		else if( len == 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] close on receiving data" , psession->sock );
			return -1;
		}
		else
		{
			int	short_len ;
			
			short_len = len ;
			if( short_len > 4096 )
				short_len = 4096 ;
			DebugHexLog( __FILE__ , __LINE__ , psession->recv_buffer + psession->total_recv_len , short_len , "received sock[%d] [%d]bytes" , psession->sock , len );
			
			psession->total_recv_len += len ;
		}
	}
	
	return 0;
}

int SyncSendSocketData( struct SocketSession *psession , int *p_timeout )
{
	fd_set			read_fds , expt_fds ;
	struct timeval		tv ;
	time_t			tt1 , tt2 ;
	int			len ;
	
	int			nret = 0 ;
	
	while( psession->send_len < psession->total_send_len )
	{
		time( & tt1 );
		
		FD_ZERO( & read_fds );
		FD_ZERO( & expt_fds );
		FD_SET( psession->sock , & read_fds );
		FD_SET( psession->sock , & expt_fds );
		tv.tv_sec = (*p_timeout) ;
		tv.tv_usec = 0 ;
		nret = select( psession->sock+1 , NULL , & read_fds , & expt_fds , & tv ) ;
		if( nret == 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "send timeout when selecting" );
			return DC4C_ERROR_TIMETOUT;
		}
		
		if( FD_ISSET( psession->sock , & expt_fds ) )
		{
			ErrorLog( __FILE__ , __LINE__ , "expection when selecting" );
			return DC4C_ERROR_SOCKET_EXPECTION;
		}
		
		time( & tt2 );
		(*p_timeout) -= ( tt2 - tt1 ) ;
		if( (*p_timeout) <= 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "send timeout after select" );
			return DC4C_ERROR_TIMETOUT;
		}
		
		len = (int)send( psession->sock , psession->send_buffer + psession->send_len , psession->total_send_len - psession->send_len , 0 ) ;
		time( & (psession->active_timestamp) );
		if( len < 0 )
		{
			if( _ERRNO == _EWOULDBLOCK )
				return RETURN_SENDING_IN_PROGRESS;
			
			ErrorLog( __FILE__ , __LINE__ , "detected sock[%d] error on sending data" , psession->sock );
			return -1;
		}
		else
		{
			int	short_len ;
			
			short_len = len ;
			if( short_len > 4096 )
				short_len = 4096 ;
			DebugHexLog( __FILE__ , __LINE__ , psession->send_buffer + psession->send_len , short_len , "sended sock[%d] [%d]bytes" , psession->sock , len );
			
			psession->send_len += len ;
		}
	}
	
	CleanSendBuffer( psession );
	
	return 0;
}

void FormatSendHead( struct SocketSession *psession , char *msg_type , int msg_len )
{
	char		pre_buffer[ LEN_COMMHEAD + LEN_MSGHEAD + 1 ] ;
	
	psession->total_send_len = LEN_COMMHEAD + LEN_MSGHEAD + msg_len ;
	psession->send_len = 0 ;
	memset( pre_buffer , 0x00 , sizeof(pre_buffer) );
	snprintf( pre_buffer , sizeof(pre_buffer) , "%0*d%-*s" , LEN_COMMHEAD , psession->total_send_len - LEN_COMMHEAD , LEN_MSGHEAD , msg_type );
	memcpy( psession->send_buffer , pre_buffer , LEN_COMMHEAD + LEN_MSGHEAD );
	
	return;
}

static int HexExpand( char *HexBuf , int HexBufLen , char *AscBuf )
{
	int     i,j=0;
	char    c;
	
	for(i=0;i<HexBufLen;i++){
		c=(HexBuf[i]>>4)&0x0f;
		if(c<=9) AscBuf[j++]=c+'0';
		else AscBuf[j++]=c+'A'-10;
		
		c=HexBuf[i]&0x0f;
		if(c<=9) AscBuf[j++]=c+'0';
		else AscBuf[j++]=c+'A'-10;
	}
	AscBuf[j]=0;
	return(0);
}

int FileMd5( char *pathfilename , char *program_md5_exp )
{
	MD5_CTX		context ;
	FILE		*fp = NULL ;
	char		buf[ 4096 + 1 ] ;
	int		size ;
	char		md5[ MD5_DIGEST_LENGTH + 1 ] ;
	
	MD5_Init( & context );
	
	fp = fopen( pathfilename , "rb" ) ;
	if( fp == NULL )
	{
		return -1;
	}
	
	while( ! feof(fp) )
	{
		memset( buf , 0x00 , sizeof(buf) );
		size = (int)fread( buf , sizeof(char) , sizeof(buf)-1 , fp ) ;
		if( size == 0 )
			break;
		
		MD5_Update( & context , buf , size );
	}
	
	fclose( fp );
	
	memset( md5 , 0x00 , sizeof(md5) );
	MD5_Final( (void*)md5 , & context );
	
	HexExpand( md5 , MD5_DIGEST_LENGTH , program_md5_exp );
	
	return 0;
}


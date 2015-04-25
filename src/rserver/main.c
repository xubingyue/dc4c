#include "rserver.h"

static void version()
{
	printf( "rserver v1.0.0 build %s %s\n" , __DATE__ , __TIME__ );
	printf( "Copyright by calvin<calvinwilliams.c@gmail.com> 2014,2015\n\n" );
	return;
}

static void usage()
{
	printf( "USAGE : rserver --rserver-ip-port ip:port\n\n" );
	return;
}

static int ParseCommandParameter( int argc , char *argv[] , struct ServerEnv *penv )
{
	int	i ;
	
	if( argc == 1 )
	{
		version();
		usage();
		return -1;
	}
	
	for( i = 1 ; i < argc ; i++ )
	{
		if( STRCMP( argv[i] , == , "--rserver-ip-port" ) && i + 1 < argc )
		{
			i++;
			sscanf( argv[i] , "%[^:]:%d" , penv->param.rserver_ip , & (penv->param.rserver_port) );
		}
		else
		{
			printf( "invalid parameter[%s]\n" , argv[i] );
			usage();
			return -1;
		}
	}
	
	if( penv->param.rserver_ip[0] == '\0' || penv->param.rserver_port == 0 )
	{
		usage();
		return -7;
	}
	
	return 0;
}

int main( int argc , char *argv[] )
{
	struct ServerEnv	env , *penv = & env ;
	int			i ;
	
	int			nret = 0 ;
	
	memset( penv , 0x00 , sizeof(struct ServerEnv) );
	
	nret = ParseCommandParameter( argc , argv , penv ) ;
	if( nret )
		return -nret;
	
	SetLogFile( "%s/log/rserver.log" , getenv("HOME") );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	InfoLog( __FILE__ , __LINE__ , "--- rserver [%s:%d] --- begin" , penv->param.rserver_ip , penv->param.rserver_port );
	
	penv->epoll_socks = epoll_create( EPOLL_FD_COUNT ) ;
	if( penv->epoll_socks < 0 )
	{
		ErrorLog( __FILE__ , __LINE__ , "epoll_create failed[%d]errno[%d]" , penv->epoll_socks , errno );
		printf( "epoll_create failed[%d]errno[%d]\n" , penv->epoll_socks , errno );
		return 1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "epoll_create ok , sock[%d]" , penv->epoll_socks );
	}
	
	nret = InitSocketSession( & (penv->listen_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
		printf( "InitSocketSession failed[%d]errno[%d]\n" , nret , errno );
		return 1;
	}
	
	nret = BindListenSocket( penv->param.rserver_ip , penv->param.rserver_port , & (penv->listen_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "BindListenSocket[%s:%d] failed[%d]errno[%d]" , penv->param.rserver_ip , penv->param.rserver_port , nret , errno );
		printf( "BindListenSocket[%s:%d] failed[%d]errno[%d]\n" , penv->param.rserver_ip , penv->param.rserver_port , nret , errno );
		return 1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "BindListenSocket[%s:%d] ok" , penv->param.rserver_ip , penv->param.rserver_port );
	}
	
	AddInputSockToEpoll( penv->epoll_socks , & (penv->listen_session) );
	
	penv->accepted_session_array = (struct SocketSession *)malloc( sizeof(struct SocketSession) * MAXCOUNT_ACCEPTED_SESSION ) ;
	if( penv->accepted_session_array == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "malloc failed[%d]errno[%d]" , nret , errno );
		printf( "malloc failed[%d]errno[%d]\n" , nret , errno );
		return 1;
	}
	memset( penv->accepted_session_array , 0x00 , sizeof(struct SocketSession) * MAXCOUNT_ACCEPTED_SESSION );
	for( i = 0 ; i < MAXCOUNT_ACCEPTED_SESSION ; i++ )
	{
		nret = InitSocketSession( penv->accepted_session_array+i ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
			printf( "InitSocketSession failed[%d]errno[%d]\n" , nret , errno );
			return 1;
		}
	}
	
	nret = ConvertToDaemonServer() ;
	if( nret )
		return nret;
	
	nret = server( penv ) ;
	if( nret )
		return -nret;
	
	close( penv->epoll_socks );
	InfoLog( __FILE__ , __LINE__ , "close all socks and epoll_socks" );
	
	for( i = 0 ; i < MAXCOUNT_ACCEPTED_SESSION ; i++ )
	{
		CleanSocketSession( penv->accepted_session_array+i );
	}
	CleanSocketSession( & (penv->listen_session) );
	
	free( penv->accepted_session_array );
	
	InfoLog( __FILE__ , __LINE__ , "--- rserver [%s:%d] --- end" , penv->param.rserver_ip , penv->param.rserver_port );
	
	return 0;
}


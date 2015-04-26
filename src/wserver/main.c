#include "wserver.h"

static void version()
{
	printf( "wserver v1.0.0 build %s %s\n" , __DATE__ , __TIME__ );
	printf( "Copyright by calvin<calvinwilliams.c@gmail.com> 2014,2015\n\n" );
	return;
}

static void usage()
{
	printf( "USAGE : wserver --rserver-ip-port ip:port --listen-ip-port ip:port\n\n" );
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
		else if( STRCMP( argv[i] , == , "--wserver-ip-port" ) && i + 1 < argc )
		{
			i++;
			sscanf( argv[i] , "%[^:]:%d" , penv->param.wserver_ip , & (penv->param.wserver_port) );
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
	
	if( penv->param.wserver_ip[0] == '\0' || penv->param.wserver_port == 0 )
	{
		usage();
		return -7;
	}
	
	return 0;
}

int main( int argc , char *argv[] )
{
	struct ServerEnv	env , *penv = & env ;
	
	int			nret = 0 ;
	
	memset( penv , 0x00 , sizeof(struct ServerEnv) );
	
	nret = ParseCommandParameter( argc , argv , penv ) ;
	if( nret )
		return -nret;
	
	SetLogFile( "%s/log/wserver_%ld.log" , getenv("HOME") , penv->param.wserver_port );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	InfoLog( __FILE__ , __LINE__ , "--- rserver [%s:%d] - [%s:%d] --- begin" , penv->param.wserver_ip , penv->param.wserver_port , penv->param.rserver_ip , penv->param.rserver_port );
	
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
	
	nret = BindListenSocket( penv->param.wserver_ip , penv->param.wserver_port , & (penv->listen_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "BindListenSocket[%s:%d] failed[%d]errno[%d]" , penv->param.wserver_ip , penv->param.wserver_port , nret , errno );
		printf( "BindListenSocket[%s:%d] failed[%d]errno[%d]\n" , penv->param.wserver_ip , penv->param.wserver_port , nret , errno );
		return 1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "BindListenSocket[%s:%d] ok" , penv->param.wserver_ip , penv->param.wserver_port );
	}
	
	AddInputSockToEpoll( penv->epoll_socks , & (penv->listen_session) );
	
	nret = InitSocketSession( & (penv->connect_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
		printf( "InitSocketSession failed[%d]errno[%d]\n" , nret , errno );
		return 1;
	}
	
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
	
	nret = InitSocketSession( & (penv->accepted_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
		printf( "InitSocketSession failed[%d]errno[%d]\n" , nret , errno );
		return 1;
	}
	
	nret = ConvertToDaemonServer() ;
	if( nret )
		return nret;
	
	nret = server( penv ) ;
	if( nret )
		return -nret;
	
	DeleteSockFromEpoll( penv->epoll_socks , & (penv->listen_session) );
	close( penv->listen_session.sock );
	CleanSocketSession( & (penv->listen_session) );
	
	DeleteSockFromEpoll( penv->epoll_socks , & (penv->connect_session) );
	close( penv->connect_session.sock );
	CleanSocketSession( & (penv->connect_session) );
	
	close( penv->epoll_socks );
	InfoLog( __FILE__ , __LINE__ , "close all socks and epoll_socks" );
	
	InfoLog( __FILE__ , __LINE__ , "--- rserver [%s:%d] - [%s:%d] --- end" , penv->param.wserver_ip , penv->param.wserver_port , penv->param.rserver_ip , penv->param.rserver_port );
	
	return 0;
}


/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 * LastVersion	: v1.0.0
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "server.h"

char __DC4C_WSERVER_VERSION_1_0_0[] = "1.0.0" ;
char *__DC4C_WSERVER_VERSION = __DC4C_WSERVER_VERSION_1_0_0 ;

int		g_main_exit_flag = 0 ;

static void main_signal_proc( int signum )
{
	if( signum == SIGTERM )
		g_main_exit_flag = 1 ;
	return;
}

static void version()
{
	printf( "wserver v%s build %s %s ( util v%s , api v%s )\n" , __DC4C_WSERVER_VERSION , __DATE__ , __TIME__ , __DC4C_UTIL_VERSION , __DC4C_API_VERSION );
	printf( "Copyright by calvin<calvinwilliams.c@gmail.com> 2014,2015\n\n" );
	return;
}

static void usage()
{
	printf( "USAGE : wserver [ -r | --rserver-ip-port ] ip:port,... [ -w | --wserver-ip-port ] ip:port [ -c | --wserver_count count ] [ -d | --loglevel-debug ]\n\n" );
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
		if( ( STRCMP( argv[i] , == , "-r" ) || STRCMP( argv[i] , == , "--rserver-ip-port" ) ) && i + 1 < argc )
		{
			char	buf[ 1024 + 1 ] ;
			char	*p = NULL ;
			
			i++;
			strcpy( buf , argv[i] );
			p = strtok( buf , "," ) ;
			if( p == NULL )
				p = buf ;
			for( penv->rserver_count = 0 ; penv->rserver_count < RSERVER_ARRAYSIZE && p ; penv->rserver_count++ )
			{
				sscanf( p , "%[^:]:%d" , penv->param.rserver_ip[penv->rserver_count] , & (penv->param.rserver_port[penv->rserver_count]) );
				p = strtok( NULL , "," ) ;
			}
		}
		else if( ( STRCMP( argv[i] , == , "-w" ) || STRCMP( argv[i] , == , "--wserver-ip-port" ) ) && i + 1 < argc )
		{
			i++;
			sscanf( argv[i] , "%[^:]:%d" , penv->param.wserver_ip , & (penv->param.wserver_port) );
		}
		else if( ( STRCMP( argv[i] , == , "-c" ) || STRCMP( argv[i] , == , "--wserver-count" ) ) && i + 1 < argc )
		{
			i++;
			penv->param.wserver_count = atoi(argv[i]) ;
		}
		else if( STRCMP( argv[i] , == , "-d" ) || STRCMP( argv[i] , == , "--loglevel-debug" ) )
		{
			penv->param.loglevel_debug = 1 ;
		}
		else
		{
			printf( "invalid parameter[%s]\n" , argv[i] );
			usage();
			return -1;
		}
	}
	
	if( penv->param.rserver_ip[0][0] == '\0' || penv->param.rserver_port[0] == 0 )
	{
		usage();
		return -7;
	}
	
	if( penv->param.wserver_ip[0] == '\0' || penv->param.wserver_port == 0 )
	{
		usage();
		return -7;
	}
	
	if( penv->param.wserver_count == 0 )
		penv->param.wserver_count = 1 ;
	
	penv->param.wserver_port_base = penv->param.wserver_port ;
	
	return 0;
}

int main( int argc , char *argv[] )
{
	struct ServerEnv	env , *penv = & env ;
	
	int			rserver_index ;
	pid_t			pid , pids[ MAXCOUNT_WSERVERS ] ;
	int			status ;
	
	int			nret = 0 ;
	
	memset( penv , 0x00 , sizeof(struct ServerEnv) );
	
	nret = ParseCommandParameter( argc , argv , penv ) ;
	if( nret )
		return -nret;
	
	SetLogFile( "%s/log/dc4c_wserver_m_%s:%d.log" , getenv("HOME") , penv->param.wserver_ip , penv->param.wserver_port );
	if( penv->param.loglevel_debug == 1 )
		SetLogLevel( LOGLEVEL_DEBUG );
	else
		SetLogLevel( LOGLEVEL_INFO );
	
	InfoLog( __FILE__ , __LINE__ , "--- wserver [%s:%d] --- begin" , penv->param.wserver_ip , penv->param.wserver_port );
	for( rserver_index = 0 ; rserver_index < penv->rserver_count ; rserver_index++ )
	{
		InfoLog( __FILE__ , __LINE__ , "--- rserver [%s:%d]" , penv->param.rserver_ip[rserver_index] , penv->param.rserver_port[rserver_index] );
	}
	
	nret = ConvertToDaemonServer() ;
	if( nret )
		return nret;
	
	signal( SIGCLD , SIG_DFL );
	signal( SIGCHLD , SIG_DFL );
	signal( SIGPIPE , SIG_IGN );
	
	signal( SIGTERM , & main_signal_proc );
	
	for( penv->wserver_index = 0 ; penv->wserver_index < penv->param.wserver_count ; penv->wserver_index++ )
	{
		penv->param.wserver_port = penv->param.wserver_port_base + penv->wserver_index ;
		
		pids[penv->wserver_index] = fork() ;
		if( pids[penv->wserver_index] < 0 )
		{
			FatalLog( __FILE__ , __LINE__ , "fork failed , errno[%d]" , (int)pids[penv->wserver_index] );
		}
		else if( pids[penv->wserver_index] == 0 )
		{
			nret = server( penv ) ;
			exit(-nret);
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "[%d]fork[%d] - [%d]" , (int)getpid() , (int)pids[penv->wserver_index] , penv->wserver_index+1 );
			
		}
	}
	
	while( ! g_main_exit_flag )
	{
		pid = waitpid( -1 , & status , 0 ) ;
		if( g_main_exit_flag )
			break;
		for( penv->wserver_index = 0 ; penv->wserver_index < penv->param.wserver_count ; penv->wserver_index++ )
			if( pids[penv->wserver_index] == pid )
				break;
		if( penv->wserver_index >= penv->param.wserver_count )
		{
			FatalLog( __FILE__ , __LINE__ , "pid[%d] invalid" , (int)pid );
			return DC4C_ERROR_INTERNAL;
		}
		if( WTERMSIG(status) )
		{
			ErrorLog( __FILE__ , __LINE__ , "wserver[%d] terminated" , (int)pid );
		}
		else if( WIFSIGNALED(status) )
		{
			ErrorLog( __FILE__ , __LINE__ , "wserver[%d] signaled" , (int)pid );
		}
		else if( WIFSTOPPED(status) )
		{
			ErrorLog( __FILE__ , __LINE__ , "wserver[%d] stoped" , (int)pid );
		}
		else if( WIFEXITED(status) )
		{
			InfoLog( __FILE__ , __LINE__ , "wserver[%d] exit[%d]" , (int)pid , WEXITSTATUS(status) );
		}
		
		sleep(10);
		
		penv->param.wserver_port = penv->param.wserver_port_base + penv->wserver_index ;
		
		pids[penv->wserver_index] = fork() ;
		if( pids[penv->wserver_index] < 0 )
		{
			FatalLog( __FILE__ , __LINE__ , "fork failed , errno[%d]" , (int)pids[penv->wserver_index] );
		}
		else if( pids[penv->wserver_index] == 0 )
		{
			nret = server( penv ) ;
			exit(-nret);
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "[%d]fork[%d] - [%d]" , (int)getpid() , (int)pids[penv->wserver_index] , penv->wserver_index+1 );
		}
	}
	
	InfoLog( __FILE__ , __LINE__ , "--- wserver [%s:%d] --- end" , penv->param.wserver_ip , penv->param.wserver_port );
	
	return 0;
}


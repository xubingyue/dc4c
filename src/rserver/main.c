/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 * LastVersion	: v1.0.0
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "server.h"

char __DC4C_RSERVER_VERSION_1_0_0[] = "1.0.0" ;
char *__DC4C_RSERVER_VERSION = __DC4C_RSERVER_VERSION_1_0_0 ;

int		g_main_exit_flag = 0 ;

static void main_signal_proc( int signum )
{
	if( signum == SIGTERM )
		g_main_exit_flag = 1 ;
	return;
}

static void version()
{
	printf( "rserver v%s build %s %s ( util v%s , api v%s )\n" , __DC4C_RSERVER_VERSION , __DATE__ , __TIME__ , __DC4C_UTIL_VERSION , __DC4C_API_VERSION );
	printf( "Copyright by calvin<calvinwilliams.c@gmail.com> 2014,2015\n\n" );
	return;
}

static void usage()
{
	printf( "USAGE : rserver [ -r | --rserver-ip-port ] ip:port,... [ -d | --loglevel-debug ]\n\n" );
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
			i++;
			sscanf( argv[i] , "%[^:]:%d" , penv->param.rserver_ip , & (penv->param.rserver_port) );
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
	
	pid_t			pid ;
	int			status ;
	
	int			nret = 0 ;
	
	memset( penv , 0x00 , sizeof(struct ServerEnv) );
	
	nret = ParseCommandParameter( argc , argv , penv ) ;
	if( nret )
		return -nret;
	
	SetLogFile( "%s/log/dc4c_rserver_m_%s:%d.log" , getenv("HOME") , penv->param.rserver_ip , penv->param.rserver_port );
	if( penv->param.loglevel_debug == 1 )
		SetLogLevel( LOGLEVEL_DEBUG );
	else
		SetLogLevel( LOGLEVEL_INFO );
	
	InfoLog( __FILE__ , __LINE__ , "--- rserver [%s:%d] --- begin" , penv->param.rserver_ip , penv->param.rserver_port );
	
	nret = ConvertToDaemonServer() ;
	if( nret )
		return nret;
	
	signal( SIGCLD , SIG_DFL );
	signal( SIGCHLD , SIG_DFL );
	signal( SIGPIPE , SIG_IGN );
	
	signal( SIGTERM , & main_signal_proc );
	
	while( ! g_main_exit_flag )
	{
		pid = fork() ;
		if( pid < 0 )
		{
			FatalLog( __FILE__ , __LINE__ , "fork failed , errno[%d]" , (int)pid );
		}
		else if( pid == 0 )
		{
			nret = server( penv ) ;
			exit(-nret);
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "[%d]fork[%d]" , (int)getpid() , (int)pid );
			
			pid = wait( & status ) ;
			if( g_main_exit_flag )
				break;
			if( WTERMSIG(status) )
			{
				ErrorLog( __FILE__ , __LINE__ , "rserver[%d] terminated" , (int)pid );
			}
			else if( WIFSIGNALED(status) )
			{
				ErrorLog( __FILE__ , __LINE__ , "rserver[%d] signaled" , (int)pid );
			}
			else if( WIFSTOPPED(status) )
			{
				ErrorLog( __FILE__ , __LINE__ , "rserver[%d] stoped" , (int)pid );
			}
			else if( WIFEXITED(status) )
			{
				InfoLog( __FILE__ , __LINE__ , "rserver[%d] exit[%d]" , (int)pid , WEXITSTATUS(status) );
			}
			
			sleep(10);
		}
	}
	
	InfoLog( __FILE__ , __LINE__ , "--- rserver [%s:%d] --- end" , penv->param.rserver_ip , penv->param.rserver_port );
	
	return 0;
}


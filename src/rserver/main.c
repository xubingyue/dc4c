/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "server.h"

int		g_main_exit_flag = 0 ;

static void main_signal_proc( int signum )
{
	if( signum == SIGTERM )
		g_main_exit_flag = 1 ;
	return;
}

static void version()
{
	printf( "rserver v%s build %s %s\n" , __DC4C_VERSION , __DATE__ , __TIME__ );
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
	
	struct sigaction	act , oact ;
	
	pid_t			pid , pids[1] ;
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
	
	memset( & act , 0x00 , sizeof(struct sigaction) );
	act.sa_handler = main_signal_proc ;
	sigemptyset( & (act.sa_mask) );
	act.sa_flags = 0 ;
	sigaction( SIGTERM , & act , & oact );
	
	while( ! g_main_exit_flag )
	{
		pids[0] = fork() ;
		if( pids[0] < 0 )
		{
			FatalLog( __FILE__ , __LINE__ , "fork failed , errno[%d]" , (int)pids[0] );
		}
		else if( pids[0] == 0 )
		{
			nret = server( penv ) ;
			exit(-nret);
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "[%d]fork[%d]" , (int)getpid() , (int)pids[0] );
			
			pid = waitpid( pids[0] , & status , 0 ) ;
			if( g_main_exit_flag )
			{
				InfoLog( __FILE__ , __LINE__ , "kill [%d]" , pids[0] );
				kill( pids[0] , SIGTERM );
				break;
			}
			
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


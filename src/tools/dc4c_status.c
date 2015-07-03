/*
 * dc4c_status
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

static void version()
{
	printf( "dc4c_status v1.0.0 build %s %s\n" , __DATE__ , __TIME__ );
	printf( "Copyright by calvin<calvinwilliams.c@gmail.com> 2014,2015\n\n" );
	return;
}

static void usage()
{
	printf( "USAGE : dc4c_status waitpid_status\n\n" );
	return;
}

static int _status( int status )
{
	if( WTERMSIG(status) )
	{
		printf( "WTERMSIG\n" );
	}
	else if( WIFSIGNALED(status) )
	{
		printf( "WIFSIGNALED\n" );
	}
	else if( WIFSTOPPED(status) )
	{
		printf( "WIFSTOPPED\n" );
	}
	else if( WIFEXITED(status) )
	{
		printf( "WIFEXITED %d\n" , WEXITSTATUS(status) );
	}
	else
	{
		printf( "Unknow status\n" );
	}
	
	return 0;
}

int main( int argc , char *argv[] )
{
	if( argc == 1 + 1 )
	{
		if( strcmp( argv[1] , "-v" ) == 0 )
		{
			version();
			usage();
			exit(0);
		}
		
		_status( atoi(argv[1]) );
	}
	else
	{
		usage();
	}
	
	return 0;
}


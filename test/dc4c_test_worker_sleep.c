#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "dc4c_api.h"

int main( int argc , char *argv[] )
{
	struct timeval		tv ;
	int			seconds ;
	
	DC4CSetAppLogFile( "dc4c_test_worker_sleep" );
	
	gettimeofday( & tv , NULL );
	srand( (unsigned int)(tv.tv_sec*tv.tv_usec) );
	
	if( argc == 1 + 1 )
	{
		InfoLog( __FILE__ , __LINE__ , "BEGIN SLEEP [%s]seconds" , argv[1] );
		seconds = atoi(argv[1]) ;
		if( seconds < 0 )
			seconds = rand() % (-seconds) ;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "BEGIN SLEEP" );
		seconds = 1 ;
	}
	
	sleep( seconds );
	InfoLog( __FILE__ , __LINE__ , "END SLEEP [%d]seconds" , seconds );
	
	DC4CSetReplyInfo( "Elapse [%d]seconds" , seconds );
	
	return 0;
}


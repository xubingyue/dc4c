#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "dc4c_api.h"

int main( int argc , char *argv[] )
{
	struct timeval		tv ;
	
	DC4CSetAppLogFile( "dc4c_test_worker_sleep" );
	
	gettimeofday( & tv , NULL );
	srand( (unsigned int)(tv.tv_sec*tv.tv_usec) );
	
	if( argc == 1 + 1 )
	{
		int	seconds ;
		
		InfoLog( __FILE__ , __LINE__ , "BEGIN SLEEP [%s]seconds" , argv[1] );
		
		seconds = atoi(argv[1]) ;
		if( seconds < 0 )
			seconds = rand() % (-seconds) ;
		sleep( seconds );
		
		InfoLog( __FILE__ , __LINE__ , "END SLEEP [%d]seconds" , seconds );
	}
	
	return 0;
}


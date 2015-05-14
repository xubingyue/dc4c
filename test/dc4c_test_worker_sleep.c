#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "dc4c_api.h"

int main( int argc , char *argv[] )
{
	DC4CSetAppLogFile();
	
	srand( (unsigned)time( NULL ) );
	
	InfoLog( __FILE__ , __LINE__ , "dc4c_test_worker_sleep ( api v%s )" , __DC4C_API_VERSION );
	
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
	
	return 1;
}


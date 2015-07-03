#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

#include "dc4c_api.h"

int main( int argc , char *argv[] )
{
	struct timeval		tv ;
	int			seconds ;
	
	gettimeofday( & tv , NULL );
	srand( (unsigned int)(tv.tv_sec*tv.tv_usec) );
	
	if( argc == 1 + 1 )
	{
		seconds = atoi(argv[1]) ;
		if( seconds < 0 )
			seconds = rand() % (-seconds) ;
	}
	else
	{
		seconds = 1 ;
	}
	
	sleep( seconds );
	
	DC4CFormatReplyInfo( "Elapse [%d]seconds" , seconds );
	
	return 0;
}


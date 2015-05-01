#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "LOGC.h"

int main( int argc , char *argv[] )
{
	srand( (unsigned)time( NULL ) );
	
	if( argc == 1 + 1 )
	{
		int	num ;
		
		num = atoi(argv[1]) ;
		if( num < 0 )
			sleep( rand() % (-num) );
		else
			sleep( num );
	}
	
	return 2;
}


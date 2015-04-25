#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "LOGC.h"

int main( int argc , char *argv[] )
{
	if( argc > 1 )
	{
		sleep( atoi(argv[1]) );
	}
	
	return 0;
}


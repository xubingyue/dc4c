#include "dc4c_api.h"

int main( int argc , char *argv[] )
{
	execute_program_response	epp ;
	
	int				nret = 0 ;
	
	if( argc == 1 + 4 )
	{
		nret = DC4CDoTaskImmediately( argv[1] , atoi(argv[2]) , argv[3] , argv[4] , & epp ) ;
		if( nret )
		{
			printf( "DC4CDoTaskImmediately failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CDoTaskImmediately ok , response[%d][%d]\n" , epp.response_code , epp.status );
		}
	}
	else
	{
		printf( "USAGE : dc4c_test_rserver_ip rserver_port master program params\n" );
		exit(7);
	}
	
	return 0;
}


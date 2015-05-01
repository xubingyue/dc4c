#include "dc4c_api.h"

int main( int argc , char *argv[] )
{
	struct Dc4cApiEnv	*penv = NULL ;
	int			status ;
	
	int			nret = 0 ;
	
	if( argc == 1 + 3 )
	{
		nret = DC4CInitEnv( & penv , argv[1] , atoi(argv[2]) ) ;
		if( nret )
		{
			printf( "DC4CInitEnv failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CInitEnv ok\n" );
		}
		
		DC4CSetTimeout( penv , 120 );
		
		nret = DC4CDoTask( penv , argv[3] ) ;
		if( nret )
		{
			printf( "DC4CDoTask failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CDoTask ok\n" );
			
			DC4CGetTaskResponseStatus( penv , & status );
			printf( "DC4CGetTaskResponseStatus ok , status[%d]\n" , WEXITSTATUS(status) );
		}
		
		DC4CCleanEnv( & penv );
		printf( "DC4CCleanEnv ok\n" );
	}
	else
	{
		printf( "USAGE : dc4c_test_rserver_ip rserver_port master program_and_params\n" );
		exit(7);
	}
	
	return 0;
}


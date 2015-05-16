#include "dc4c_api.h"	

int main( int argc , char *argv[] )
{
	struct Dc4cApiEnv	*penv = NULL ;
	
	char			*program_and_params = NULL ;
	char			*tid = NULL ;
	char			*ip = NULL ;
	long			port ;
	int			response_code ;
	int			status ;
	
	int			nret = 0 ;
	
	printf( "dc4c_test_master ( api v%s )\n" , __DC4C_API_VERSION );
	
	if( argc == 1 + 2 )
	{
		nret = DC4CInitEnv( & penv , argv[1] ) ;
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
		
		nret = DC4CDoTask( penv , argv[2] ) ;
		if( nret )
		{
			printf( "DC4CDoTask failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CDoTask ok\n" );
			
			DC4CGetTaskProgramAndParam( penv , & program_and_params );
			DC4CGetTaskTid( penv , & tid );
			DC4CGetTaskIp( penv , & ip );
			DC4CGetTaskPort( penv , & port );
			DC4CGetTaskResponseCode( penv , & response_code );
			DC4CGetTaskStatus( penv , & status );
			printf( "DC4CGetTaskResponseStatus - [%s] [%s] - [%s] [%ld] - [%d] [%d]\n" , program_and_params , tid , ip , port , response_code , WEXITSTATUS(status) );
		}
		
		DC4CCleanEnv( & penv );
		printf( "DC4CCleanEnv ok\n" );
	}
	else
	{
		printf( "USAGE : dc4c_test_master rserver_ip:rserver_port program_and_params\n" );
		exit(7);
	}
	
	return 0;
}


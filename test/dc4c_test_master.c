#include "dc4c_api.h"	

/* for testing
time ./dc4c_test_master 192.168.6.54:12001,192.168.6.54:12002 "dc4c_test_worker_sleep 3"
*/

int main( int argc , char *argv[] )
{
	struct Dc4cApiEnv	*penv = NULL ;
	
	char			*ip = NULL ;
	long			port ;
	char			*tid = NULL ;
	char			*program_and_params = NULL ;
	int			timeout ;
	int			elapse ;
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
		
		DC4CSetTimeout( penv , 60 );
		
		nret = DC4CDoTask( penv , argv[2] , DC4CGetTimeout(penv) ) ;
		if( nret )
		{
			printf( "DC4CDoTask failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CDoTask ok\n" );
			
			DC4CGetTaskIp( penv , & ip );
			DC4CGetTaskPort( penv , & port );
			DC4CGetTaskTid( penv , & tid );
			DC4CGetTaskProgramAndParam( penv , & program_and_params );
			DC4CGetTaskTimeout( penv , & timeout );
			DC4CGetTaskElapse( penv , & elapse );
			DC4CGetTaskResponseCode( penv , & response_code );
			DC4CGetTaskStatus( penv , & status );
			printf( "DC4CGetTaskResponseStatus - [%s] [%ld] - [%s] [%s] [%d] [%d] - [%d] [%d]\n" , ip , port , tid , program_and_params , timeout , elapse , response_code , WEXITSTATUS(status) );
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


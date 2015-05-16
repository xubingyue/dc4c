#include "dc4c_api.h"

int main( int argc , char *argv[] )
{
	struct Dc4cApiEnv	*penv = NULL ;
	char			**program_and_params_array = NULL ;
	int			program_and_params_count ;
	int			worker_count ;
	int			i ;
	
	char			*program_and_params = NULL ;
	char			*tid = NULL ;
	char			*ip = NULL ;
	long			port ;
	int			response_code ;
	int			status ;
	
	int			nret = 0 ;
	
	printf( "dc4c_test_batch_master ( api v%s )\n" , __DC4C_API_VERSION );
	
	if( argc >= 1 + 3 )
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
		
		DC4CSetTimeout( penv , 5 );
		
		worker_count = atoi(argv[2]) ;
		if( worker_count > 0 )
		{
			program_and_params_array = argv+1+2 ;
			program_and_params_count = argc-1-2 ;
		}
		else if( worker_count == 0 )
		{
			worker_count = argc-1-2 ;
			program_and_params_array = argv+1+2 ;
			program_and_params_count = argc-1-2 ;
		}
		else
		{
			int		i ;
			
			worker_count = -worker_count ;
			program_and_params_array = (char**)malloc( sizeof(char*) * worker_count ) ;
			if( program_and_params_array == NULL )
			{
				printf( "alloc failed , errno[%d]\n" , errno );
				return 1;
			}
			memset( program_and_params_array , 0x00 , sizeof(char*) * worker_count );
			
			for( i = 0 ; i < worker_count ; i++ )
			{
				program_and_params_array[i] = argv[3] ;
			}
			
			program_and_params_count = worker_count ;
		}
		nret = DC4CDoBatchTasks( penv , worker_count , program_and_params_array , program_and_params_count ) ;
		if( nret )
		{
			printf( "DC4CDoBatchTasks failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CDoBatchTasks ok\n" );
			
			for( i = 1 ; i <= program_and_params_count ; i++ )
			{
				DC4CGetBatchTasksProgramAndParam( penv , i , & program_and_params );
				DC4CGetBatchTasksTid( penv , i , & tid );
				DC4CGetBatchTasksIp( penv , i , & ip );
				DC4CGetBatchTasksPort( penv , i , & port );
				DC4CGetBatchTasksResponseCode( penv , i , & response_code );
				DC4CGetBatchTasksStatus( penv , i , & status );
				printf( "DC4CGetBatchTasksResponseStatus - [%d] - [%s] [%s] - [%s] [%ld] - [%d] [%d]\n" , i , program_and_params , tid , ip , port , response_code , WEXITSTATUS(status) );
			}
		}
		
		DC4CCleanEnv( & penv );
		printf( "DC4CCleanEnv ok\n" );
	}
	else
	{
		printf( "USAGE : dc4c_test_batch_master rserver_ip:rserver_port program_and_params_1 ...\n" );
		exit(7);
	}
	
	return 0;
}


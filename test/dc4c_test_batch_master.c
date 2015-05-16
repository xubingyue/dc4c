#include "dc4c_api.h"

/* for testing
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 2 3 "dc4c_test_worker_sleep 1" "dc4c_test_worker_sleep 2" "dc4c_test_worker_sleep 3"
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 4 -10 "dc4c_test_worker_sleep 10"
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 -1 -100 "dc4c_test_worker_sleep -10"
*/

int main( int argc , char *argv[] )
{
	struct Dc4cApiEnv	*penv = NULL ;
	struct Dc4cBatchTask	*p_tasks = NULL ;
	struct Dc4cBatchTask	*p_task = NULL ;
	int			tasks_count ;
	int			workers_count ;
	int			repeat_task_flag ;
	int			i ;
	
	char			*ip = NULL ;
	long			port ;
	char			*tid = NULL ;
	char			*program_and_params = NULL ;
	int			timeout ;
	int			elapse ;
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
		
		DC4CSetTimeout( penv , 60);
		
		workers_count = atoi(argv[2]) ;
		tasks_count = atoi(argv[3]) ;
		
		if( tasks_count < 0 )
		{
			repeat_task_flag = 1 ;
			tasks_count = -tasks_count ;
		}
		else
		{
			repeat_task_flag = 0 ;
		}
		
		if( workers_count < 0 )
		{
			workers_count = tasks_count ;
		}
		
		p_tasks = (struct Dc4cBatchTask *)malloc( sizeof(struct Dc4cBatchTask) * tasks_count ) ;
		if( p_tasks == NULL )
		{
			printf( "alloc failed , errno[%d]\n" , errno );
			return 1;
		}
		memset( p_tasks , 0x00 , sizeof(sizeof(struct Dc4cBatchTask) * tasks_count) );
		
		for( i = 0 , p_task = p_tasks ; i < tasks_count ; i++ , p_task++ )
		{
			if( repeat_task_flag == 1 )
				strcpy( p_task->program_and_params , argv[4] );
			else
				strcpy( p_task->program_and_params , argv[4+i] );
			p_task->timeout = DC4CGetTimeout(penv) ;
		}
		
		nret = DC4CDoBatchTasks( penv , workers_count , p_tasks , tasks_count ) ;
		if( nret )
		{
			printf( "DC4CDoBatchTasks failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CDoBatchTasks ok\n" );
			
			for( i = 1 ; i <= tasks_count ; i++ )
			{
				DC4CGetBatchTasksIp( penv , i , & ip );
				DC4CGetBatchTasksPort( penv , i , & port );
				DC4CGetBatchTasksTid( penv , i , & tid );
				DC4CGetBatchTasksProgramAndParam( penv , i , & program_and_params );
				DC4CGetBatchTasksTimeout( penv , i , & timeout );
				DC4CGetBatchTasksElapse( penv , i , & elapse );
				DC4CGetBatchTasksResponseCode( penv , i , & response_code );
				DC4CGetBatchTasksStatus( penv , i , & status );
				printf( "DC4CGetBatchTasksResponseStatus - [%d] - [%s] [%ld] - [%s] [%s] [%d] [%d] - [%d] [%d]\n" , i , ip , port , tid , program_and_params , timeout , elapse , response_code , WEXITSTATUS(status) );
			}
		}
		
		DC4CCleanEnv( & penv );
		printf( "DC4CCleanEnv ok\n" );
	}
	else
	{
		printf( "USAGE : dc4c_test_batch_master rserver_ip:rserver_port,... workers_count task_count program_and_params_1 ...\n" );
		exit(7);
	}
	
	return 0;
}


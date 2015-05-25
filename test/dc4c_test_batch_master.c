#include "dc4c_api.h"

/* for testing
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 2 3 "dc4c_test_worker_sleep 1" "dc4c_test_worker_sleep 2" "dc4c_test_worker_sleep 3"
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 4 -10 "dc4c_test_worker_sleep 10"
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 -1 -100 "dc4c_test_worker_sleep -10"
*/

int main( int argc , char *argv[] )
{
	struct Dc4cApiEnv	*penv = NULL ;
	struct Dc4cBatchTask	*tasks_array = NULL ;
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
	int			error ;
	int			status ;
	char			*info = NULL ;
	
	int			nret = 0 ;
	
	DC4CSetAppLogFile( "dc4c_test_batch_master" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
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
		
		DC4CSetTimeout( penv , 60 );
		DC4CSetOptions( penv , DC4C_OPTIONS_INTERRUPT_BY_APP );
		
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
		
		tasks_array = (struct Dc4cBatchTask *)malloc( sizeof(struct Dc4cBatchTask) * tasks_count ) ;
		if( tasks_array == NULL )
		{
			printf( "alloc failed , errno[%d]\n" , errno );
			return 1;
		}
		memset( tasks_array , 0x00 , sizeof(struct Dc4cBatchTask) * tasks_count );
		
		for( i = 0 , p_task = tasks_array ; i < tasks_count ; i++ , p_task++ )
		{
			if( repeat_task_flag == 1 )
				strcpy( p_task->program_and_params , argv[4] );
			else
				strcpy( p_task->program_and_params , argv[4+i] );
			p_task->timeout = DC4CGetTimeout(penv) ;
		}
		
		nret = DC4CDoBatchTasks( penv , workers_count , tasks_array , tasks_count ) ;
		free( tasks_array );
		if( nret )
		{
			printf( "DC4CDoBatchTasks failed[%d]\n" , nret );
		}
		else
		{
			printf( "DC4CDoBatchTasks ok\n" );
		}
		
		printf( "tasks_count[%d] worker_count[%d] - prepare_count[%d] running_count[%d] finished_count[%d]\n" , DC4CGetTasksCount(penv) , DC4CGetWorkersCount(penv) , DC4CGetPrepareTasksCount(penv) , DC4CGetRunningTasksCount(penv) , DC4CGetFinishedTasksCount(penv) );
		
		tasks_count = DC4CGetTasksCount( penv ) ;
		for( i = 0 ; i < tasks_count ; i++ )
		{
			DC4CGetBatchTasksIp( penv , i , & ip );
			DC4CGetBatchTasksPort( penv , i , & port );
			DC4CGetBatchTasksTid( penv , i , & tid );
			DC4CGetBatchTasksProgramAndParam( penv , i , & program_and_params );
			DC4CGetBatchTasksTimeout( penv , i , & timeout );
			DC4CGetBatchTasksElapse( penv , i , & elapse );
			DC4CGetBatchTasksError( penv , i , & error );
			DC4CGetBatchTasksStatus( penv , i , & status );
			DC4CGetBatchTasksInfo( penv , i , & info );
			printf( "Task[%d]-[%s][%ld]-[%s][%s][%d][%d]-[%d][%d][%s]\n" , i , ip , port , tid , program_and_params , timeout , elapse , error , WEXITSTATUS(status) , info );
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

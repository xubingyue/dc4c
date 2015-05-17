#include "dc4c_api.h"

/* for testing
time ./dc4c_test_multi_batch_master 192.168.6.54:12001,192.168.6.54:12002 -1 -100 "dc4c_test_worker_sleep -10"
*/

int main( int argc , char *argv[] )
{
	struct Dc4cApiEnv	**ppenvs = NULL ;
	int			envs_count ;
	struct Dc4cApiEnv	*penv = NULL ;
	int			remain_envs_count ;
	struct Dc4cBatchTask	task ;
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
	int			error ;
	int			status ;
	char			*info = NULL ;
	
	int			nret = 0 ;
	
	printf( "dc4c_test_multi_batch_master ( api v%s )\n" , __DC4C_API_VERSION );
	
	if( argc >= 1 + 3 )
	{
		envs_count = 2 ;
		ppenvs = (struct Dc4cApiEnv**)malloc( sizeof(struct Dc4cApiEnv*) * envs_count ) ;
		if( ppenvs == NULL )
		{
			printf( "malloc failed[%d] , errno[%d]\n" , nret , errno );
			return 1;
		}
		memset( ppenvs , 0x00 , sizeof(struct Dc4cApiEnv*) * envs_count );
		
		nret = DC4CInitEnv( & (ppenvs[0]) , argv[1] ) ;
		if( nret )
		{
			printf( "DC4CInitEnv failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CInitEnv ok\n" );
		}
		
		DC4CSetTimeout( ppenvs[0] , 60 );
		
		nret = DC4CInitEnv( & (ppenvs[1]) , argv[1] ) ;
		if( nret )
		{
			printf( "DC4CInitEnv failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CInitEnv ok\n" );
		}
		
		DC4CSetTimeout( ppenvs[1] , 60 );
		
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
		
		strcpy( task.program_and_params , "dc4c_test_worker_sleep 8" );
		task.timeout = DC4CGetTimeout(ppenvs[1]) ;
		
		nret = DC4CBeginBatchTasks( ppenvs[0] , workers_count , & task , 1 ) ;
		if( nret )
		{
			printf( "DC4CBeginBatchTasks failed[%d] , errno[%d]\n" , nret , errno );
			return 1;
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
			p_task->timeout = DC4CGetTimeout(ppenvs[1]) ;
		}
		
		nret = DC4CBeginBatchTasks( ppenvs[1] , workers_count , p_tasks , tasks_count ) ;
		if( nret )
		{
			printf( "DC4CBeginBatchTasks failed[%d] , errno[%d]\n" , nret , errno );
			return 1;
		}
		
		while(1)
		{
			nret = DC4CPerformMultiBatchTasks( ppenvs , envs_count , & penv , & remain_envs_count ) ;
			if( nret == DC4C_INFO_NO_UNFINISHED_ENVS )
			{
				printf( "DC4CPerformMultiBatchTasks ok , all is done\n" );
				free( p_tasks );
				break;
			}
			else if( nret )
			{
				printf( "DC4CPerformMultiBatchTasks failed[%d]\n" , nret );
				free( p_tasks );
				return 1;
			}
			else
			{
				printf( "DC4CPerformMultiBatchTasks ok\n" );
				
				tasks_count = DC4CGetTasksCount( penv ) ;
				for( i = 1 ; i <= tasks_count ; i++ )
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
					printf( "DC4CGetBatchTasksResponseStatus - [%d] - [%s] [%ld] - [%s] [%s] [%d] [%d] - [%d] [%d] [%s]\n" , i , ip , port , tid , program_and_params , timeout , elapse , error , WEXITSTATUS(status) , info );
				}
			}
		}
		
		DC4CCleanEnv( & (ppenvs[0]) );
		DC4CCleanEnv( & (ppenvs[1]) );
		free( ppenvs );
		printf( "DC4CCleanEnv ok\n" );
	}
	else
	{
		printf( "USAGE : dc4c_test_multi_batch_master rserver_ip:rserver_port,... workers_count task_count program_and_params_1 ...\n" );
		exit(7);
	}
	
	return 0;
}


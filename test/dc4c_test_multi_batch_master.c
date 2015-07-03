#include "dc4c_util.h"
#include "dc4c_api.h"

/* for testing
time ./dc4c_test_multi_batch_master 192.168.6.54:12001,192.168.6.54:12002 3 -1 -100 "dc4c_test_worker_sleep -10"
*/

int main( int argc , char *argv[] )
{
	struct Dc4cApiEnv	**a_penv = NULL ;
	int			*a_tasks_count ;
	int			*a_workers_count ;
	struct Dc4cBatchTask	**a_tasks_array = NULL ;
	int			envs_count ;
	int			envs_index ;
	struct Dc4cApiEnv	*penv = NULL ;
	int			tasks_count ;
	int			task_index ;
	struct Dc4cBatchTask	*p_task = NULL ;
	int			repeat_task_flag ;
	
	char			begin_timebuf[ 256 + 1 ] ;
	char			end_timebuf[ 256 + 1 ] ;
	
	int			nret = 0 ;
	
	DC4CSetAppLogFile( "dc4c_test_multi_batch_master" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	if( argc >= 1 + 4 )
	{
		envs_count = atoi(argv[2]) ;
		
		a_penv = (struct Dc4cApiEnv**)malloc( sizeof(struct Dc4cApiEnv*) * envs_count ) ;
		if( a_penv == NULL )
		{
			printf( "malloc failed[%d] , errno[%d]\n" , nret , errno );
			return 1;
		}
		memset( a_penv , 0x00 , sizeof(struct Dc4cApiEnv*) * envs_count );
		
		a_tasks_count = (int*)malloc( sizeof(int) * envs_count ) ;
		if( a_tasks_count == NULL )
		{
			printf( "malloc failed[%d] , errno[%d]\n" , nret , errno );
			return 1;
		}
		memset( a_tasks_count , 0x00 , sizeof(int) * envs_count );
		
		a_workers_count = (int*)malloc( sizeof(int) * envs_count ) ;
		if( a_workers_count == NULL )
		{
			printf( "malloc failed[%d] , errno[%d]\n" , nret , errno );
			return 1;
		}
		memset( a_workers_count , 0x00 , sizeof(int) * envs_count );
		
		a_tasks_array = (struct Dc4cBatchTask**)malloc( sizeof(struct Dc4cBatchTask*) * envs_count ) ;
		if( a_tasks_array == NULL )
		{
			printf( "malloc failed[%d] , errno[%d]\n" , nret , errno );
			return 1;
		}
		memset( a_tasks_array , 0x00 , sizeof(struct Dc4cBatchTask*) * envs_count );
		
		for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
		{
			nret = DC4CInitEnv( & (a_penv[envs_index]) , argv[1] ) ;
			if( nret )
			{
				printf( "DC4CInitEnv failed[%d]\n" , nret );
				return 1;
			}
			else
			{
				printf( "DC4CInitEnv ok , penv[%p]\n" , a_penv[envs_index] );
			}
			
			DC4CSetTimeout( a_penv[envs_index] , 15 );
			DC4CSetOptions( a_penv[envs_index] , DC4C_OPTIONS_INTERRUPT_BY_APP );
			
			a_workers_count[envs_index] = atoi(argv[3]) ;
			a_tasks_count[envs_index] = atoi(argv[4]) ;
			
			if( a_tasks_count[envs_index] < 0 )
			{
				a_tasks_count[envs_index] = -a_tasks_count[envs_index] ;
				repeat_task_flag = 1 ;
			}
			else
			{
				repeat_task_flag = 0 ;
			}
			
			if( a_workers_count[envs_index] < 0 )
			{
				if( a_workers_count[envs_index] == -2 )
				{
					nret = DC4CQueryWorkers( penv ) ;
					if( nret )
					{
						printf( "DC4CQueryWorkers failed[%d]\n" , nret );
						return 1;
					}
					
					a_workers_count[envs_index] = DC4CGetUnusedWorkersCount( penv ) ;
					if( a_workers_count[envs_index] <= 0 )
					{
						printf( "workers_count[%d] invalid\n" , a_workers_count[envs_index] );
						return 1;
					}
					
					a_tasks_count[envs_index] = a_workers_count[envs_index] ;
				}
				else
				{
					a_workers_count[envs_index] = a_tasks_count[envs_index] ;
				}
			}
			
			a_tasks_array[envs_index] = (struct Dc4cBatchTask *)malloc( sizeof(struct Dc4cBatchTask) * a_tasks_count[envs_index] ) ;
			if( a_tasks_array[envs_index] == NULL )
			{
				printf( "alloc failed , errno[%d]\n" , errno );
				return 1;
			}
			memset( a_tasks_array[envs_index] , 0x00 , sizeof(struct Dc4cBatchTask) * a_tasks_count[envs_index] );
			
			for( task_index = 0 ; task_index < a_tasks_count[envs_index] ; task_index++ )
			{
				p_task = & (a_tasks_array[envs_index][task_index]) ;
				
				p_task->order_index = task_index ;
				if( repeat_task_flag == 1 )
					strcpy( p_task->program_and_params , argv[5] );
				else
					strcpy( p_task->program_and_params , argv[5+task_index] );
				p_task->timeout = DC4CGetTimeout(a_penv[envs_index]) ;
			}
		}
		
		nret = DC4CDoMultiBatchTasks( a_penv , envs_count , a_workers_count , a_tasks_array , a_tasks_count ) ;
		if( nret )
		{
			printf( "DC4CDoMultiBatchTasks failed[%d]\n" , nret );
		}
		else
		{
			printf( "DC4CDoMultiBatchTasks ok\n" );
			
		}
		
		for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
		{
			penv = a_penv[envs_index] ;
			printf( "penv[%p]\n" , penv );
			tasks_count = DC4CGetTasksCount( penv ) ;
			for( task_index = 0 ; task_index < tasks_count ; task_index++ )
			{
				printf( "[%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
					, task_index , DC4CGetBatchTasksIp(penv,task_index) , DC4CGetBatchTasksPort(penv,task_index)
					, DC4CGetBatchTasksTid(penv,task_index) , DC4CGetBatchTasksProgramAndParams(penv,task_index) , DC4CGetBatchTasksTimeout(penv,task_index) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,task_index),begin_timebuf,sizeof(begin_timebuf))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,task_index),end_timebuf,sizeof(end_timebuf))+11 , DC4CGetBatchTasksElapse(penv,task_index)
					, DC4CGetBatchTasksProgress(penv,task_index) , DC4CGetBatchTasksError(penv,task_index) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,task_index)) , DC4CGetBatchTasksInfo(penv,task_index) );
			}
		}
		
		for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
		{
			free( a_tasks_array[envs_index] );
			DC4CCleanEnv( & (a_penv[envs_index]) );
		}
		free( a_tasks_count );
		free( a_workers_count );
		free( a_penv );
		free( a_tasks_array );
		printf( "DC4CCleanEnv ok\n" );
	}
	else
	{
		printf( "USAGE : dc4c_test_multi_batch_master rserver_ip:rserver_port,... envs_count workers_count task_count program_and_params_1 ...\n" );
		exit(7);
	}
	
	return 0;
}


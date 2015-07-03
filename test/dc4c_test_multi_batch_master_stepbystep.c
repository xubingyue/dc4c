#include "dc4c_util.h"
#include "dc4c_api.h"

/* for testing
time ./dc4c_test_multi_batch_master 192.168.6.54:12001,192.168.6.54:12002 3 -1 -100 "dc4c_test_worker_sleep -10"
*/

int main( int argc , char *argv[] )
{
	struct Dc4cApiEnv	**a_penv = NULL ;
	int			envs_count ;
	int			envs_index ;
	struct Dc4cApiEnv	*penv = NULL ;
	struct Dc4cBatchTask	*tasks_array = NULL ;
	struct Dc4cBatchTask	*p_task = NULL ;
	int			tasks_count ;
	int			workers_count ;
	int			task_index ;
	int			repeat_task_flag ;
	
	char			begin_timebuf[ 256 + 1 ] ;
	char			end_timebuf[ 256 + 1 ] ;
	
	int			nret = 0 ;
	
	DC4CSetAppLogFile( "dc4c_test_multi_batch_master_stepbystep" );
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
			
			workers_count = atoi(argv[3]) ;
			tasks_count = atoi(argv[4]) ;
			
			if( tasks_count < 0 )
			{
				tasks_count = -tasks_count ;
				repeat_task_flag = 1 ;
			}
			else
			{
				repeat_task_flag = 0 ;
			}
			
			if( workers_count < 0 )
			{
				if( workers_count == -2 )
				{
					nret = DC4CQueryWorkers( penv ) ;
					if( nret )
					{
						printf( "DC4CQueryWorkers failed[%d]\n" , nret );
						return 1;
					}
					
					workers_count = DC4CGetUnusedWorkersCount( penv ) ;
					if( workers_count <= 0 )
					{
						printf( "workers_count[%d] invalid\n" , workers_count );
						return 1;
					}
					
					tasks_count = workers_count ;
				}
				else
				{
					workers_count = tasks_count ;
				}
			}
			
			tasks_array = (struct Dc4cBatchTask *)malloc( sizeof(struct Dc4cBatchTask) * tasks_count ) ;
			if( tasks_array == NULL )
			{
				printf( "alloc failed , errno[%d]\n" , errno );
				return 1;
			}
			memset( tasks_array , 0x00 , sizeof(struct Dc4cBatchTask) * tasks_count );
			
			for( task_index = 0 ; task_index < tasks_count ; task_index++ )
			{
				p_task = & (tasks_array[task_index]) ;
				
				p_task->order_index = task_index ;
				if( repeat_task_flag == 1 )
					strcpy( p_task->program_and_params , argv[5] );
				else
					strcpy( p_task->program_and_params , argv[5+task_index] );
				p_task->timeout = DC4CGetTimeout(a_penv[envs_index]) ;
			}
			
			nret = DC4CBeginBatchTasks( a_penv[envs_index] , workers_count , tasks_array , tasks_count ) ;
			free( tasks_array );
			if( nret )
			{
				printf( "DC4CBeginBatchTasks failed[%d] , errno[%d]\n" , nret , errno );
				return 1;
			}
		}
		
		while(1)
		{
			nret = DC4CPerformMultiBatchTasks( a_penv , envs_count , & penv , & task_index ) ;
			if( nret == DC4C_INFO_TASK_FINISHED )
			{
				printf( "DC4CPerformMultiBatchTasks return DC4C_INFO_TASK_FINISHED , penv[%p]\n" , penv );
			}
			else if( nret == DC4C_INFO_BATCH_TASKS_FINISHED )
			{
				printf( "DC4CPerformMultiBatchTasks return DC4C_INFO_BATCH_TASKS_FINISHED , penv[%p]\n" , penv );
				continue;
			}
			else if( nret == DC4C_INFO_ALL_ENVS_FINISHED )
			{
				if( DC4CIsMultiBatchTasksInterrupted( a_penv , envs_count ) )
					printf( "DC4CPerformMultiBatchTasks return DC4C_INFO_ALL_ENVS_FINISHED WITH ERROR\n" );
				else
					printf( "DC4CPerformMultiBatchTasks return DC4C_INFO_ALL_ENVS_FINISHED\n" );
				break;
			}
			else if( nret == DC4C_ERROR_TIMEOUT )
			{
				printf( "DC4CPerformMultiBatchTasks return DC4C_ERROR_TIMEOUT\n" );
			}
			else if( nret == DC4C_ERROR_APP )
			{
				printf( "DC4CPerformMultiBatchTasks return DC4C_ERROR_APP\n" );
			}
			else
			{
				printf( "DC4CPerformMultiBatchTasks failed[%d] , penv[%p]\n" , nret , penv );
				break;
			}
			
			printf( "[%p][%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
				, penv , task_index , DC4CGetBatchTasksIp(penv,task_index) , DC4CGetBatchTasksPort(penv,task_index)
				, DC4CGetBatchTasksTid(penv,task_index) , DC4CGetBatchTasksProgramAndParams(penv,task_index) , DC4CGetBatchTasksTimeout(penv,task_index) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,task_index),begin_timebuf,sizeof(begin_timebuf))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,task_index),end_timebuf,sizeof(end_timebuf))+11 , DC4CGetBatchTasksElapse(penv,task_index)
				, DC4CGetBatchTasksProgress(penv,task_index) , DC4CGetBatchTasksError(penv,task_index) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,task_index)) , DC4CGetBatchTasksInfo(penv,task_index) );
		}
		
		for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
		{
			DC4CCleanEnv( & (a_penv[envs_index]) );
		}
		free( a_penv );
		printf( "DC4CCleanEnv ok\n" );
	}
	else
	{
		printf( "USAGE : dc4c_test_multi_batch_master rserver_ip:rserver_port,... envs_count workers_count task_count program_and_params_1 ...\n" );
		exit(7);
	}
	
	return 0;
}


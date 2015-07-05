#include "dc4c_util.h"
#include "dc4c_api.h"

/* for testing
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 2 3 "dc4c_test_worker_sleep 1" "dc4c_test_worker_sleep 2" "dc4c_test_worker_sleep 3"
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 4 -10 "dc4c_test_worker_sleep 10"
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 -1 -100 "dc4c_test_worker_sleep -10"
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 -2 -100 "dc4c_test_worker_sleep -10"
*/

funcDC4COnBeginTaskProc OnBeginTaskProc ;
void OnBeginTaskProc( struct Dc4cApiEnv *penv , int task_index , void *p1 , void *p2 )
{
	char			begin_timebuf[ 256 + 1 ] ;
	char			end_timebuf[ 256 + 1 ] ;
	
	printf( "%s-BEGIN -[%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
		, (char*)p1
		, task_index , DC4CGetBatchTasksIp(penv,task_index) , DC4CGetBatchTasksPort(penv,task_index)
		, DC4CGetBatchTasksTid(penv,task_index) , DC4CGetBatchTasksProgramAndParams(penv,task_index) , DC4CGetBatchTasksTimeout(penv,task_index) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,task_index),begin_timebuf,sizeof(begin_timebuf))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,task_index),end_timebuf,sizeof(end_timebuf))+11 , DC4CGetBatchTasksElapse(penv,task_index)
		, DC4CGetBatchTasksProgress(penv,task_index) , DC4CGetBatchTasksError(penv,task_index) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,task_index)) , DC4CGetBatchTasksInfo(penv,task_index) );
	return;
	return;
}

funcDC4COnCancelTaskProc OnCancelTaskProc ;
void OnCancelTaskProc( struct Dc4cApiEnv *penv , int task_index , void *p1 , void *p2 )
{
	char			begin_timebuf[ 256 + 1 ] ;
	char			end_timebuf[ 256 + 1 ] ;
	
	printf( "%s-CANCEL-[%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
		, (char*)p1
		, task_index , DC4CGetBatchTasksIp(penv,task_index) , DC4CGetBatchTasksPort(penv,task_index)
		, DC4CGetBatchTasksTid(penv,task_index) , DC4CGetBatchTasksProgramAndParams(penv,task_index) , DC4CGetBatchTasksTimeout(penv,task_index) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,task_index),begin_timebuf,sizeof(begin_timebuf))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,task_index),end_timebuf,sizeof(end_timebuf))+11 , DC4CGetBatchTasksElapse(penv,task_index)
		, DC4CGetBatchTasksProgress(penv,task_index) , DC4CGetBatchTasksError(penv,task_index) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,task_index)) , DC4CGetBatchTasksInfo(penv,task_index) );
	return;
	return;
}

funcDC4COnFinishTaskProc OnFinishTaskProc ;
void OnFinishTaskProc( struct Dc4cApiEnv *penv , int task_index , void *p1 , void *p2 )
{
	char			begin_timebuf[ 256 + 1 ] ;
	char			end_timebuf[ 256 + 1 ] ;
	
	printf( "%s-FINISH-[%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
		, (char*)p1
		, task_index , DC4CGetBatchTasksIp(penv,task_index) , DC4CGetBatchTasksPort(penv,task_index)
		, DC4CGetBatchTasksTid(penv,task_index) , DC4CGetBatchTasksProgramAndParams(penv,task_index) , DC4CGetBatchTasksTimeout(penv,task_index) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,task_index),begin_timebuf,sizeof(begin_timebuf))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,task_index),end_timebuf,sizeof(end_timebuf))+11 , DC4CGetBatchTasksElapse(penv,task_index)
		, DC4CGetBatchTasksProgress(penv,task_index) , DC4CGetBatchTasksError(penv,task_index) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,task_index)) , DC4CGetBatchTasksInfo(penv,task_index) );
	return;
}

int main( int argc , char *argv[] )
{
	struct Dc4cApiEnv	*penv = NULL ;
	struct Dc4cBatchTask	*tasks_array = NULL ;
	struct Dc4cBatchTask	*p_task = NULL ;
	int			tasks_count ;
	int			task_index ;
	int			workers_count ;
	int			repeat_task_flag ;
	int			i ;
	
	int			nret = 0 ;
	
	DC4CSetAppLogFile( "dc4c_test_batch_master_stepbystep" );
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
		
		DC4CSetTimeout( penv , 15 );
		DC4CSetOptions( penv , DC4C_OPTIONS_INTERRUPT_BY_APP );
		
		DC4CSetProcDataPtr( penv , "PROC" , NULL );
		DC4CSetOnBeginTaskProc( penv , & OnBeginTaskProc );
		DC4CSetOnCancelTaskProc( penv , & OnCancelTaskProc );
		DC4CSetOnFinishTaskProc( penv , & OnFinishTaskProc );
		
		workers_count = atoi(argv[2]) ;
		tasks_count = atoi(argv[3]) ;
		
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
		
		for( i = 0 , p_task = tasks_array ; i < tasks_count ; i++ , p_task++ )
		{
			if( repeat_task_flag == 1 )
				strcpy( p_task->program_and_params , argv[4] );
			else
				strcpy( p_task->program_and_params , argv[4+i] );
			p_task->timeout = DC4CGetTimeout(penv) ;
		}
		
		nret = DC4CBeginBatchTasks( penv , workers_count , tasks_array , tasks_count ) ;
		free( tasks_array );
		if( nret )
		{
			printf( "DC4CBeginBatchTasks failed[%d]\n" , nret );
		}
		else
		{
			printf( "DC4CBeginBatchTasks ok\n" );
		}
		
		while(1)
		{
			nret = DC4CPerformBatchTasks( penv , & task_index ) ;
			if( nret == DC4C_INFO_TASK_FINISHED )
			{
				printf( "DC4CPerformBatchTasks return DC4C_INFO_TASK_FINISHED\n" );
			}
			else if( nret == DC4C_INFO_BATCH_TASKS_FINISHED )
			{
				if( DC4CIsBatchTasksInterrupted(penv) )
					printf( "DC4CPerformBatchTasks return DC4C_INFO_BATCH_TASKS_FINISHED WITH ERROR\n" );
				else
					printf( "DC4CPerformBatchTasks return DC4C_INFO_BATCH_TASKS_FINISHED\n" );
				break;
			}
			else if( nret == DC4C_ERROR_TIMEOUT )
			{
				printf( "DC4CPerformBatchTasks return DC4C_ERROR_TIMEOUT\n" );
			}
			else if( nret == DC4C_ERROR_APP )
			{
				printf( "DC4CPerformBatchTasks return DC4C_ERROR_APP\n" );
			}
			else
			{
				printf( "DC4CPerformBatchTasks failed[%d]\n" , nret );
				break;
			}
		}
		
		DC4CCleanEnv( & penv );
		printf( "DC4CCleanEnv ok\n" );
	}
	else
	{
		printf( "USAGE : dc4c_test_batch_master rserver_ip:rserver_port,... workers_count tasks_count program_and_params_1 ...\n" );
		exit(7);
	}
	
	return 0;
}


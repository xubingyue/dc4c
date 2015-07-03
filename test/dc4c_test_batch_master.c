#include "dc4c_util.h"
#include "dc4c_api.h"

/* for testing
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 2 3 "dc4c_test_worker_sleep 1" "dc4c_test_worker_sleep 2" "dc4c_test_worker_sleep 3"
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 4 -10 "dc4c_test_worker_sleep 10"
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 -1 -100 "dc4c_test_worker_sleep -10"
time ./dc4c_test_batch_master 192.168.6.54:12001,192.168.6.54:12002 -2 -100 "dc4c_test_worker_sleep -10"
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
	
	char			begin_timebuf[ 256 + 1 ] ;
	char			end_timebuf[ 256 + 1 ] ;
	
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
		
		DC4CSetTimeout( penv , 15 );
		DC4CSetOptions( penv , DC4C_OPTIONS_INTERRUPT_BY_APP );
		
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
		
		tasks_count = DC4CGetTasksCount( penv ) ;
		for( i = 0 ; i < tasks_count ; i++ )
		{
			printf( "[%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
				, i , DC4CGetBatchTasksIp(penv,i) , DC4CGetBatchTasksPort(penv,i)
				, DC4CGetBatchTasksTid(penv,i) , DC4CGetBatchTasksProgramAndParams(penv,i) , DC4CGetBatchTasksTimeout(penv,i) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,i),begin_timebuf,sizeof(begin_timebuf))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,i),end_timebuf,sizeof(end_timebuf))+11 , DC4CGetBatchTasksElapse(penv,i)
				, DC4CGetBatchTasksProgress(penv,i) , DC4CGetBatchTasksError(penv,i) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,i)) , DC4CGetBatchTasksInfo(penv,i) );
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


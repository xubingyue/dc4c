#include "dc4c_util.h"
#include "dc4c_api.h"
#include "dc4c_tfc_dag.h"

/* for testing
time ./dc4c_test_tfc_dag_master rservers_ip_port *.dag_schedule"
*/

funcDC4COnBeginDagBatchProc OnBeginDagBatchProc ;
void OnBeginDagBatchProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , void *p1 )
{
	printf( "%s-BEGINBATCH-[%s][%p]\n"
		, (char*)p1
		, DC4CGetDagBatchName(p_batch) , DC4CGetDagBatchApiEnvPtr(p_batch) );
	return;
}

funcDC4COnFinishDagBatchProc OnFinishDagBatchProc ;
void OnFinishDagBatchProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , void *p1 )
{
	printf( "%s-FINISHBATCH-[%s][%p]\n"
		, (char*)p1
		, DC4CGetDagBatchName(p_batch) , DC4CGetDagBatchApiEnvPtr(p_batch) );
	return;
}

funcDC4COnBeginDagBatchTaskProc OnBeginDagBatchTaskProc ;
void OnBeginDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , int task_index , void *p1 )
{
	char			begin_timebuf[ 256 + 1 ] ;
	char			end_timebuf[ 256 + 1 ] ;
	
	printf( "%s-BEGIN -[%s][%s]-[%p][%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
		, (char*)p1
		, DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch)
		, penv , task_index
		, DC4CGetBatchTasksIp(penv,task_index) , DC4CGetBatchTasksPort(penv,task_index)
		, DC4CGetBatchTasksTid(penv,task_index) , DC4CGetBatchTasksProgramAndParams(penv,task_index) , DC4CGetBatchTasksTimeout(penv,task_index) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,task_index),begin_timebuf,sizeof(begin_timebuf))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,task_index),end_timebuf,sizeof(end_timebuf))+11 , DC4CGetBatchTasksElapse(penv,task_index)
		, DC4CGetBatchTasksProgress(penv,task_index) , DC4CGetBatchTasksError(penv,task_index) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,task_index)) , DC4CGetBatchTasksInfo(penv,task_index) );
	return;
}

funcDC4COnCancelDagBatchTaskProc OnCancelDagBatchTaskProc ;
void OnCancelDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , int task_index , void *p1 )
{
	char			begin_timebuf[ 256 + 1 ] ;
	char			end_timebuf[ 256 + 1 ] ;
	
	printf( "%s-CANCEL-[%s][%s]-[%p][%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
		, (char*)p1
		, DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch)
		, penv , task_index
		, DC4CGetBatchTasksIp(penv,task_index) , DC4CGetBatchTasksPort(penv,task_index)
		, DC4CGetBatchTasksTid(penv,task_index) , DC4CGetBatchTasksProgramAndParams(penv,task_index) , DC4CGetBatchTasksTimeout(penv,task_index) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,task_index),begin_timebuf,sizeof(begin_timebuf))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,task_index),end_timebuf,sizeof(end_timebuf))+11 , DC4CGetBatchTasksElapse(penv,task_index)
		, DC4CGetBatchTasksProgress(penv,task_index) , DC4CGetBatchTasksError(penv,task_index) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,task_index)) , DC4CGetBatchTasksInfo(penv,task_index) );
	return;
}

funcDC4COnFinishDagBatchTaskProc OnFinishDagBatchTaskProc ;
void OnFinishDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , int task_index , void *p1 )
{
	char			begin_timebuf[ 256 + 1 ] ;
	char			end_timebuf[ 256 + 1 ] ;
	
	printf( "%s-FINISH-[%s][%s]-[%p][%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
		, (char*)p1
		, DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch)
		, penv , task_index
		, DC4CGetBatchTasksIp(penv,task_index) , DC4CGetBatchTasksPort(penv,task_index)
		, DC4CGetBatchTasksTid(penv,task_index) , DC4CGetBatchTasksProgramAndParams(penv,task_index) , DC4CGetBatchTasksTimeout(penv,task_index) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,task_index),begin_timebuf,sizeof(begin_timebuf))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,task_index),end_timebuf,sizeof(end_timebuf))+11 , DC4CGetBatchTasksElapse(penv,task_index)
		, DC4CGetBatchTasksProgress(penv,task_index) , DC4CGetBatchTasksError(penv,task_index) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,task_index)) , DC4CGetBatchTasksInfo(penv,task_index) );
	return;
}

static int TestDagSchedule( char *dag_schedule_pathfilename , char *rservers_ip_port )
{
	struct Dc4cDagSchedule	*p_sched = NULL ;
	struct Dc4cDagBatch	*p_batch = NULL ;
	struct Dc4cApiEnv	*penv = NULL ;
	int			task_index ;
	
	int			perform_return = 0 ;
	int			nret = 0 ;
	
	nret = DC4CLoadDagScheduleFromFile( & p_sched , dag_schedule_pathfilename , rservers_ip_port , 0 ) ;
	if( nret )
	{
		printf( "DC4CLoadDagScheduleFromFile failed[%d]\n" , nret );
		return -1;
	}
	else
	{
		printf( "DC4CLoadDagScheduleFromFile ok\n" );
	}
	
	DC4CLogDagSchedule( p_sched );
	
	DC4CSetDagBatchProcDataPtr( p_sched  , "PROC" );
	DC4CSetOnBeginDagBatchProc( p_sched  , & OnBeginDagBatchProc );
	DC4CSetOnFinishDagBatchProc( p_sched  , & OnFinishDagBatchProc );
	DC4CSetOnBeginDagBatchTaskProc( p_sched  , & OnBeginDagBatchTaskProc );
	DC4CSetOnCancelDagBatchTaskProc( p_sched  , & OnCancelDagBatchTaskProc );
	DC4CSetOnFinishDagBatchTaskProc( p_sched  , & OnFinishDagBatchTaskProc );
	
	nret = DC4CBeginDagSchedule( p_sched ) ;
	if( nret )
		return nret;
	
	while(1)
	{
		perform_return = DC4CPerformDagSchedule( p_sched , & p_batch , & penv , & task_index ) ;
		if( perform_return == DC4C_INFO_TASK_FINISHED )
		{
			printf( "DC4CPerformDagSchedule return DC4C_INFO_TASK_FINISHED , batch_name[%s] task_index[%d]\n" , DC4CGetDagBatchName(p_batch) , task_index );
		}
		else if( perform_return == DC4C_INFO_BATCH_TASKS_FINISHED )
		{
			printf( "DC4CPerformDagSchedule return DC4C_INFO_BATCH_TASKS_FINISHED , batch_name[%s]\n" , DC4CGetDagBatchName(p_batch) );
		}
		else if( perform_return == DC4C_INFO_ALL_ENVS_FINISHED )
		{
			if( DC4CDagScheduleInterruptCode(p_sched) )
			{
				printf( "DC4CPerformDagSchedule return DC4C_DAGSCHEDULE_PROGRESS_FINISHED_WITH_ERROR\n" );
				break;
			}
			else
			{
				printf( "DC4CPerformDagSchedule return DC4C_INFO_ALL_ENVS_FINISHED\n" );
				break;
			}
		}
		else if( perform_return == DC4C_ERROR_TIMEOUT )
		{
			printf( "DC4CPerformDagSchedule return DC4C_ERROR_TIMEOUT\n" );
		}
		else if( perform_return == DC4C_ERROR_APP )
		{
			printf( "DC4CPerformDagSchedule failed[%d] , batch_name[%s]\n" , perform_return , DC4CGetDagBatchName(p_batch) );
		}
		else
		{
			printf( "DC4CPerformDagSchedule failed[%d]\n" , nret );
		}
	}
	
	DC4CUnloadDagSchedule( & p_sched );
	printf( "DC4CUnloadDagSchedule ok\n" );
	
	return 0;
}

int main( int argc , char *argv[] )
{
	DC4CSetAppLogFile( "dc4c_test_tfc_dag_master" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	if( argc == 1 + 2 )
	{
		return TestDagSchedule( argv[2] , argv[1] );
	}
	else
	{
		printf( "USAGE : dc4c_test_tfc_dag_master rservers_ip_port .dag_schedule\n" );
		exit(7);
	}
}


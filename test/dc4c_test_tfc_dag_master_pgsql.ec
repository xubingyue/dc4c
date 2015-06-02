#include "dc4c_api.h"
#include "dc4c_tfc_dag.h"
#include "IDL_dag_schedule.dsc.ESQL.eh"
#include "IDL_dag_batches_info.dsc.ESQL.eh"
#include "IDL_dag_batches_tasks.dsc.ESQL.eh"
#include "IDL_dag_batches_direction.dsc.ESQL.eh"

#include "IDL_dag_schedule.dsc.LOG.c"
#include "IDL_dag_batches_info.dsc.LOG.c"
#include "IDL_dag_batches_direction.dsc.LOG.c"
#include "IDL_dag_batches_tasks.dsc.LOG.c"

/* for testing
time ./dc4c_test_tfc_dag_master rservers_ip_port schedule_name_from_database"
*/

static int TestDagSchedule( char *schedule_name , char *rservers_ip_port )
{
	struct Dc4cDagSchedule	*p_sched = NULL ;
	struct Dc4cDagBatch	*p_batch = NULL ;
	struct Dc4cApiEnv	*penv = NULL ;
	
	int			perform_return ;
	dag_schedule		schedule ;
	dag_batches_info	batches_info ;
	dag_batches_tasks	batches_tasks ;
	int			i ;
	int			tasks_count ;
	char			begin_datetime[ 19 + 1 ] ;
	char			end_datetime[ 19 + 1 ] ;
	
	int			nret = 0 ;
	
	nret = DC4CLoadDagScheduleFromDatabase( & p_sched , schedule_name , rservers_ip_port , DC4C_OPTIONS_INTERRUPT_BY_APP ) ;
	if( nret )
	{
		printf( "DC4CLoadDagScheduleFromDatabase failed[%d]\n" , nret );
		return -1;
	}
	else
	{
		printf( "DC4CLoadDagScheduleFromDatabase ok\n" );
	}
	
	if( DC4CGetDagScheduleProgress(p_sched) == DC4C_DAGSCHEDULE_PROGRESS_FINISHED )
	{
		printf( "schedule status[%d] , can't running\n" , DC4CGetDagScheduleProgress(p_sched) );
		DC4CUnloadDagSchedule( & p_sched );
		return 0;
	}
	
	DSCDBBEGINWORK();
	
	memset( & schedule , 0x00 , sizeof(dag_schedule) );
	strncpy( schedule.schedule_name , DC4CGetDagScheduleName(p_sched) , sizeof(schedule.schedule_name)-1 );
	strncpy( schedule.begin_datetime , GetTimeStringNow(begin_datetime,sizeof(begin_datetime)) , sizeof(schedule.begin_datetime)-1 );
	schedule.progress = DC4C_DAGSCHEDULE_PROGRESS_EXECUTING ;
	DSCSQLACTION_UPDATE_dag_schedule_SET_begin_datetime_progress_WHERE_schedule_name_E( & schedule );
	if( SQLCODE )
	{
		printf( "DSCSQLACTION_UPDATE_dag_schedule_SET_begin_datetime_progress_WHERE_schedule_name_E failed , SQLCODE[%d][%s][%s]\n" , SQLCODE , SQLSTATE , SQLDESC );
		DSCLOG_dag_schedule( & schedule );
		DSCDBROLLBACK();
		DC4CUnloadDagSchedule( & p_sched );
		return DC4C_ERROR_DATABASE;
	}
	
	DSCDBCOMMIT();
	
	DC4CLogDagSchedule( p_sched );
	
	nret = DC4CBeginDagSchedule( p_sched ) ;
	if( nret )
	{
		printf( "DC4CBeginDagSchedule failed[%d]\n" , nret );
		return -1;
	}
	else
	{
		printf( "DC4CBeginDagSchedule ok\n" );
	}
	
	while(1)
	{
		perform_return = DC4CPerformDagSchedule( p_sched , & p_batch ) ;
		if( perform_return == DC4C_INFO_NO_UNFINISHED_ENVS )
		{
			printf( "DC4CPerformDagSchedule ok , all is done\n" );
			break;
		}
		else if( perform_return == DC4C_ERROR_APP )
		{
			printf( "DC4CPerformDagSchedule ok , batch_name[%s] return error\n" , DC4CGetDagBatchName(p_batch) );
		}
		else if( perform_return )
		{
			printf( "DC4CPerformDagSchedule failed[%d] , batch_name[%s]\n" , perform_return , DC4CGetDagBatchName(p_batch) );
			return perform_return;
		}
		else
		{
			printf( "DC4CPerformDagSchedule ok , batch_name[%s]\n" , DC4CGetDagBatchName(p_batch) );
		}
		
		DSCDBBEGINWORK();
		
		memset( & batches_info , 0x00 , sizeof(dag_batches_info) );
		strncpy( batches_info.schedule_name , DC4CGetDagScheduleName(p_sched) , sizeof(batches_info.schedule_name)-1 );
		strncpy( batches_info.batch_name , DC4CGetDagBatchName(p_batch) , sizeof(batches_info.batch_name)-1 );
		strncpy( batches_info.begin_datetime , DC4CGetDagBatchBeginDatetime(p_batch) , sizeof(batches_info.begin_datetime)-1 );
		strncpy( batches_info.end_datetime , DC4CGetDagBatchEndDatetime(p_batch) , sizeof(batches_info.end_datetime)-1 );
		batches_info.progress = (perform_return?DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR:DC4C_DAGBATCH_PROGRESS_FINISHED) ;
		DSCSQLACTION_UPDATE_dag_batches_info_SET_begin_datetime_end_datetime_progress_WHERE_schedule_name_E_AND_batch_name_E( & batches_info );
		if( SQLCODE )
		{
			printf( "DSCSQLACTION_UPDATE_dag_batches_info_SET_begin_datetime_end_datetime_progress_WHERE_schedule_name_E_AND_batch_name_E failed , SQLCODE[%d][%s][%s]\n" , SQLCODE , SQLSTATE , SQLDESC );
			DSCLOG_dag_batches_info( & batches_info );
			DSCDBROLLBACK();
			DC4CUnloadDagSchedule( & p_sched );
			return DC4C_ERROR_DATABASE;
		}
		
		penv = DC4CGetDagBatchApiEnvPtr( p_batch ) ;
		tasks_count = DC4CGetTasksCount( penv ) ;
		for( i = 0 ; i < tasks_count ; i++ )
		{
			printf( "[%d]-[%s][%ld]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%s]\n"
				, i , DC4CGetBatchTasksIp(penv,i) , DC4CGetBatchTasksPort(penv,i)
				, DC4CGetBatchTasksTid(penv,i) , DC4CGetBatchTasksProgramAndParams(penv,i) , DC4CGetBatchTasksTimeout(penv,i) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,i),begin_datetime,sizeof(begin_datetime))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,i),end_datetime,sizeof(end_datetime))+11 , DC4CGetBatchTasksElapse(penv,i)
				, DC4CGetBatchTasksError(penv,i) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,i)) , DC4CGetBatchTasksInfo(penv,i) );
			
			memset( & batches_tasks , 0x00 , sizeof(dag_batches_tasks) );
			strncpy( batches_tasks.schedule_name , DC4CGetDagScheduleName(p_sched) , sizeof(batches_tasks.schedule_name)-1 );
			strncpy( batches_tasks.batch_name , DC4CGetDagBatchName(p_batch) , sizeof(batches_tasks.batch_name)-1 );
			batches_tasks.order_index = DC4CGetBatchTasksOrderIndex(penv,i) ;
			strncpy( batches_tasks.program_and_params , DC4CGetBatchTasksProgramAndParams(penv,i) , sizeof(batches_tasks.program_and_params)-1 );
			ConvertTimeString( DC4CGetBatchTasksBeginTimestamp(penv,i) , begin_datetime , sizeof(begin_datetime) );
			strncpy( batches_tasks.begin_datetime , begin_datetime , sizeof(batches_tasks.begin_datetime)-1 );
			ConvertTimeString( DC4CGetBatchTasksEndTimestamp(penv,i) , end_datetime , sizeof(end_datetime) );
			strncpy( batches_tasks.begin_datetime , end_datetime , sizeof(batches_tasks.begin_datetime)-1 );
			if( DC4CGetBatchTasksError(penv,i) == 0 && DC4CGetBatchTasksStatus(penv,i) == 0 )
				batches_tasks.progress = DC4C_DAGTASK_PROGRESS_FINISHED ;
			else
				batches_tasks.progress = DC4C_DAGTASK_PROGRESS_FINISHED_WITH_ERROR ;
			batches_tasks.error = DC4CGetBatchTasksError(penv,i) ;
			batches_tasks.status = DC4CGetBatchTasksStatus(penv,i) ;
			DSCSQLACTION_UPDATE_dag_batches_tasks_SET_begin_datetime_end_datetime_progress_error_status_WHERE_schedule_name_E_AND_batch_name_E_AND_order_index_E( & batches_tasks );
			if( SQLCODE )
			{
				printf( "DSCSQLACTION_UPDATE_dag_batches_tasks_SET_begin_datetime_end_datetime_progress_error_status_WHERE_schedule_name_E_AND_batch_name_E_AND_order_index_E failed , SQLCODE[%d][%s][%s]\n" , SQLCODE , SQLSTATE , SQLDESC );
				DSCLOG_dag_batches_tasks( & batches_tasks );
				DSCDBROLLBACK();
				DC4CUnloadDagSchedule( & p_sched );
				return DC4C_ERROR_DATABASE;
			}
		}
		
		DSCDBCOMMIT();
		
		if( perform_return )
			break;
	}
	
	DSCDBBEGINWORK();
	
	memset( & schedule , 0x00 , sizeof(dag_schedule) );
	strncpy( schedule.schedule_name , DC4CGetDagScheduleName(p_sched) , sizeof(schedule.schedule_name)-1 );
	strncpy( schedule.end_datetime , GetTimeStringNow(end_datetime,sizeof(end_datetime)) , sizeof(schedule.end_datetime)-1 );
	schedule.progress = (DC4CGetDagScheduleProgress(p_sched)==DC4C_DAGSCHEDULE_PROGRESS_FINISHED?DC4C_DAGSCHEDULE_PROGRESS_FINISHED:DC4C_DAGSCHEDULE_PROGRESS_FINISHED_WITH_ERROR) ;
	DSCSQLACTION_UPDATE_dag_schedule_SET_end_datetime_progress_WHERE_schedule_name_E( & schedule );
	if( SQLCODE )
	{
		printf( "DSCSQLACTION_UPDATE_dag_schedule_SET_end_datetime_progress_WHERE_schedule_name_E failed , SQLCODE[%d][%s][%s]\n" , SQLCODE , SQLSTATE , SQLDESC );
		DSCLOG_dag_schedule( & schedule );
		DSCDBROLLBACK();
		DC4CUnloadDagSchedule( & p_sched );
		return DC4C_ERROR_DATABASE;
	}
	
	DSCDBCOMMIT();
	
	DC4CUnloadDagSchedule( & p_sched );
	printf( "DC4CUnloadDagSchedule ok\n" );
	
	return 0;
}

int main( int argc , char *argv[] )
{
	int		nret = 0 ;
	
	DC4CSetAppLogFile( "dc4c_test_tfc_dag_master_pgsql" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	if( argc == 1 + 2 )
	{
		DSCDBCONN( getenv("DBHOST") , atoi(getenv("DBPORT")) , getenv("DBNAME") , getenv("DBUSER") , getenv("DBPASS") );
		if( SQLCODE )
		{
			printf( "DSCDBCONN failed , SQLCODE[%d][%s][%s]\n" , SQLCODE , SQLSTATE , SQLDESC );
			return 1;
		}
		else
		{
			printf( "DSCDBCONN ok\n" );
		}
		
		nret = TestDagSchedule( argv[2] , argv[1] ) ;
		
		DSCDBDISCONN();
		printf( "DSCDBDISCONN ok\n" );
		
		return -nret;
	}
	else
	{
		printf( "USAGE : dc4c_test_tfc_dag_master rservers_ip_port .dag_schedule\n" );
		exit(7);
	}
}


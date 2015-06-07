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

sqls dag_schedule ; sqls dag_batches_info ; sqls dag_batches_tasks
*/

static int DC4CLoadDagScheduleFromDatabase( struct Dc4cDagSchedule **pp_sched , char *schedule_name , char *rservers_ip_port , int options )
{
	dag_schedule_configfile *p_config = NULL ;
	
	dag_schedule		schedule ;
	dag_batches_info	batches_info ;
	dag_batches_tasks	batches_tasks ;
	dag_batches_direction	batches_direction ;
	char			begin_datetime[ 19 + 1 ] ;
	
	int			nret = 0 ;
	
	p_config = (dag_schedule_configfile *)malloc( sizeof(dag_schedule_configfile) ) ;
	if( p_config == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return DC4C_ERROR_ALLOC;
	}
	
	DSCINIT_dag_schedule_configfile( p_config );
	
	memset( & schedule , 0x00 , sizeof(dag_schedule) );
	strncpy( schedule.schedule_name , schedule_name , sizeof(schedule.schedule_name)-1 );
	DSCSQLACTION_SELECT_A_FROM_dag_schedule_WHERE_schedule_name_E( & schedule );
	if( SQLCODE )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_SELECT_A_FROM_dag_schedule_WHERE_schedule_name_E failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
		free( p_config );
		return DC4C_ERROR_DATABASE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_SELECT_A_FROM_dag_schedule_WHERE_schedule_name_E ok" );
	}
	
	if( schedule.progress == DC4C_DAGSCHEDULE_PROGRESS_FINISHED )
	{
		InfoLog( __FILE__ , __LINE__ , "schedule status[%d] , all is done" , schedule.progress );
		free( p_config );
		return 0;
	}
	
	DSCDBBEGINWORK();
	
	strncpy( schedule.begin_datetime , GetTimeStringNow(begin_datetime,sizeof(begin_datetime)) , sizeof(schedule.begin_datetime)-1 );
	schedule.progress = DC4C_DAGSCHEDULE_PROGRESS_EXECUTING ;
	DSCSQLACTION_UPDATE_dag_schedule_SET_begin_datetime_progress_WHERE_schedule_name_E( & schedule );
	if( SQLCODE )
	{
		printf( "DSCSQLACTION_UPDATE_dag_schedule_SET_begin_datetime_progress_WHERE_schedule_name_E failed , SQLCODE[%d][%s][%s]\n" , SQLCODE , SQLSTATE , SQLDESC );
		free( p_config );
		return DC4C_ERROR_DATABASE;
	}
	
	DSCDBCOMMIT();
	
	strncpy( p_config->schedule.schedule_name , schedule.schedule_name , sizeof(p_config->schedule.schedule_name)-1 );
	strncpy( p_config->schedule.schedule_desc , schedule.schedule_desc , sizeof(p_config->schedule.schedule_desc)-1 );
	strncpy( p_config->schedule.begin_datetime , schedule.begin_datetime , sizeof(p_config->schedule.begin_datetime)-1 );
	strncpy( p_config->schedule.end_datetime , schedule.end_datetime , sizeof(p_config->schedule.end_datetime)-1 );
	p_config->schedule.progress = schedule.progress ;
	
	memset( & batches_info , 0x00 , sizeof(dag_batches_info) );
	strncpy( batches_info.schedule_name , schedule.schedule_name , sizeof(batches_info.schedule_name)-1 );
	DSCSQLACTION_OPEN_CURSOR_dag_batches_info_cursor1_SELECT_A_FROM_dag_batches_info_WHERE_schedule_name_E( & batches_info );
	if( SQLCODE )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_info_cursor1_SELECT_A_FROM_dag_batches_info_WHERE_schedule_name_E failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
		free( p_config );
		return DC4C_ERROR_DATABASE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_info_cursor1_SELECT_A_FROM_dag_batches_info_WHERE_schedule_name_E ok" );
	}
	
	for(	p_config->batches._batches_info_count = 0
		; p_config->batches._batches_info_count < p_config->batches._batches_info_size
		; p_config->batches._batches_info_count++ )
	{
		DSCSQLACTION_FETCH_CURSOR_dag_batches_info_cursor1( & batches_info );
		if( SQLCODE == SQLNOTFOUND )
		{
			break;
		}
		else if( SQLCODE )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_info_cursor1 failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
			free( p_config );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_info_cursor1 ok" );
		}
		
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].batch_name , batches_info.batch_name , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].batch_name)-1 );
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].batch_desc , batches_info.batch_desc , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].batch_desc)-1 );
		p_config->batches.batches_info[p_config->batches._batches_info_count].view_pos_x = batches_info.view_pos_x ;
		p_config->batches.batches_info[p_config->batches._batches_info_count].view_pos_y = batches_info.view_pos_y ;
		
		memset( & batches_tasks , 0x00 , sizeof(dag_batches_tasks) );
		strncpy( batches_tasks.schedule_name , schedule.schedule_name , sizeof(batches_tasks.schedule_name)-1 );
		strncpy( batches_tasks.batch_name , batches_info.batch_name , sizeof(batches_tasks.batch_name)-1 );
		DSCSQLACTION_OPEN_CURSOR_dag_batches_tasks_cursor2_SELECT_A_FROM_dag_batches_tasks_WHERE_schedule_name_E_AND_batch_name_E_ORDER_BY_order_index( & batches_tasks );
		if( SQLCODE )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_tasks_cursor2_SELECT_A_FROM_dag_batches_tasks_WHERE_schedule_name_E_AND_batch_name_E_ORDER_BY_order_index failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
			free( p_config );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_tasks_cursor2_SELECT_A_FROM_dag_batches_tasks_WHERE_schedule_name_E_AND_batch_name_E_ORDER_BY_order_index ok" );
		}
		
		for(	p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count = 0
			; p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count < p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_size
			; )
		{
			DSCSQLACTION_FETCH_CURSOR_dag_batches_tasks_cursor2( & batches_tasks );
			if( SQLCODE == SQLNOTFOUND )
			{
				break;
			}
			else if( SQLCODE )
			{
				ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_tasks_cursor2 failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
				free( p_config );
				return DC4C_ERROR_DATABASE;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_tasks_cursor2 ok" );
			}
			
			if( batches_tasks.progress == DC4C_DAGTASK_PROGRESS_FINISHED )
				continue;
			
			strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].program_and_params , batches_tasks.program_and_params , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].program_and_params)-1 );
			p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].timeout = batches_tasks.timeout ;
			p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].order_index = batches_tasks.order_index ;
			p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].progress = batches_tasks.progress ;
			
			p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count++;
		}
		
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].begin_datetime , batches_info.begin_datetime , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].begin_datetime)-1 );
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].end_datetime , batches_info.end_datetime , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].end_datetime)-1 );
		p_config->batches.batches_info[p_config->batches._batches_info_count].progress = batches_info.progress ;
		
		DSCSQLACTION_CLOSE_CURSOR_dag_batches_tasks_cursor2();
		DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_CLOSE_CURSOR_dag_batches_tasks_cursor2 ok" );
	}
	
	DSCSQLACTION_CLOSE_CURSOR_dag_batches_info_cursor1();
	DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_CLOSE_CURSOR_dag_batches_info_cursor1 ok" );
	
	memset( & batches_direction , 0x00 , sizeof(dag_batches_direction) );
	strncpy( batches_direction.schedule_name , schedule.schedule_name , sizeof(batches_direction.schedule_name)-1 );
	DSCSQLACTION_OPEN_CURSOR_dag_batches_direction_cursor1_SELECT_A_FROM_dag_batches_direction_WHERE_schedule_name_E( & batches_direction );
	if( SQLCODE )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_direction_cursor1_SELECT_A_FROM_dag_batches_direction_WHERE_schedule_name_E failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
		free( p_config );
		return DC4C_ERROR_DATABASE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_direction_cursor1_SELECT_A_FROM_dag_batches_direction_WHERE_schedule_name_E ok" );
	}
	
	for(	p_config->batches._batches_direction_count = 0
		; p_config->batches._batches_direction_count < p_config->batches._batches_direction_size
		; p_config->batches._batches_direction_count++ )
	{
		DSCSQLACTION_FETCH_CURSOR_dag_batches_direction_cursor1( & batches_direction );
		if( SQLCODE == SQLNOTFOUND )
		{
			break;
		}
		else if( SQLCODE )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_direction_cursor1 failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
			free( p_config );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_direction_cursor1 ok" );
		}
		
		strncpy( p_config->batches.batches_direction[p_config->batches._batches_direction_count].from_batch , batches_direction.from_batch , sizeof(p_config->batches.batches_direction[p_config->batches._batches_direction_count].from_batch)-1 );
		strncpy( p_config->batches.batches_direction[p_config->batches._batches_direction_count].to_batch , batches_direction.to_batch , sizeof(p_config->batches.batches_direction[p_config->batches._batches_direction_count].to_batch)-1 );
	}
	
	DSCSQLACTION_CLOSE_CURSOR_dag_batches_direction_cursor1();
	DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_CLOSE_CURSOR_dag_batches_direction_cursor1 ok" );
	
	nret = DC4CLoadDagScheduleFromStruct( pp_sched , p_config , rservers_ip_port , options ) ;
	free( p_config );
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CLoadDagScheduleFromStruct failed[%d]" , nret );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CLoadDagScheduleFromStruct ok" );
	}
	
	return 0;
}

static int DC4CUpdateBatchTasks( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv )
{
	int			tasks_count ;
	int			i ;
	dag_batches_info	batches_info ;
	dag_batches_tasks	batches_tasks ;
	char			begin_datetime[ 19 + 1 ] ;
	char			end_datetime[ 19 + 1 ] ;
	
	DSCDBBEGINWORK();
	
	memset( & batches_info , 0x00 , sizeof(dag_batches_info) );
	strncpy( batches_info.schedule_name , DC4CGetDagScheduleName(p_sched) , sizeof(batches_info.schedule_name)-1 );
	strncpy( batches_info.batch_name , DC4CGetDagBatchName(p_batch) , sizeof(batches_info.batch_name)-1 );
	strncpy( batches_info.begin_datetime , DC4CGetDagBatchBeginDatetime(p_batch) , sizeof(batches_info.begin_datetime)-1 );
	strncpy( batches_info.end_datetime , DC4CGetDagBatchEndDatetime(p_batch) , sizeof(batches_info.end_datetime)-1 );
	batches_info.progress = DC4CGetDagBatchProgress(p_batch) ;
	DSCSQLACTION_UPDATE_dag_batches_info_SET_begin_datetime_end_datetime_progress_WHERE_schedule_name_E_AND_batch_name_E( & batches_info );
	if( SQLCODE )
	{
		printf( "DSCSQLACTION_UPDATE_dag_batches_info_SET_begin_datetime_end_datetime_progress_WHERE_schedule_name_E_AND_batch_name_E failed , SQLCODE[%d][%s][%s]\n" , SQLCODE , SQLSTATE , SQLDESC );
		DSCLOG_dag_batches_info( & batches_info );
		DSCDBROLLBACK();
		DC4CUnloadDagSchedule( & p_sched );
		return DC4C_ERROR_DATABASE;
	}
	
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
		strncpy( batches_tasks.end_datetime , end_datetime , sizeof(batches_tasks.end_datetime)-1 );
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
	
	return 0;
}

static int DC4CUnloadDagScheduleToDatabase( struct Dc4cDagSchedule **pp_sched )
{
	dag_schedule		schedule ;
	char			end_datetime[ 19 + 1 ] ;
	
	DSCDBBEGINWORK();
	
	memset( & schedule , 0x00 , sizeof(dag_schedule) );
	strncpy( schedule.schedule_name , DC4CGetDagScheduleName(*pp_sched) , sizeof(schedule.schedule_name)-1 );
	strncpy( schedule.end_datetime , GetTimeStringNow(end_datetime,sizeof(end_datetime)) , sizeof(schedule.end_datetime)-1 );
	schedule.progress = (DC4CGetDagScheduleProgress(*pp_sched)==DC4C_DAGSCHEDULE_PROGRESS_FINISHED?DC4C_DAGSCHEDULE_PROGRESS_FINISHED:DC4C_DAGSCHEDULE_PROGRESS_FINISHED_WITH_ERROR) ;
	DSCSQLACTION_UPDATE_dag_schedule_SET_end_datetime_progress_WHERE_schedule_name_E( & schedule );
	if( SQLCODE )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_UPDATE_dag_schedule_SET_end_datetime_progress_WHERE_schedule_name_E failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
		DSCLOG_dag_schedule( & schedule );
		DSCDBROLLBACK();
		DC4CUnloadDagSchedule( pp_sched );
		return DC4C_ERROR_DATABASE;
	}
	
	DSCDBCOMMIT();
	
	DC4CUnloadDagSchedule( pp_sched );
	ErrorLog( __FILE__ , __LINE__ , "DC4CUnloadDagSchedule ok" );
	
	return 0;
}

static int TestDagSchedule( char *schedule_name , char *rservers_ip_port )
{
	struct Dc4cDagSchedule	*p_sched = NULL ;
	struct Dc4cDagBatch	*p_batch = NULL ;
	struct Dc4cApiEnv	*penv	= NULL ;
	int			task_index ;
	
	int			perform_return = 0 ;
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
	
	if( p_sched == NULL )
	{
		printf( "All is done\n" );
		return 0;
	}
	
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
		perform_return = DC4CPerformDagSchedule( p_sched , & p_batch , & penv , & task_index ) ;
		if( perform_return == DC4C_INFO_TASK_FINISHED )
		{
			printf( "DC4CPerformDagSchedule return DC4C_INFO_TASK_FINISHED , batch_name[%s] task_index[%d]\n" , DC4CGetDagBatchName(p_batch) , task_index );
			continue;
		}
		else if( perform_return == DC4C_INFO_BATCH_TASKS_FINISHED )
		{
			printf( "DC4CPerformDagSchedule return DC4C_INFO_BATCH_TASKS_FINISHED , batch_name[%s]\n" , DC4CGetDagBatchName(p_batch) );
		}
		else if( perform_return == DC4C_INFO_ALL_ENVS_FINISHED )
		{
			printf( "DC4CPerformDagSchedule return DC4C_INFO_ALL_ENVS_FINISHED\n" );
			break;
		}
		else if( perform_return == DC4C_ERROR_TIMEOUT )
		{
			printf( "DC4CPerformDagSchedule return DC4C_ERROR_TIMEOUT\n" );
			break;
		}
		else if( perform_return == DC4C_ERROR_APP )
		{
			printf( "DC4CPerformDagSchedule return DC4C_ERROR_APP\n" );
			break;
		}
		else
		{
			printf( "DC4CPerformDagSchedule failed[%d] , batch_name[%s]\n" , perform_return , DC4CGetDagBatchName(p_batch) );
			break;
		}
		
		nret = DC4CUpdateBatchTasks( p_sched , p_batch , penv );
		if( nret )
		{
			printf( "DC4CUpdateBatchTasks failed[%d]\n" , nret );
			break;
		}
		else
		{
			printf( "DC4CUpdateBatchTasks ok\n" );
		}
	}
	
	nret = DC4CUnloadDagScheduleToDatabase( & p_sched ) ;
	if( nret )
	{
		printf( "DC4CUnloadDagScheduleToDatabase failed[%d]\n" , nret );
		return -1;
	}
	else
	{
		printf( "DC4CUnloadDagScheduleToDatabase ok\n" );
	}
	
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


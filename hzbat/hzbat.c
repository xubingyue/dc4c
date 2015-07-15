#include "dc4c_api.h"
#include "dc4c_tfc_dag.h"

#include <time.h>

#include "hzb_log.h"

#include "businessdb.h"

#include "DB_Pm_Hzbat_Schedule.h"
#include "DB_Pm_Hzbat_Batches_Info.h"
#include "DB_Pm_Hzbat_Batches_Direction.h"
#include "DB_Pm_Hzbat_Batches_Filter.h"
#include "DB_Pm_Hzbat_Batches_Tasks.h"

int DB_pm_hzbat_batches_info_debug_print(char *reason,Pm_Hzbat_Batches_Info *adata, char *filename, int line_no);

/* for testing
sql "drop table pm_hzbat_schedule"
sql "drop table pm_hzbat_batches_info"
sql "drop table pm_hzbat_batches_direction"
sql "drop table pm_hzbat_batches_filter"
sql "drop table pm_hzbat_batches_tasks"

time hzbat "2015-07-01" POIT_STAD
time hzbat "2015-07-01" POIT_CALC
time hzbat "2015-07-01" POIT_RPT

sqlfile hzbat_init.sql ; rmlog2 ; hzbat_all_schedules.sh
sqlfile hzbat_reset.sql ; rmlog2 ; hzbat_all_schedules.sh

msqls pm_hzbat_schedule
msqls pm_hzbat_batches_info
msqls pm_hzbat_batches_tasks

msqls pm_hzbat_schedule ; msqls pm_hzbat_batches_info ; msqls pm_hzbat_batches_tasks

sql "SELECT DATE_FORMAT(a.business_date,'%Y-%m-%d') FROM pmp_spma_para a WHERE a.system_id='hzbank'"
sql "update pmp_spma_para a set a.business_date=str_to_date('2016-07-06','%Y-%m-%d') WHERE a.system_id='hzbank'"

sql "update ftp_rar_para set run_flag=1 where table_name='ZHJF_EVENT'"
sql "update ftp_rar_para set run_flag=3 where table_name='ZHJF_EVENT'"
sql "update ftp_rar_para set busi_date='2015-07-09'"

sqlfile hzbat_init.sql ; rmlog2 ; valgrind --leak-check=full hzbat 2015-07-07 POIT_DAYE POTHD001

*/

#define IS_LEAPYEAR(_year_)	( ( (_year_%4==0&&_year_%100!=0)||(_year_%400==0) ) ? 1 : 0 )

static int DaysOfMonth[2][13] =
	{     
		{ 0 , 31 , 28 , 31 , 30 , 31 , 30 , 31 , 31 , 30 , 31 , 30 , 31 } ,
		{ 0 , 31 , 29 , 31 , 30 , 31 , 30 , 31 , 31 , 30 , 31 , 30 , 31 }
	} ; 

static int CheckFilterRule( char *business_date , Pm_Hzbat_Batches_Filter *p_batches_filter )
{
	struct tm	tt ;
	
	InfoLog( __FILE__ , __LINE__ , "business_date[%s]" , business_date );
	strptime( business_date , "%Y-%m-%d" , & tt );
	InfoLog( __FILE__ , __LINE__ , "tt.tm_year[%d] .tm_mon+1[%d] .tm_mday[%d] .tm_wday[%d]" , tt.tm_year , tt.tm_mon+1 , tt.tm_mday , tt.tm_wday );
	
	InfoLog( __FILE__ , __LINE__ , "CheckFilterRule p_batches_filter->batch_name[%s]" , p_batches_filter->batch_name );
	if( STRCMP( p_batches_filter->filter_type , == , "DD" ) )
	{
		char	filter_param[ sizeof(p_batches_filter->filter_param) ] ;
		char	*ptr = NULL ;
		
		strcpy( filter_param , p_batches_filter->filter_param );
		ptr = strtok( filter_param , "," ) ;
		while( ptr )
		{
			InfoLog( __FILE__ , __LINE__ , "DD : ptr[%s] tt.tm_year+1900[%d] tt.tm_mon+1[%d] tt.tm_mday[%d] DaysOfMonth [IS_LEAPYEAR(tt.tm_year+1900)] [tt.tm_mon+1] [%d]" , ptr , tt.tm_year+1900 , tt.tm_mon+1 , tt.tm_mday , DaysOfMonth [IS_LEAPYEAR(tt.tm_year+1900)] [tt.tm_mon+1] );
			if( STRCMP( ptr , == , "MB" ) && tt.tm_mday == 1 )
				break;
			else if( STRCMP( ptr , == , "ME" ) && tt.tm_mday == DaysOfMonth [IS_LEAPYEAR(tt.tm_year+1900)] [tt.tm_mon+1] )
				break;
			else if( atoi(ptr) == tt.tm_mday )
				break;
			
			ptr = strtok( NULL , "," ) ;
		}
		if( ptr )
			return 0;
	}
	else if( STRCMP( p_batches_filter->filter_type , == , "MM-DD" ) )
	{
		char	filter_param[ sizeof(p_batches_filter->filter_param) ] ;
		char	*ptr = NULL ;
		char	_date[ 4+1+2+1+2 + 1 ] ;
		
		memset( _date , 0x00 , sizeof(_date) );
		snprintf( _date , sizeof(_date)-1 , "%02d-%02d" , tt.tm_mon+1 , tt.tm_mday );
		
		strcpy( filter_param , p_batches_filter->filter_param );
		ptr = strtok( filter_param , "," ) ;
		while( ptr )
		{
			InfoLog( __FILE__ , __LINE__ , "MM-DD : ptr[%s] _date[%s]" , ptr , _date );
			if( STRCMP( ptr , == , _date ) )
				break;
			
			ptr = strtok( NULL , "," ) ;
		}
		if( ptr )
			return 0;
	}
	else if( STRCMP( p_batches_filter->filter_type , == , "WDAY" ) )
	{
		char	filter_param[ sizeof(p_batches_filter->filter_param) ] ;
		char	*ptr = NULL ;
		
		strcpy( filter_param , p_batches_filter->filter_param );
		ptr = strtok( filter_param , "," ) ;
		while( ptr )
		{
			InfoLog( __FILE__ , __LINE__ , "WDAY : (atoi(ptr)%%7)[%d] tt.tm_wday[%d]" , (atoi(ptr)%7) , tt.tm_wday );
			if( (atoi(ptr)%7) == tt.tm_wday )
				break;
			
			ptr = strtok( NULL , "," ) ;
		}
		if( ptr )
			return 0;
	}
	else
	{
		ErrorLog( __FILE__ , __LINE__ , "filter_type[%s] invalid" , p_batches_filter->filter_type );
		return DC4C_ERROR_PARAMETER;
	}
	
	return 1;
}

static funcDC4COnBeginTaskProc OnBeginTaskProc ;
void OnBeginTaskProc( struct Dc4cApiEnv *penv , int task_index , void *p1 , void *p2 )
{
	int			tasks_count ;
	Pm_Hzbat_Batches_Info	batch_info ;
	Pm_Hzbat_Batches_Tasks	batches_tasks ;
	char			begin_datetime[ 19 + 1 ] ;
	
	int			nret = 0 ;
	
	dbBeginWork();
	
	memset( & batches_tasks , 0x00 , sizeof(batches_tasks) );
	ConvertTimeString( DC4CGetBatchTasksBeginTimestamp(penv,task_index) , begin_datetime , sizeof(begin_datetime) );
	strncpy( batches_tasks.begin_datetime , begin_datetime , sizeof(batches_tasks.begin_datetime)-1 );
	batches_tasks.progress = DC4C_DAGTASK_PROGRESS_EXECUTING ;
	strncpy( batches_tasks.ip , DC4CGetBatchTasksIp(penv,task_index) , sizeof(batches_tasks.ip)-1 );
	batches_tasks.port = DC4CGetBatchTasksPort(penv,task_index) ;
	nret = DB_pm_hzbat_batches_tasks_update_for_begin_datetime_and_end_datetime_and_ip_and_port_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index( (char*)p1 , (char*)p2 , DC4CGetBatchTasksOrderIndex(penv,task_index) , & batches_tasks ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_batches_tasks_update_for_begin_datetime_and_end_datetime_and_ip_and_port_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index failed[%d] , schedule_name[%s] batch_name[%s] order_index[%d]\n" , nret , (char*)p1 , (char*)p2 , DC4CGetBatchTasksOrderIndex(penv,task_index) );
		DB_pm_hzbat_batches_tasks_debug_print( "ERROR" , & batches_tasks , __FILE__ , __LINE__ );
		dbRollback();
		return;
	}
	
	dbCommit();
	
	return;
}

static funcDC4COnCancelTaskProc OnCancelTaskProc ;
void OnCancelTaskProc( struct Dc4cApiEnv *penv , int task_index , void *p1 , void *p2 )
{
	int			tasks_count ;
	Pm_Hzbat_Batches_Info	batch_info ;
	Pm_Hzbat_Batches_Tasks	batches_tasks ;
	char			begin_datetime[ 19 + 1 ] ;
	
	int			nret = 0 ;
	
	dbBeginWork();
	
	memset( & batches_tasks , 0x00 , sizeof(batches_tasks) );
	batches_tasks.progress = DC4C_DAGTASK_PROGRESS_INIT ;
	nret = DB_pm_hzbat_batches_tasks_update_for_begin_datetime_and_end_datetime_and_ip_and_port_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index( (char*)p1 , (char*)p2 , DC4CGetBatchTasksOrderIndex(penv,task_index) , & batches_tasks ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_batches_tasks_update_for_begin_datetime_and_end_datetime_and_ip_and_port_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index failed[%d] , schedule_name[%s] batch_name[%s] order_index[%d]\n" , nret , (char*)p1 , (char*)p2 , DC4CGetBatchTasksOrderIndex(penv,task_index) );
		DB_pm_hzbat_batches_tasks_debug_print( "ERROR" , & batches_tasks , __FILE__ , __LINE__ );
		dbRollback();
		return;
	}
	
	dbCommit();
	
	return;
}

static funcDC4COnFinishTaskProc OnFinishTaskProc ;
void OnFinishTaskProc( struct Dc4cApiEnv *penv , int task_index , void *p1 , void *p2 )
{
	int			tasks_count ;
	Pm_Hzbat_Batches_Info	batch_info ;
	Pm_Hzbat_Batches_Tasks	batches_tasks ;
	char			begin_datetime[ 19 + 1 ] ;
	char			end_datetime[ 19 + 1 ] ;
	
	int			nret = 0 ;
	
	printf( "BATCHTASK-[%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
		, task_index , DC4CGetBatchTasksIp(penv,task_index) , DC4CGetBatchTasksPort(penv,task_index)
		, DC4CGetBatchTasksTid(penv,task_index) , DC4CGetBatchTasksProgramAndParams(penv,task_index) , DC4CGetBatchTasksTimeout(penv,task_index) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,task_index),begin_datetime,sizeof(begin_datetime))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,task_index),end_datetime,sizeof(end_datetime))+11 , DC4CGetBatchTasksElapse(penv,task_index)
		, DC4CGetBatchTasksProgress(penv,task_index) , DC4CGetBatchTasksError(penv,task_index) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,task_index)) , DC4CGetBatchTasksInfo(penv,task_index) );
	
	dbBeginWork();
	
	memset( & batches_tasks , 0x00 , sizeof(batches_tasks) );
	ConvertTimeString( DC4CGetBatchTasksBeginTimestamp(penv,task_index) , begin_datetime , sizeof(begin_datetime) );
	strncpy( batches_tasks.begin_datetime , begin_datetime , sizeof(batches_tasks.begin_datetime)-1 );
	ConvertTimeString( DC4CGetBatchTasksEndTimestamp(penv,task_index) , end_datetime , sizeof(end_datetime) );
	strncpy( batches_tasks.end_datetime , end_datetime , sizeof(batches_tasks.end_datetime)-1 );
	if( DC4CGetBatchTasksError(penv,task_index) == 0 && DC4CGetBatchTasksStatus(penv,task_index) == 0 )
		batches_tasks.progress = DC4C_DAGTASK_PROGRESS_FINISHED ;
	else
		batches_tasks.progress = DC4C_DAGTASK_PROGRESS_FINISHED_WITH_ERROR ;
	batches_tasks.error = DC4CGetBatchTasksError(penv,task_index) ;
	batches_tasks.status = DC4CGetBatchTasksStatus(penv,task_index) ;
	nret = DB_pm_hzbat_batches_tasks_update_for_end_datetime_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index( (char*)p1 , (char*)p2 , DC4CGetBatchTasksOrderIndex(penv,task_index) , & batches_tasks ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_batches_tasks_update_for_end_datetime_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index failed[%d] , schedule_name[%s] batch_name[%s] order_index[%d]\n" , nret , (char*)p1 , (char*)p2 , DC4CGetBatchTasksOrderIndex(penv,task_index) );
		DB_pm_hzbat_batches_tasks_debug_print( "ERROR" , & batches_tasks , __FILE__ , __LINE__ );
		dbRollback();
		return;
	}
	
	dbCommit();
	
	return;
}

static int _hzbat_schedule_batch( char *business_date , char *schedule_name , char *batch_name )
{
	Pm_Hzbat_Batches_Info	batch_info ;
	
	struct Dc4cApiEnv	*penv = NULL ;
	
	struct Dc4cBatchTask	*a_tasks = NULL ;
	Pm_Hzbat_Batches_Tasks	*a_batches_tasks = NULL ;
	long			tasks_count ;
	long			task_no ;
	int			task_index ;
	
	Select_Info		batch_task_cursor ;
	Pm_Hzbat_Batches_Tasks	batch_task ;
	Pm_Hzbat_Batches_Filter	batch_filter ;
	
	char			filename[ 256 + 1 ] ;
	char			pathfilename[ 256 + 1 ] ;
	
	char			begin_datetime[ 19 + 1 ] ;
	char			end_datetime[ 19 + 1 ] ;
	
	int			nret = 0 ;
	
	memset( & batch_info , 0x00 , sizeof(batch_info) );
	nret = DB_pm_hzbat_batches_info_read_by_schedule_name_and_batch_name( schedule_name , batch_name , & batch_info ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_info_read_by_schedule_name_and_batch_name failed[%d] , schedule_name[%s] batch_name[%s]" , nret , schedule_name , batch_name );
		return DC4C_ERROR_DATABASE;
	}
	
	nret = DC4CInitEnv( & penv , NULL ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CInitEnv failed[%d]" , nret );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CInitEnv ok" );
	}
	
	nret = DC4CDoTask( penv , batch_info.pretask_program_and_params , batch_info.pretask_timeout , NULL ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CDoTask failed[%d]" , nret );
	}
	
	printf( "BATCHTASK-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
		, DC4CGetTaskIp(penv) , DC4CGetTaskPort(penv)
		, DC4CGetTaskTid(penv) , DC4CGetTaskProgramAndParams(penv) , DC4CGetTaskTimeout(penv) , ConvertTimeString(DC4CGetTaskBeginTimestamp(penv),begin_datetime,sizeof(begin_datetime))+11 , ConvertTimeString(DC4CGetTaskEndTimestamp(penv),end_datetime,sizeof(end_datetime))+11 , DC4CGetTaskElapse(penv)
		, DC4CGetTaskProgress(penv) , DC4CGetTaskError(penv) , WEXITSTATUS(DC4CGetTaskStatus(penv)) , DC4CGetTaskInfo(penv) );
	
	dbBeginWork();
	
	memset( & batch_info , 0x00 , sizeof(batch_info) );
	batch_info.pretask_progress = DC4CGetTaskProgress( penv );
	strncpy( batch_info.pretask_ip , DC4CGetTaskIp(penv) , sizeof(batch_info.pretask_ip)-1 );
	batch_info.pretask_port = DC4CGetTaskPort( penv );
	batch_info.pretask_error = DC4CGetTaskError( penv );
	batch_info.pretask_status = DC4CGetTaskStatus( penv );
	nret = DB_pm_hzbat_batches_info_update_for_pretask_ip_and_pretask_port_and_pretask_progress_and_pretask_error_and_pretask_status_by_schedule_name_and_batch_name( schedule_name , batch_name , & batch_info ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_batches_info_update_for_pretask_ip_and_pretask_port_and_pretask_progress_and_pretask_error_and_pretask_status_by_schedule_name_and_batch_name failed[%d] , schedule_name[%s] batch_name[%s]\n" , nret , schedule_name , batch_name );
		dbRollback();
		return DC4C_ERROR_DATABASE;
	}
	
	dbCommit();
	
	DC4CCleanEnv( & penv );
	
	if( nret )
	{
		return nret;
	}
	
	nret = DB_pm_hzbat_batches_tasks_count_by_schedule_name_and_batch_name( schedule_name , batch_name , & tasks_count ) ;
	if( nret == SQLNOTFOUND )
	{
		InfoLog( __FILE__ , __LINE__ , "tasks not found" );
		return 0;
	}
	else if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_tasks_count_by_schedule_name_and_batch_name failed[%d] , schedule_name[%s] batch_name[%s]" , nret , schedule_name , batch_name );
		return DC4C_ERROR_DATABASE;
	}
	
	a_tasks = (struct Dc4cBatchTask *)malloc( sizeof(struct Dc4cBatchTask) * tasks_count ) ;
	if( a_tasks == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed[%d] , errno[%d]" , nret , errno );
		return DC4C_ERROR_ALLOC;
	}
	memset( a_tasks , 0x00 , sizeof(struct Dc4cBatchTask) * tasks_count );
	
	a_batches_tasks = (Pm_Hzbat_Batches_Tasks *)malloc( sizeof(Pm_Hzbat_Batches_Tasks) * tasks_count ) ;
	if( a_batches_tasks == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed[%d] , errno[%d]" , nret , errno );
		return DC4C_ERROR_ALLOC;
	}
	memset( a_batches_tasks , 0x00 , sizeof(Pm_Hzbat_Batches_Tasks) * tasks_count );
	
	memset( & batch_task_cursor , 0x00 , sizeof(batch_task_cursor) );
	nret = DB_pm_hzbat_batches_tasks_open_select_by_schedule_name_and_batch_name_order_by_order_index( schedule_name , batch_name , & batch_task_cursor ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_tasks_open_select_by_schedule_name_and_batch_name_order_by_order_index failed[%d] , schedule_name[%s] batch_name[%s]" , nret , schedule_name , batch_name );
		free( a_tasks );
		return DC4C_ERROR_DATABASE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_tasks_open_select_by_schedule_name_and_batch_name_order_by_order_index ok , schedule_name[%s] batch_name[%s]" , schedule_name , batch_name );
	}
	
	task_no = 0 ;
	while(1)
	{
		memset( & batch_task , 0x00 , sizeof(batch_task) );
		nret = DB_pm_hzbat_batches_tasks_fetch_select( & batch_task_cursor , & batch_task ) ;
		if( nret == SQLNOTFOUND )
		{
			break;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_tasks_fetch_select failed[%d]" , nret );
			free( a_tasks );
			free( a_batches_tasks );
			DB_pm_hzbat_batches_tasks_close_select( & batch_task_cursor );
			return DC4C_ERROR_DATABASE;
		}
		
		if( batch_task.progress == DC4C_DAGTASK_PROGRESS_FINISHED )
			continue;
		if( STRCMP( batch_task.program_and_params , == , "" ) )
			continue;
		
		strcpy( a_tasks[task_no].program_and_params , batch_task.program_and_params );
		a_tasks[task_no].order_index = batch_task.order_index ;
		
		memcpy( & (a_batches_tasks[task_no]) , & batch_task , sizeof(Pm_Hzbat_Batches_Tasks) );
		
		memset( & batch_filter , 0x00 , sizeof(batch_filter) );
		nret = DB_pm_hzbat_batches_filter_read_by_schedule_name_and_batch_name( schedule_name , batch_task.batch_name , & batch_filter ) ;
		if( nret == SQLNOTFOUND )
		{
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_filter_read_by_schedule_name_and_batch_name failed[%d] , schedule_name[%s] batch_name[%s]" , nret , schedule_name , batch_task.batch_name );
			free( a_tasks );
			free( a_batches_tasks );
			DB_pm_hzbat_batches_tasks_close_select( & batch_task_cursor );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			nret = CheckFilterRule( business_date , & batch_filter ) ;
			if( nret == 1 )
			{
				InfoLog( __FILE__ , __LINE__ , "FILTER!" );
				continue;
			}
			else if( nret )
			{
				free( a_tasks );
				free( a_batches_tasks );
				DB_pm_hzbat_batches_tasks_close_select( & batch_task_cursor );
				return nret;
			}
		}
		
		memset( filename , 0x00 , sizeof(filename) );
		sscanf( batch_task.program_and_params , "%s" , filename );
		snprintf( pathfilename , sizeof(pathfilename) , "%s/bin/%s" , getenv("HOME") , filename );
		if( access( pathfilename , F_OK|R_OK ) == -1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "access[%s] failed , errno[%d]" , pathfilename , errno );
			free( a_tasks );
			free( a_batches_tasks );
			DB_pm_hzbat_batches_tasks_close_select( & batch_task_cursor );
			return DC4C_ERROR_FILE_NOT_FOUND;
		}
		
		task_no++;
		if( task_no > tasks_count )
		{
			ErrorLog( __FILE__ , __LINE__ , "task_no[%ld]>tasks_count[%ld]" , task_no , tasks_count );
			free( a_tasks );
			free( a_batches_tasks );
			DB_pm_hzbat_batches_tasks_close_select( & batch_task_cursor );
			return DC4C_ERROR_DATABASE;
		}
	}
	
	DB_pm_hzbat_batches_tasks_close_select( & batch_task_cursor );
	DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_tasks_close_select ok" );
	
	tasks_count = task_no ;
	
	nret = DC4CInitEnv( & penv , NULL ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CInitEnv failed[%d]" , nret );
		free( a_tasks );
		free( a_batches_tasks );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CInitEnv ok" );
	}
	
	DC4CSetOptions( penv , DC4C_OPTIONS_INTERRUPT_BY_APP );
	
	DC4CSetProcDataPtr( penv , schedule_name , batch_name );
	DC4CSetOnBeginTaskProc( penv  , & OnBeginTaskProc );
	DC4CSetOnCancelTaskProc( penv  , & OnCancelTaskProc );
	DC4CSetOnFinishTaskProc( penv  , & OnFinishTaskProc );
	
	nret = DC4CBeginBatchTasks( penv , (int)tasks_count , a_tasks , (int)tasks_count ) ;
	if( nret )
	{
		printf( "DC4CBeginBatchTasks failed[%d]\n" , nret );
		free( a_tasks );
		free( a_batches_tasks );
		DC4CCleanEnv( & penv );
		return nret;
	}
	
	while(1)
	{
		nret = DC4CPerformBatchTasks( penv , & task_index ) ;
		if( nret == DC4C_INFO_TASK_FINISHED )
		{
		}
		else if( nret == DC4C_INFO_BATCH_TASKS_FINISHED )
		{
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
	
	free( a_tasks );
	free( a_batches_tasks );
	
	nret = DC4CBatchTasksInterruptCode( penv );
	
	DC4CCleanEnv( & penv );
	
	return nret;
}

static int hzbat_schedule_batch( char *business_date , char *schedule_name , char *batch_name )
{
	Pm_Hzbat_Batches_Info	batch_info ;
	
	int			nret = 0 ;
	int			nret2 = 0 ;
	
	dbBeginWork();
	
	memset( & batch_info , 0x00 , sizeof(batch_info) );
	GetTimeStringNow( batch_info.begin_datetime , sizeof(batch_info.begin_datetime) );
	batch_info.progress = DC4C_DAGBATCH_PROGRESS_EXECUTING ;
	nret = DB_pm_hzbat_batches_info_update_for_begin_datetime_and_end_datetime_and_progress_by_schedule_name_and_batch_name( schedule_name , batch_name , & batch_info ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_batches_info_update_for_begin_datetime_and_end_datetime_and_progress_by_schedule_name_and_batch_name failed[%d] , schedule_name[%s] batch_name[%s]\n" , nret , schedule_name , batch_name );
		DB_pm_hzbat_batches_info_debug_print( "ERROR" , & batch_info , __FILE__ , __LINE__ );
		dbRollback();
		return;
	}
	
	nret2 = _hzbat_schedule_batch( business_date , schedule_name , batch_name ) ;
	if( nret2 )
	{
		printf( "_hzbat_schedule_batch failed[%d]\n" , nret2 );
	}
	
	dbBeginWork();
	
	memset( & batch_info , 0x00 , sizeof(batch_info) );
	GetTimeStringNow( batch_info.end_datetime , sizeof(batch_info.end_datetime) );
	if( nret2 )
		batch_info.progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
	else
		batch_info.progress = DC4C_DAGBATCH_PROGRESS_FINISHED ;
	nret = DB_pm_hzbat_batches_info_update_for_end_datetime_and_progress_by_schedule_name_and_batch_name( schedule_name , batch_name , & batch_info ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_batches_info_update_for_end_datetime_and_progress_by_schedule_name_and_batch_name failed[%d] , schedule_name[%s] batch_name[%s]\n" , nret , schedule_name , batch_name );
		DB_pm_hzbat_batches_info_debug_print( "ERROR" , & batch_info , __FILE__ , __LINE__ );
		dbRollback();
		return;
	}
	
	dbCommit();
	
	return nret2;
}

static int HZBATDoPrepareTasks( char *business_date , char *schedule_name )
{
	struct Dc4cBatchTask	*a_pretasks = NULL ;
	Pm_Hzbat_Batches_Info	*a_batches_info = NULL ;
	long			pretasks_count ;
	long			pretasks_no ;
	int			pretask_index ;
	
	Select_Info		batch_info_cursor ;
	Pm_Hzbat_Batches_Info	batch_info ;
	Pm_Hzbat_Batches_Filter	batch_filter ;
	
	char			filename[ 256 + 1 ] ;
	char			pathfilename[ 256 + 1 ] ;
	
	struct Dc4cApiEnv	*penv = NULL ;
	char			begin_datetime[ 19 + 1 ] ;
	char			end_datetime[ 19 + 1 ] ;
	
	int			nret = 0 ;
	int			nret2 = 0 ;
	
	nret = DB_pm_hzbat_batches_info_count_by_schedule_name( schedule_name , & pretasks_count ) ;
	if( nret == SQLNOTFOUND )
	{
		InfoLog( __FILE__ , __LINE__ , "batch not found" );
		return 0;
	}
	else if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_info_count_by_schedule_name failed[%d] , schedule_name[%s]" , nret , schedule_name );
		return DC4C_ERROR_DATABASE;
	}
	
	a_pretasks = (struct Dc4cBatchTask *)malloc( sizeof(struct Dc4cBatchTask) * pretasks_count ) ;
	if( a_pretasks == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed[%d] , errno[%d]" , nret , errno );
		return DC4C_ERROR_ALLOC;
	}
	memset( a_pretasks , 0x00 , sizeof(struct Dc4cBatchTask) * pretasks_count );
	
	a_batches_info = (Pm_Hzbat_Batches_Info *)malloc( sizeof(Pm_Hzbat_Batches_Info) * pretasks_count ) ;
	if( a_batches_info == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed[%d] , errno[%d]" , nret , errno );
		return DC4C_ERROR_ALLOC;
	}
	memset( a_batches_info , 0x00 , sizeof(Pm_Hzbat_Batches_Info) * pretasks_count );
	
	memset( & batch_info_cursor , 0x00 , sizeof(batch_info_cursor) );
	nret = DB_pm_hzbat_batches_info_open_select_by_schedule_name( schedule_name , & batch_info_cursor ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_info_open_select_by_schedule_name failed[%d] , schedule_name[%s]" , nret , schedule_name );
		free( a_pretasks );
		return DC4C_ERROR_DATABASE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_info_open_select_by_schedule_name ok , schedule_name[%s]" , schedule_name );
	}
	
	pretasks_no = 0 ;
	while(1)
	{
		memset( & batch_info , 0x00 , sizeof(batch_info) );
		nret = DB_pm_hzbat_batches_info_fetch_select( & batch_info_cursor , & batch_info ) ;
		if( nret == SQLNOTFOUND )
		{
			break;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_info_fetch_select failed[%d]" , nret );
			free( a_pretasks );
			free( a_batches_info );
			DB_pm_hzbat_batches_info_close_select( & batch_info_cursor );
			return DC4C_ERROR_DATABASE;
		}
		
		if( batch_info.pretask_progress == DC4C_DAGTASK_PROGRESS_FINISHED )
			continue;
		if( STRCMP( batch_info.pretask_program_and_params , == , "" ) )
			continue;
		
		strcpy( a_pretasks[pretasks_no].program_and_params , batch_info.pretask_program_and_params );
		a_pretasks[pretasks_no].order_index = pretasks_no ;
		
		memcpy( & (a_batches_info[pretasks_no]) , & batch_info , sizeof(Pm_Hzbat_Batches_Info) );
		
		memset( & batch_filter , 0x00 , sizeof(batch_filter) );
		nret = DB_pm_hzbat_batches_filter_read_by_schedule_name_and_batch_name( schedule_name , batch_info.batch_name , & batch_filter ) ;
		if( nret == SQLNOTFOUND )
		{
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_filter_read_by_schedule_name_and_batch_name failed[%d] , schedule_name[%s] batch_name[%s]" , nret , schedule_name , batch_info.batch_name );
			free( a_pretasks );
			free( a_batches_info );
			DB_pm_hzbat_batches_info_close_select( & batch_info_cursor );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			nret = CheckFilterRule( business_date , & batch_filter ) ;
			if( nret == 1 )
			{
				InfoLog( __FILE__ , __LINE__ , "FILTER!" );
				continue;
			}
			else if( nret )
			{
				free( a_pretasks );
				free( a_batches_info );
				DB_pm_hzbat_batches_info_close_select( & batch_info_cursor );
				return nret;
			}
		}
		
		memset( filename , 0x00 , sizeof(filename) );
		sscanf( batch_info.pretask_program_and_params , "%s" , filename );
		snprintf( pathfilename , sizeof(pathfilename) , "%s/bin/%s" , getenv("HOME") , filename );
		if( access( pathfilename , F_OK|R_OK ) == -1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "access[%s] failed , errno[%d]" , pathfilename , errno );
			free( a_pretasks );
			free( a_batches_info );
			DB_pm_hzbat_batches_info_close_select( & batch_info_cursor );
			return DC4C_ERROR_FILE_NOT_FOUND;
		}
		
		pretasks_no++;
		if( pretasks_no > pretasks_count )
		{
			ErrorLog( __FILE__ , __LINE__ , "pretasks_no[%ld]>pretasks_count[%ld]" , pretasks_no , pretasks_count );
			free( a_pretasks );
			free( a_batches_info );
			DB_pm_hzbat_batches_info_close_select( & batch_info_cursor );
			return DC4C_ERROR_DATABASE;
		}
	}
	
	DB_pm_hzbat_batches_info_close_select( & batch_info_cursor );
	DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_info_close_select ok" );
	
	pretasks_count = pretasks_no ;
	
	nret = DC4CInitEnv( & penv , NULL ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CInitEnv failed[%d]" , nret );
		free( a_pretasks );
		free( a_batches_info );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CInitEnv ok" );
	}
	
	DC4CSetOptions( penv , DC4C_OPTIONS_INTERRUPT_BY_APP );
	
	nret = DC4CBeginBatchTasks( penv , (int)pretasks_count , a_pretasks , (int)pretasks_count ) ;
	if( nret )
	{
		printf( "DC4CBeginBatchTasks failed[%d]\n" , nret );
		free( a_pretasks );
		free( a_batches_info );
		DC4CCleanEnv( & penv );
		return nret;
	}
	
	while(1)
	{
		nret = DC4CPerformBatchTasks( penv , & pretask_index ) ;
		if( nret == DC4C_INFO_TASK_FINISHED )
		{
		}
		else if( nret == DC4C_INFO_BATCH_TASKS_FINISHED )
		{
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
		}
		
		if( pretask_index >= 0 )
		{
			printf( "BATCHTASK-[%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
				, pretask_index , DC4CGetBatchTasksIp(penv,pretask_index) , DC4CGetBatchTasksPort(penv,pretask_index)
				, DC4CGetBatchTasksTid(penv,pretask_index) , DC4CGetBatchTasksProgramAndParams(penv,pretask_index) , DC4CGetBatchTasksTimeout(penv,pretask_index) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,pretask_index),begin_datetime,sizeof(begin_datetime))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,pretask_index),end_datetime,sizeof(end_datetime))+11 , DC4CGetBatchTasksElapse(penv,pretask_index)
				, DC4CGetBatchTasksProgress(penv,pretask_index) , DC4CGetBatchTasksError(penv,pretask_index) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,pretask_index)) , DC4CGetBatchTasksInfo(penv,pretask_index) );
			
			dbBeginWork();
			
			memset( & batch_info , 0x00 , sizeof(batch_info) );
			batch_info.pretask_progress = DC4CGetBatchTasksProgress(penv,pretask_index) ;
			strncpy( batch_info.pretask_ip , DC4CGetBatchTasksIp(penv,pretask_index) , sizeof(batch_info.pretask_ip)-1 );
			batch_info.pretask_port = DC4CGetBatchTasksPort(penv,pretask_index) ;
			batch_info.pretask_error = DC4CGetBatchTasksError(penv,pretask_index) ;
			batch_info.pretask_status = DC4CGetBatchTasksStatus(penv,pretask_index) ;
			nret2 = DB_pm_hzbat_batches_info_update_for_pretask_ip_and_pretask_port_and_pretask_progress_and_pretask_error_and_pretask_status_by_schedule_name_and_batch_name( a_batches_info[pretask_index].schedule_name , a_batches_info[pretask_index].batch_name , & batch_info ) ;
			if( nret2 )
			{
				printf( "DB_pm_hzbat_batches_info_update_for_pretask_ip_and_pretask_port_and_pretask_progress_and_pretask_error_and_pretask_status_by_schedule_name_and_batch_name failed[%d] , schedule_name[%s] batch_name[%s]\n" , nret2 , a_batches_info[pretask_index].schedule_name , a_batches_info[pretask_index].batch_name );
				dbRollback();
				break;
			}
			
			dbCommit();
		}
		
		if( nret != DC4C_INFO_TASK_FINISHED )
			break;
	}
	
	free( a_pretasks );
	free( a_batches_info );
	
	nret = DC4CBatchTasksInterruptCode( penv );
	
	DC4CCleanEnv( & penv );
	
	return nret;
}

static int HZBATLoadDagScheduleFromDatabase( struct Dc4cDagSchedule **pp_sched , char *business_date , char *schedule_name , char *rservers_ip_port , int options )
{
	dag_schedule_configfile 	*p_config = NULL ;
	
	Pm_Hzbat_Schedule		schedule ;
	Pm_Hzbat_Batches_Info		batch_info ;
	Pm_Hzbat_Batches_Direction	batches_direction ;
	Pm_Hzbat_Batches_Filter		batch_filter ;
	Pm_Hzbat_Batches_Tasks		batches_tasks ;
	Select_Info			batch_info_cursor ;
	Select_Info			batches_direction_cursor ;
	Select_Info			batches_tasks_cursor ;
	char				begin_datetime[ 19 + 1 ] ;
	
	char				filename[ 256 + 1 ] ;
	char				pathfilename[ 256 + 1 ] ;
	
	int				nret = 0 ;
	
	p_config = (dag_schedule_configfile *)malloc( sizeof(dag_schedule_configfile) ) ;
	if( p_config == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return DC4C_ERROR_ALLOC;
	}
	
	DSCINIT_dag_schedule_configfile( p_config );
	
	memset( & schedule , 0x00 , sizeof(schedule) );
	nret = DB_pm_hzbat_schedule_read_by_schedule_name( schedule_name , & schedule ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_schedule_read_by_schedule_name failed[%d] , schedule_name[%s]" , nret , schedule_name );
		free( p_config );
		return DC4C_ERROR_DATABASE;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "DB_pm_hzbat_schedule_read_by_schedule_name ok , schedule_name[%s]" , schedule_name );
	}
	
	if( schedule.progress == DC4C_DAGSCHEDULE_PROGRESS_FINISHED )
	{
		InfoLog( __FILE__ , __LINE__ , "schedule status[%d] , all is done" , schedule.progress );
		free( p_config );
		return 0;
	}
	
	dbBeginWork();
	
	strncpy( schedule.begin_datetime , GetTimeStringNow(begin_datetime,sizeof(begin_datetime)) , sizeof(schedule.begin_datetime)-1 );
	schedule.progress = DC4C_DAGSCHEDULE_PROGRESS_EXECUTING ;
	nret = DB_pm_hzbat_schedule_update_for_begin_datetime_and_progress_by_schedule_name( schedule_name , & schedule ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_UPDATE_dag_schedule_SET_begin_datetime_progress_WHERE_schedule_name_E failed[%d] , schedule_name[%s]" , nret , schedule_name );
		free( p_config );
		dbRollback();
		return DC4C_ERROR_DATABASE;
	}
	
	dbCommit();
	
	strncpy( p_config->schedule.schedule_name , schedule.schedule_name , sizeof(p_config->schedule.schedule_name)-1 );
	strncpy( p_config->schedule.schedule_desc , schedule.schedule_desc , sizeof(p_config->schedule.schedule_desc)-1 );
	strncpy( p_config->schedule.begin_datetime , schedule.begin_datetime , sizeof(p_config->schedule.begin_datetime)-1 );
	strncpy( p_config->schedule.end_datetime , schedule.end_datetime , sizeof(p_config->schedule.end_datetime)-1 );
	p_config->schedule.progress = schedule.progress ;
	
	memset( & batch_info_cursor , 0x00 , sizeof(batch_info_cursor) );
	nret = DB_pm_hzbat_batches_info_open_select_by_schedule_name( schedule_name , & batch_info_cursor ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_info_open_select_by_schedule_name failed[%d] , schedule_name[%s]" , nret , schedule_name );
		free( p_config );
		return DC4C_ERROR_DATABASE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_info_open_select_by_schedule_name ok , schedule_name[%s]" , schedule_name );
	}
	
	for(	p_config->batches._batches_info_count = 0
		; p_config->batches._batches_info_count < p_config->batches._batches_info_size
		; )
	{
		memset( & batch_info , 0x00 , sizeof(batch_info) );
		nret = DB_pm_hzbat_batches_info_fetch_select( & batch_info_cursor , & batch_info ) ;
		if( nret == SQLNOTFOUND )
		{
			break;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_info_fetch_select failed[%d]" , nret );
			free( p_config );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_info_fetch_select ok" );
		}
		
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].batch_name , batch_info.batch_name , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].batch_name)-1 );
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].batch_desc , batch_info.batch_desc , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].batch_desc)-1 );
		p_config->batches.batches_info[p_config->batches._batches_info_count].view_pos_x = batch_info.view_pos_x ;
		p_config->batches.batches_info[p_config->batches._batches_info_count].view_pos_y = batch_info.view_pos_y ;
		
		memset( & batches_tasks_cursor , 0x00 , sizeof(batches_tasks_cursor) );
		nret = DB_pm_hzbat_batches_tasks_open_select_by_schedule_name_and_batch_name_order_by_order_index( schedule_name , batch_info.batch_name , & batches_tasks_cursor ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_tasks_open_select_by_schedule_name_and_batch_name_order_by_order_index failed[%d] , schedule_name[%s] batch_name[%s]" , nret , schedule_name , batch_info.batch_name );
			free( p_config );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_tasks_open_select_by_schedule_name_and_batch_name_order_by_order_index ok , schedule_name[%s] batch_name[%s]" , schedule_name , batch_info.batch_name );
		}
		
		for(	p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count = 0
			; p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count < p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_size
			; )
		{
			memset( & batches_tasks , 0x00 , sizeof(batches_tasks) );
			nret = DB_pm_hzbat_batches_tasks_fetch_select( & batches_tasks_cursor , & batches_tasks ) ;
			if( nret == SQLNOTFOUND )
			{
				break;
			}
			else if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_tasks_fetch_select failed[%d]" , nret );
				free( p_config );
				return DC4C_ERROR_DATABASE;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_tasks_fetch_select ok" );
			}
			
			if( batches_tasks.progress == DC4C_DAGTASK_PROGRESS_FINISHED )
				continue;
			
			p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].order_index = batches_tasks.order_index ;
			strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].program_and_params , batches_tasks.program_and_params , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].program_and_params)-1 );
			p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].timeout = batches_tasks.timeout ;
			p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].progress = batches_tasks.progress ;
			
			memset( filename , 0x00 , sizeof(filename) );
			sscanf( batches_tasks.program_and_params , "%s" , filename );
			snprintf( pathfilename , sizeof(pathfilename) , "%s/bin/%s" , getenv("HOME") , filename );
			if( access( pathfilename , F_OK|R_OK ) == -1 )
			{
				ErrorLog( __FILE__ , __LINE__ , "access[%s] failed , errno[%d]" , pathfilename , errno );
				free( p_config );
				return DC4C_ERROR_FILE_NOT_FOUND;
			}
			
			p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count++;
		}
		p_config->batches.batches_info[p_config->batches._batches_info_count].interrupt_by_app = batch_info.interrupt_by_app ;
		
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].begin_datetime , batch_info.begin_datetime , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].begin_datetime)-1 );
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].end_datetime , batch_info.end_datetime , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].end_datetime)-1 );
		p_config->batches.batches_info[p_config->batches._batches_info_count].progress = batch_info.progress ;
		
		memset( & batch_filter , 0x00 , sizeof(batch_filter) );
		nret = DB_pm_hzbat_batches_filter_read_by_schedule_name_and_batch_name( schedule_name , batch_info.batch_name , & batch_filter ) ;
		if( nret == SQLNOTFOUND )
		{
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_filter_read_by_schedule_name_and_batch_name failed[%d] , schedule_name[%s] batch_name[%s]" , nret , schedule_name , batch_info.batch_name );
			free( p_config );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			nret = CheckFilterRule( business_date , & batch_filter ) ;
			if( nret == 1 )
			{
				InfoLog( __FILE__ , __LINE__ , "FILTER!" );
				p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count = 0 ;
			}
			else if( nret )
			{
				free( p_config );
				return nret;
			}
		}
		
		p_config->batches._batches_info_count++;
		
		DB_pm_hzbat_batches_tasks_close_select( & batches_tasks_cursor );
		DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_tasks_close_select ok" );
	}
	
	DB_pm_hzbat_batches_info_close_select( & batch_info_cursor );
	DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_info_close_select ok" );
	
	memset( & batches_direction_cursor , 0x00 , sizeof(batches_direction_cursor) );
	nret = DB_pm_hzbat_batches_direction_open_select_by_schedule_name( schedule_name , & batches_direction_cursor ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_direction_open_select_by_schedule_name failed[%d] , schedule_name[%s]" , nret , schedule_name );
		free( p_config );
		return DC4C_ERROR_DATABASE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_direction_open_select_by_schedule_name ok , schedule_name[%s]" , schedule_name );
	}
	
	for(	p_config->batches._batches_direction_count = 0
		; p_config->batches._batches_direction_count < p_config->batches._batches_direction_size
		; p_config->batches._batches_direction_count++ )
	{
		memset( & batches_direction , 0x00 , sizeof(batches_direction) );
		nret = DB_pm_hzbat_batches_direction_fetch_select( & batches_direction_cursor , & batches_direction ) ;
		if( nret == SQLNOTFOUND )
		{
			break;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_direction_fetch_select failed[%d]" , nret );
			free( p_config );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_direction_fetch_select ok" );
		}
		
		strncpy( p_config->batches.batches_direction[p_config->batches._batches_direction_count].from_batch , batches_direction.from_batch , sizeof(p_config->batches.batches_direction[p_config->batches._batches_direction_count].from_batch)-1 );
		strncpy( p_config->batches.batches_direction[p_config->batches._batches_direction_count].to_batch , batches_direction.to_batch , sizeof(p_config->batches.batches_direction[p_config->batches._batches_direction_count].to_batch)-1 );
	}
	
	DB_pm_hzbat_batches_direction_close_select( & batches_direction_cursor );
	DebugLog( __FILE__ , __LINE__ , "DB_pm_hzbat_batches_direction_close_select ok" );
	
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

int DB_pm_hzbat_batches_tasks_debug_print(char *reason,Pm_Hzbat_Batches_Tasks *adata, char *filename, int line_no);
int DB_pm_hzbat_schedule_debug_print(char *reason,Pm_Hzbat_Schedule *adata, char *filename, int line_no);

static funcDC4COnBeginDagBatchProc OnBeginDagBatchProc ;
void OnBeginDagBatchProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , void *p1 )
{
	Pm_Hzbat_Batches_Info	batch_info ;
	
	int			nret = 0 ;
	
	dbBeginWork();
	
	memset( & batch_info , 0x00 , sizeof(batch_info) );
	strncpy( batch_info.begin_datetime , DC4CGetDagBatchBeginDatetime(p_batch) , sizeof(batch_info.begin_datetime)-1 );
	batch_info.progress = DC4CGetDagBatchProgress(p_batch) ;
	nret = DB_pm_hzbat_batches_info_update_for_begin_datetime_and_end_datetime_and_progress_by_schedule_name_and_batch_name( DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch) , & batch_info ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_batches_info_update_for_begin_datetime_and_end_datetime_and_progress_by_schedule_name_and_batch_name failed[%d] , schedule_name[%s] batch_name[%s]\n" , nret , DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch) );
		DB_pm_hzbat_batches_info_debug_print( "ERROR" , & batch_info , __FILE__ , __LINE__ );
		dbRollback();
		return;
	}
	
	dbCommit();
	
	return;
}

static funcDC4COnFinishDagBatchProc OnFinishDagBatchProc ;
void OnFinishDagBatchProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , void *p1 )
{
	Pm_Hzbat_Batches_Info	batch_info ;
	
	int			nret = 0 ;
	
	printf( "BATCH-[%s][%s]-[%d]-[%s][%s][%d]\n"
		, DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch)
		, DC4CGetTasksCount(penv)
		, DC4CGetDagBatchBeginDatetime(p_batch) , DC4CGetDagBatchEndDatetime(p_batch) , DC4CGetDagBatchProgress(p_batch) );
	
	dbBeginWork();
	
	memset( & batch_info , 0x00 , sizeof(batch_info) );
	strncpy( batch_info.end_datetime , DC4CGetDagBatchEndDatetime(p_batch) , sizeof(batch_info.end_datetime)-1 );
	batch_info.progress = DC4CGetDagBatchProgress(p_batch) ;
	nret = DB_pm_hzbat_batches_info_update_for_end_datetime_and_progress_by_schedule_name_and_batch_name( DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch) , & batch_info ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_batches_info_update_for_end_datetime_and_progress_by_schedule_name_and_batch_name failed[%d] , schedule_name[%s] batch_name[%s]\n" , nret , DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch) );
		DB_pm_hzbat_batches_info_debug_print( "ERROR" , & batch_info , __FILE__ , __LINE__ );
		dbRollback();
		return;
	}
	
	dbCommit();
	
	return;
}

static funcDC4COnBeginDagBatchTaskProc OnBeginDagBatchTaskProc ;
void OnBeginDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , int task_index , void *p1 )
{
	int			tasks_count ;
	Pm_Hzbat_Batches_Info	batch_info ;
	Pm_Hzbat_Batches_Tasks	batches_tasks ;
	char			begin_datetime[ 19 + 1 ] ;
	
	int			nret = 0 ;
	
	dbBeginWork();
	
	memset( & batches_tasks , 0x00 , sizeof(batches_tasks) );
	ConvertTimeString( DC4CGetBatchTasksBeginTimestamp(penv,task_index) , begin_datetime , sizeof(begin_datetime) );
	strncpy( batches_tasks.begin_datetime , begin_datetime , sizeof(batches_tasks.begin_datetime)-1 );
	batches_tasks.progress = DC4C_DAGTASK_PROGRESS_EXECUTING ;
	strncpy( batches_tasks.ip , DC4CGetBatchTasksIp(penv,task_index) , sizeof(batches_tasks.ip)-1 );
	batches_tasks.port = DC4CGetBatchTasksPort(penv,task_index) ;
	nret = DB_pm_hzbat_batches_tasks_update_for_begin_datetime_and_end_datetime_and_ip_and_port_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index( DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch) , DC4CGetBatchTasksOrderIndex(penv,task_index) , & batches_tasks ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_batches_tasks_update_for_begin_datetime_and_end_datetime_and_ip_and_port_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index failed[%d] , schedule_name[%s] batch_name[%s] order_index[%d]\n" , nret , DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch) , DC4CGetBatchTasksOrderIndex(penv,task_index) );
		DB_pm_hzbat_batches_tasks_debug_print( "ERROR" , & batches_tasks , __FILE__ , __LINE__ );
		dbRollback();
		return;
	}
	
	dbCommit();
	
	return;
}

static funcDC4COnCancelDagBatchTaskProc OnCancelDagBatchTaskProc ;
void OnCancelDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , int task_index , void *p1 )
{
	int			tasks_count ;
	Pm_Hzbat_Batches_Info	batch_info ;
	Pm_Hzbat_Batches_Tasks	batches_tasks ;
	char			begin_datetime[ 19 + 1 ] ;
	
	int			nret = 0 ;
	
	dbBeginWork();
	
	memset( & batches_tasks , 0x00 , sizeof(batches_tasks) );
	batches_tasks.progress = DC4C_DAGTASK_PROGRESS_INIT ;
	nret = DB_pm_hzbat_batches_tasks_update_for_begin_datetime_and_end_datetime_and_ip_and_port_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index( DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch) , DC4CGetBatchTasksOrderIndex(penv,task_index) , & batches_tasks ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_batches_tasks_update_for_begin_datetime_and_end_datetime_and_ip_and_port_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index failed[%d] , schedule_name[%s] batch_name[%s] order_index[%d]\n" , nret , DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch) , DC4CGetBatchTasksOrderIndex(penv,task_index) );
		DB_pm_hzbat_batches_tasks_debug_print( "ERROR" , & batches_tasks , __FILE__ , __LINE__ );
		dbRollback();
		return;
	}
	
	dbCommit();
	
	return;
}

static funcDC4COnFinishDagBatchTaskProc OnFinishDagBatchTaskProc ;
void OnFinishDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , int task_index , void *p1 )
{
	int			tasks_count ;
	Pm_Hzbat_Batches_Info	batch_info ;
	Pm_Hzbat_Batches_Tasks	batches_tasks ;
	char			begin_datetime[ 19 + 1 ] ;
	char			end_datetime[ 19 + 1 ] ;
	
	int			nret = 0 ;
	
	printf( "BATCHTASK-[%d]-[%s][%d]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%d][%s]\n"
		, task_index , DC4CGetBatchTasksIp(penv,task_index) , DC4CGetBatchTasksPort(penv,task_index)
		, DC4CGetBatchTasksTid(penv,task_index) , DC4CGetBatchTasksProgramAndParams(penv,task_index) , DC4CGetBatchTasksTimeout(penv,task_index) , ConvertTimeString(DC4CGetBatchTasksBeginTimestamp(penv,task_index),begin_datetime,sizeof(begin_datetime))+11 , ConvertTimeString(DC4CGetBatchTasksEndTimestamp(penv,task_index),end_datetime,sizeof(end_datetime))+11 , DC4CGetBatchTasksElapse(penv,task_index)
		, DC4CGetBatchTasksProgress(penv,task_index) , DC4CGetBatchTasksError(penv,task_index) , WEXITSTATUS(DC4CGetBatchTasksStatus(penv,task_index)) , DC4CGetBatchTasksInfo(penv,task_index) );
	
	dbBeginWork();
	
	memset( & batches_tasks , 0x00 , sizeof(batches_tasks) );
	ConvertTimeString( DC4CGetBatchTasksBeginTimestamp(penv,task_index) , begin_datetime , sizeof(begin_datetime) );
	strncpy( batches_tasks.begin_datetime , begin_datetime , sizeof(batches_tasks.begin_datetime)-1 );
	ConvertTimeString( DC4CGetBatchTasksEndTimestamp(penv,task_index) , end_datetime , sizeof(end_datetime) );
	strncpy( batches_tasks.end_datetime , end_datetime , sizeof(batches_tasks.end_datetime)-1 );
	if( DC4CGetBatchTasksError(penv,task_index) == 0 && DC4CGetBatchTasksStatus(penv,task_index) == 0 )
		batches_tasks.progress = DC4C_DAGTASK_PROGRESS_FINISHED ;
	else
		batches_tasks.progress = DC4C_DAGTASK_PROGRESS_FINISHED_WITH_ERROR ;
	batches_tasks.error = DC4CGetBatchTasksError(penv,task_index) ;
	batches_tasks.status = DC4CGetBatchTasksStatus(penv,task_index) ;
	nret = DB_pm_hzbat_batches_tasks_update_for_end_datetime_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index( DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch) , DC4CGetBatchTasksOrderIndex(penv,task_index) , & batches_tasks ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_batches_tasks_update_for_end_datetime_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index failed[%d] , schedule_name[%s] batch_name[%s] order_index[%d]\n" , nret , DC4CGetDagScheduleName(p_sched) , DC4CGetDagBatchName(p_batch) , DC4CGetBatchTasksOrderIndex(penv,task_index) );
		DB_pm_hzbat_batches_tasks_debug_print( "ERROR" , & batches_tasks , __FILE__ , __LINE__ );
		dbRollback();
		return;
	}
	
	dbCommit();
	
	return;
}

static int HZBATUnloadDagScheduleToDatabase( struct Dc4cDagSchedule **pp_sched )
{
	Pm_Hzbat_Schedule	schedule ;
	char			end_datetime[ 19 + 1 ] ;
	
	int			nret = 0 ;
	
	dbBeginWork();
	
	memset( & schedule , 0x00 , sizeof(schedule) );
	strncpy( schedule.end_datetime , GetTimeStringNow(end_datetime,sizeof(end_datetime)) , sizeof(schedule.end_datetime)-1 );
	schedule.progress = (DC4CGetDagScheduleProgress(*pp_sched)==DC4C_DAGSCHEDULE_PROGRESS_FINISHED?DC4C_DAGSCHEDULE_PROGRESS_FINISHED:DC4C_DAGSCHEDULE_PROGRESS_FINISHED_WITH_ERROR) ;
	nret = DB_pm_hzbat_schedule_update_for_end_datetime_and_progress_by_schedule_name( DC4CGetDagScheduleName(*pp_sched) , & schedule ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_schedule_update_for_end_datetime_and_progress_by_schedule_name failed[%d] , schedule_name[%s]" , nret , DC4CGetDagScheduleName(*pp_sched) );
		DB_pm_hzbat_schedule_debug_print( "ERROR" , & schedule , __FILE__ , __LINE__ );
		dbRollback();
		DC4CUnloadDagSchedule( pp_sched );
		return DC4C_ERROR_DATABASE;
	}
	
	dbCommit();
	
	DC4CUnloadDagSchedule( pp_sched );
	InfoLog( __FILE__ , __LINE__ , "DC4CUnloadDagSchedule ok" );
	
	return 0;
}

static int hzbat_schedule( char *business_date , char *schedule_name )
{
	Pm_Hzbat_Schedule	schedule ;
	struct Dc4cDagSchedule	*p_sched = NULL ;
	struct Dc4cDagBatch	*p_batch = NULL ;
	struct Dc4cApiEnv	*penv	= NULL ;
	int			task_index ;
	
	int			perform_return = 0 ;
	int			nret = 0 ;
	int			nret2 = 0 ;
	
	printf( "HZBATDoPrepareTasks ...\n" );
	
	nret = HZBATDoPrepareTasks( business_date , schedule_name ) ;
	if( nret )
	{
		printf( "HZBATDoPrepareTasks failed[%d]\n" , nret );
		
		dbBeginWork();
		
		memset( & schedule , 0x00 , sizeof(schedule) );
		schedule.progress = (nret==0?DC4C_DAGSCHEDULE_PROGRESS_FINISHED:DC4C_DAGSCHEDULE_PROGRESS_FINISHED_WITH_ERROR) ;
		nret2 = DB_pm_hzbat_schedule_update_for_end_datetime_and_progress_by_schedule_name( schedule_name , & schedule ) ;
		if( nret2 )
		{
			ErrorLog( __FILE__ , __LINE__ , "DB_pm_hzbat_schedule_update_for_end_datetime_and_progress_by_schedule_name failed[%d] , schedule_name[%s]" , nret2 , schedule_name );
			DB_pm_hzbat_schedule_debug_print( "ERROR" , & schedule , __FILE__ , __LINE__ );
			dbRollback();
			return DC4C_ERROR_DATABASE;
		}
		
		dbCommit();
		
		return nret;
	}
	else
	{
		printf( "HZBATDoPrepareTasks ok\n" );
	}
	
	printf( "HZBATLoadDagScheduleFromDatabase ...\n" );
	
	nret = HZBATLoadDagScheduleFromDatabase( & p_sched , business_date , schedule_name , NULL , 0 ) ;
	if( nret )
	{
		printf( "HZBATLoadDagScheduleFromDatabase failed[%d]\n" , nret );
		return nret;
	}
	else
	{
		printf( "HZBATLoadDagScheduleFromDatabase ok\n" );
	}
	
	if( p_sched == NULL )
	{
		printf( "All is done\n" );
		return 0;
	}
	
	DC4CLogDagSchedule( p_sched );
	
	DC4CSetOnBeginDagBatchProc( p_sched  , & OnBeginDagBatchProc );
	DC4CSetOnFinishDagBatchProc( p_sched  , & OnFinishDagBatchProc );
	DC4CSetOnBeginDagBatchTaskProc( p_sched  , & OnBeginDagBatchTaskProc );
	DC4CSetOnCancelDagBatchTaskProc( p_sched  , & OnCancelDagBatchTaskProc );
	DC4CSetOnFinishDagBatchTaskProc( p_sched  , & OnFinishDagBatchTaskProc );
	
	nret = DC4CBeginDagSchedule( p_sched ) ;
	if( nret )
	{
		printf( "DC4CBeginDagSchedule failed[%d]\n" , nret );
		HZBATUnloadDagScheduleToDatabase( & p_sched );
		return nret;
	}
	
	printf( "DC4CDoDagSchedule ...\n" );
	
	while(1)
	{
		perform_return = DC4CPerformDagSchedule( p_sched , & p_batch , & penv , & task_index ) ;
		if( perform_return == DC4C_INFO_TASK_FINISHED )
		{
		}
		else if( perform_return == DC4C_INFO_BATCH_TASKS_FINISHED )
		{
		}
		else if( perform_return == DC4C_INFO_ALL_ENVS_FINISHED )
		{
			if( DC4CDagScheduleInterruptCode(p_sched) )
			{
				printf( "DC4CPerformDagSchedule return DC4C_DAGSCHEDULE_PROGRESS_FINISHED_WITH_ERROR\n" );
				perform_return = DC4CDagScheduleInterruptCode(p_sched) ;
				break;
			}
			else
			{
				printf( "DC4CPerformDagSchedule return DC4C_INFO_ALL_ENVS_FINISHED\n" );
				perform_return = 0 ;
				break;
			}
		}
		else if( perform_return == DC4C_ERROR_TIMEOUT )
		{
			printf( "DC4CPerformDagSchedule return DC4C_ERROR_TIMEOUT\n" );
		}
		else if( perform_return == DC4C_ERROR_APP )
		{
			printf( "DC4CPerformDagSchedule return DC4C_ERROR_APP\n" );
		}
		else
		{
			printf( "DC4CPerformDagSchedule failed[%d] , batch_name[%s]\n" , perform_return , DC4CGetDagBatchName(p_batch) );
		}
	}
	
	printf( "DC4CDoDagSchedule done\n" );
	
	nret = HZBATUnloadDagScheduleToDatabase( & p_sched ) ;
	if( nret )
	{
		printf( "HZBATUnloadDagScheduleToDatabase failed[%d]\n" , nret );
		return nret;
	}
	else
	{
		printf( "HZBATUnloadDagScheduleToDatabase ok\n" );
	}
	
	return perform_return;
}

static int hzbat_all_schedules( char *business_date )
{
	Select_Info		schedule_cursor ;
	Pm_Hzbat_Schedule	schedule ;
	
	int			nret = 0 ;
	
	memset( & schedule_cursor , 0x00 , sizeof(schedule_cursor) );
	nret = DB_pm_hzbat_schedule_open_select_by_order_index_GE_order_by_order_index( 0 , & schedule_cursor ) ;
	if( nret )
	{
		printf( "DB_pm_hzbat_schedule_open_select_by_order_index_GE_order_by_order_index failed[%d]\n" , nret );
		return -1;
	}
	
	while(1)
	{
		memset( & schedule , 0x00 , sizeof(schedule) );
		nret = DB_pm_hzbat_schedule_fetch_select( & schedule_cursor , & schedule ) ;
		if( nret == SQLNOTFOUND )
		{
			nret = 0 ;
			break;
		}
		else if( nret )
		{
			printf( "DB_pm_hzbat_schedule_fetch_select failed[%d]\n" , nret );
			break;
		}
		
		if( schedule.progress == DC4C_DAGSCHEDULE_PROGRESS_FINISHED )
			continue;
		
		printf( "hzbat[%s] ...\n" , schedule.schedule_name );
		
		nret = hzbat_schedule( business_date , schedule.schedule_name ) ;
		if( nret )
		{
			printf( "hzbat failed[%d]\n" , nret );
			break;
		}
		else
		{
			printf( "hzbat[%s] ok\n" , schedule.schedule_name );
		}
	}
	
	DB_pm_hzbat_schedule_close_select( & schedule_cursor );
	
	return nret;
}

static void version()
{
	printf( "hzbat v1.5.3 build %s %s\n" , __DATE__ , __TIME__ );
	return;
}

static void usage()
{
	printf( "USAGE : hzbat business_date [ (SCHEDULE_NAME) [ (BATCH_NAME) ] ]\n" );
	printf( "                            [ ALL ]\n" );
	return;
}

int main( int argc , char *argv[] )
{
	int		nret = 0 ;
	
        DC4CSetAppLogFile( "hzbat" );
        SetLogLevel( LOGLEVEL_DEBUG );
        
	if( argc == 1 )
	{
		version();
		usage();
		exit(0);
	}
	else if( argc == 1 + 2 || argc == 1 + 3 )
	{
		if( STRCMP( argv[1] , == , "" ) )
		{
			printf( "date[%s] invalid\n" , argv[1] );
			exit(1);
		}
		
		nret = BusinessDataBaseOpen() ;
		if( nret )
		{
			printf( "BusinessDataBaseOpen failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "BusinessDataBaseOpen ok\n" );
		}
		
		if( STRCMP( argv[2] , == , "ALL" ) )
			nret = hzbat_all_schedules( argv[1] ) ;
		else if( argc == 1 + 3 )
			nret = hzbat_schedule_batch( argv[1] , argv[2] , argv[3] ) ;
		else
			nret = hzbat_schedule( argv[1] , argv[2] ) ;
		
		BusinessDataBaseClose();
		printf( "BusinessDataBaseClose ok\n" );
		
		return -nret;
	}
	else
	{
		usage();
		exit(7);
	}
}


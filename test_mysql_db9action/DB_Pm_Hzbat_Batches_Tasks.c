/* 
 This C File "DB_Pm_Hzbat_Batches_Tasks.c" 
 Genenated By
 Application <db8action> for MySQL
 with the action file "DB_Pm_Hzbat_Batches_Tasks.act".
 HZBANK All Right reserved.
 Create: Thu Jul  2 14:35:28 2015
*/

#include <stdio.h>
#include "hzb_log.h"
#include "DB_Pm_Hzbat_Batches_Tasks.h"

static Pm_Hzbat_Batches_Tasks	R_pm_hzbat_batches_tasks;

static int Pm_Hzbat_Batches_Tasks_EraseTailSpace(Pm_Hzbat_Batches_Tasks *_erase_data)
{
	ERASE_TAIL_SPACE(_erase_data->schedule_name);
	ERASE_TAIL_SPACE(_erase_data->batch_name);
	ERASE_TAIL_SPACE(_erase_data->program_and_params);
	ERASE_TAIL_SPACE(_erase_data->begin_datetime);
	ERASE_TAIL_SPACE(_erase_data->end_datetime);
	return(0);
}
int DB_pm_hzbat_batches_tasks_open_select_by_schedule_name( char *schedule_name__0,Select_Info *_a_sInfo)
{
        int     r;

        r=dbCursOpen(_a_sInfo);
        if (r!=0)
                return(r);
        r=dbCursDefineSelect_va(_a_sInfo,
                "SELECT  \n\
			 schedule_name \n\
			,batch_name \n\
			,order_index \n\
			,program_and_params \n\
			,timeout \n\
			,begin_datetime \n\
			,end_datetime \n\
			,progress \n\
			,error \n\
			,status \
		FROM pm_hzbat_batches_tasks \
		WHERE schedule_name = ?",
		R_pm_hzbat_batches_tasks.schedule_name,65,DT_STR,
		R_pm_hzbat_batches_tasks.batch_name,65,DT_STR,
		&R_pm_hzbat_batches_tasks.order_index,4,DT_ITG,
		R_pm_hzbat_batches_tasks.program_and_params,257,DT_STR,
		&R_pm_hzbat_batches_tasks.timeout,4,DT_ITG,
		R_pm_hzbat_batches_tasks.begin_datetime,20,DT_STR,
		R_pm_hzbat_batches_tasks.end_datetime,20,DT_STR,
		&R_pm_hzbat_batches_tasks.progress,4,DT_ITG,
		&R_pm_hzbat_batches_tasks.error,4,DT_ITG,
		&R_pm_hzbat_batches_tasks.status,4,DT_ITG,
		NULL,
		"1",schedule_name__0,75,DT_STR,
		NULL);
        if (r!=0)
        {
                goto E;
        };
        r=dbCursExec(_a_sInfo);

  E:
        if (r!=0) dbCursClose(_a_sInfo);
        return(r);
}

int DB_pm_hzbat_batches_tasks_open_select_by_schedule_name_and_batch_name_order_by_order_index( char *schedule_name__0,char *batch_name__1,Select_Info *_a_sInfo)
{
        int     r;

        r=dbCursOpen(_a_sInfo);
        if (r!=0)
                return(r);
        r=dbCursDefineSelect_va(_a_sInfo,
                "SELECT  \n\
			 schedule_name \n\
			,batch_name \n\
			,order_index \n\
			,program_and_params \n\
			,timeout \n\
			,begin_datetime \n\
			,end_datetime \n\
			,progress \n\
			,error \n\
			,status \
		FROM pm_hzbat_batches_tasks \
		WHERE schedule_name = ? AND \
			batch_name = ? \
		ORDER BY order_index",
		R_pm_hzbat_batches_tasks.schedule_name,65,DT_STR,
		R_pm_hzbat_batches_tasks.batch_name,65,DT_STR,
		&R_pm_hzbat_batches_tasks.order_index,4,DT_ITG,
		R_pm_hzbat_batches_tasks.program_and_params,257,DT_STR,
		&R_pm_hzbat_batches_tasks.timeout,4,DT_ITG,
		R_pm_hzbat_batches_tasks.begin_datetime,20,DT_STR,
		R_pm_hzbat_batches_tasks.end_datetime,20,DT_STR,
		&R_pm_hzbat_batches_tasks.progress,4,DT_ITG,
		&R_pm_hzbat_batches_tasks.error,4,DT_ITG,
		&R_pm_hzbat_batches_tasks.status,4,DT_ITG,
		NULL,
		"1",schedule_name__0,75,DT_STR,
		"2",batch_name__1,75,DT_STR,
		NULL);
        if (r!=0)
        {
                goto E;
        };
        r=dbCursExec(_a_sInfo);

  E:
        if (r!=0) dbCursClose(_a_sInfo);
        return(r);
}

int DB_pm_hzbat_batches_tasks_fetch_select( Select_Info *_a_sInfo,Pm_Hzbat_Batches_Tasks *_a_data)
{
        int     r;

	memset(&R_pm_hzbat_batches_tasks,0,sizeof(Pm_Hzbat_Batches_Tasks));
        r=dbCursFetch(_a_sInfo);
        if (r!=0)
        {
                goto E;
        };


	Pm_Hzbat_Batches_Tasks_EraseTailSpace(&R_pm_hzbat_batches_tasks);
        memcpy(_a_data,&R_pm_hzbat_batches_tasks,sizeof(Pm_Hzbat_Batches_Tasks));

  E:
        return(r);
}

int DB_pm_hzbat_batches_tasks_close_select( Select_Info *_a_sInfo)
{
        int     r;
        r=dbCursClose(_a_sInfo);
        return(r);
}

int DB_pm_hzbat_batches_tasks_read_by_schedule_name_and_batch_name_and_order_index( char *schedule_name__0,char *batch_name__1,int order_index__2,Pm_Hzbat_Batches_Tasks *_a_data)
{
        int     r;
        DBcurs  _a_curs;

	memset(&R_pm_hzbat_batches_tasks,0,sizeof(Pm_Hzbat_Batches_Tasks));
        r=dbCursOpen(&_a_curs);
        if (r!=0)
                return(r);
        r=dbCursDefineSelect_va(&_a_curs,
                "SELECT  \n\
			 schedule_name \n\
			,batch_name \n\
			,order_index \n\
			,program_and_params \n\
			,timeout \n\
			,begin_datetime \n\
			,end_datetime \n\
			,progress \n\
			,error \n\
			,status \
		FROM pm_hzbat_batches_tasks \
		WHERE schedule_name = ? AND \
			batch_name = ? AND \
			order_index = ?",
		R_pm_hzbat_batches_tasks.schedule_name,65,DT_STR,
		R_pm_hzbat_batches_tasks.batch_name,65,DT_STR,
		&R_pm_hzbat_batches_tasks.order_index,4,DT_ITG,
		R_pm_hzbat_batches_tasks.program_and_params,257,DT_STR,
		&R_pm_hzbat_batches_tasks.timeout,4,DT_ITG,
		R_pm_hzbat_batches_tasks.begin_datetime,20,DT_STR,
		R_pm_hzbat_batches_tasks.end_datetime,20,DT_STR,
		&R_pm_hzbat_batches_tasks.progress,4,DT_ITG,
		&R_pm_hzbat_batches_tasks.error,4,DT_ITG,
		&R_pm_hzbat_batches_tasks.status,4,DT_ITG,
		NULL,
		"1",schedule_name__0,75,DT_STR,
		"2",batch_name__1,75,DT_STR,
		"3",&order_index__2,4,DT_ITG,
		NULL);
        if (r!=0)
        {
                goto E;
        };
        r=dbCursExec(&_a_curs);
        if (r!=0)
        {
                goto E;
        };
        r=dbCursFetch(&_a_curs);
        if (r!=0)
        {
                goto E;
        };


	Pm_Hzbat_Batches_Tasks_EraseTailSpace(&R_pm_hzbat_batches_tasks);
        memcpy(_a_data,&R_pm_hzbat_batches_tasks,sizeof(Pm_Hzbat_Batches_Tasks));

  E:
        dbCursClose(&_a_curs);
        return(r);
}

int DB_pm_hzbat_batches_tasks_update_for_begin_datetime_and_end_datetime_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index( char *schedule_name__6,char *batch_name__7,int order_index__8,Pm_Hzbat_Batches_Tasks *_a_data)
{
        int     r;
        memcpy(&R_pm_hzbat_batches_tasks,_a_data,sizeof(Pm_Hzbat_Batches_Tasks));
        r=dbExecSql_va("UPDATE pm_hzbat_batches_tasks \
			SET  \
			 begin_datetime=? \
			,end_datetime=? \
			,progress=? \
			,error=? \
			,status=?\
		WHERE schedule_name = ? AND \
			batch_name = ? AND \
			order_index = ?",
		"1",R_pm_hzbat_batches_tasks.begin_datetime,20,DT_STR,
		"2",R_pm_hzbat_batches_tasks.end_datetime,20,DT_STR,
		"3",&R_pm_hzbat_batches_tasks.progress,4,DT_ITG,
		"4",&R_pm_hzbat_batches_tasks.error,4,DT_ITG,
		"5",&R_pm_hzbat_batches_tasks.status,4,DT_ITG,
		"6",schedule_name__6,65,DT_STR,
		"7",batch_name__7,65,DT_STR,
		"8",&order_index__8,4,DT_ITG,
		NULL);
        return(r);
}

int DB_pm_hzbat_batches_tasks_add( Pm_Hzbat_Batches_Tasks *_a_data)
{
        int     r;
        memcpy(&R_pm_hzbat_batches_tasks,_a_data,sizeof(Pm_Hzbat_Batches_Tasks));
        r=dbExecSql_va("INSERT INTO pm_hzbat_batches_tasks \
		( schedule_name \n\
		,batch_name \n\
		,order_index \n\
		,program_and_params \n\
		,timeout \n\
		,begin_datetime \n\
		,end_datetime \n\
		,progress \n\
		,error \n\
		,status \n\
		) \
		VALUES ( \n\
			 ? \n\
			,? \n\
			,? \n\
			,? \n\
			,? \n\
			,? \n\
			,? \n\
			,? \n\
			,? \n\
			,?)",
		"1",R_pm_hzbat_batches_tasks.schedule_name,65,DT_STR,
		"2",R_pm_hzbat_batches_tasks.batch_name,65,DT_STR,
		"3",&R_pm_hzbat_batches_tasks.order_index,4,DT_ITG,
		"4",R_pm_hzbat_batches_tasks.program_and_params,257,DT_STR,
		"5",&R_pm_hzbat_batches_tasks.timeout,4,DT_ITG,
		"6",R_pm_hzbat_batches_tasks.begin_datetime,20,DT_STR,
		"7",R_pm_hzbat_batches_tasks.end_datetime,20,DT_STR,
		"8",&R_pm_hzbat_batches_tasks.progress,4,DT_ITG,
		"9",&R_pm_hzbat_batches_tasks.error,4,DT_ITG,
		"10",&R_pm_hzbat_batches_tasks.status,4,DT_ITG,
		NULL);
        return(r);
}


int DB_pm_hzbat_batches_tasks_debug_log(char *reason,Pm_Hzbat_Batches_Tasks *adata, char *filename, int line_no,int priority)
{
	hzb_log_print(filename,line_no,priority,"TABLE [Pm_Hzbat_Batches_Tasks] REASON[%s] LOG",reason);

	hzb_log_print(filename,line_no,priority, "schedule_name: %s", adata->schedule_name );
	hzb_log_print(filename,line_no,priority, "batch_name: %s", adata->batch_name );
	hzb_log_print(filename,line_no,priority, "order_index: %d", adata->order_index );
	hzb_log_print(filename,line_no,priority, "program_and_params: %s", adata->program_and_params );
	hzb_log_print(filename,line_no,priority, "timeout: %d", adata->timeout );
	hzb_log_print(filename,line_no,priority, "begin_datetime: %s", adata->begin_datetime );
	hzb_log_print(filename,line_no,priority, "end_datetime: %s", adata->end_datetime );
	hzb_log_print(filename,line_no,priority, "progress: %d", adata->progress );
	hzb_log_print(filename,line_no,priority, "error: %d", adata->error );
	hzb_log_print(filename,line_no,priority, "status: %d", adata->status );

	return(0);
}


int DB_pm_hzbat_batches_tasks_debug_print(char *reason,Pm_Hzbat_Batches_Tasks *adata, char *filename, int line_no)
{
	DB_pm_hzbat_batches_tasks_debug_log(reason,adata,filename,line_no,HZB_LOG_ERROR);
	return(0);
}

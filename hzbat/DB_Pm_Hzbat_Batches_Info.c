/* 
 This C File "DB_Pm_Hzbat_Batches_Info.c" 
 Genenated By
 Application <db8action> for MySQL
 with the action file "DB_Pm_Hzbat_Batches_Info.act".
 HZBANK All Right reserved.
 Create: Thu Jul  2 14:35:27 2015
*/

#include <stdio.h>
#include "hzb_log.h"
#include "DB_Pm_Hzbat_Batches_Info.h"

static Pm_Hzbat_Batches_Info	R_pm_hzbat_batches_info;

static int Pm_Hzbat_Batches_Info_EraseTailSpace(Pm_Hzbat_Batches_Info *_erase_data)
{
	ERASE_TAIL_SPACE(_erase_data->schedule_name);
	ERASE_TAIL_SPACE(_erase_data->batch_name);
	ERASE_TAIL_SPACE(_erase_data->batch_desc);
	ERASE_TAIL_SPACE(_erase_data->begin_datetime);
	ERASE_TAIL_SPACE(_erase_data->end_datetime);
	ERASE_TAIL_SPACE(_erase_data->pretask_program_and_params);
	return(0);
}
int DB_pm_hzbat_batches_info_count_by_schedule_name( char *schedule_name__0,long *_a_cnt)
{
        int     r;
        long	_a_count;
        DBcurs  _a_curs;

        r=dbCursOpen(&_a_curs);
        if (r!=0)
                return(r);
        r=dbCursDefineSelect_va(&_a_curs,
        	"SELECT COUNT(*)  \
		FROM pm_hzbat_batches_info \
		WHERE schedule_name = ?",
		&_a_count,8,DT_LNG,
		NULL,
		"1",schedule_name__0,75,DT_STR,
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
        *_a_cnt=_a_count;

  E:
        dbCursClose(&_a_curs);
	return(r);
}

int DB_pm_hzbat_batches_info_open_select_by_schedule_name( char *schedule_name__0,Select_Info *_a_sInfo)
{
        int     r;

        r=dbCursOpen(_a_sInfo);
        if (r!=0)
                return(r);
        r=dbCursDefineSelect_va(_a_sInfo,
                "SELECT  \n\
			 schedule_name \n\
			,batch_name \n\
			,batch_desc \n\
			,view_pos_x \n\
			,view_pos_y \n\
			,interrupt_by_app \n\
			,begin_datetime \n\
			,end_datetime \n\
			,progress \n\
			,pretask_program_and_params \n\
			,pretask_timeout \n\
			,pretask_progress \n\
			,pretask_error \n\
			,pretask_status \
		FROM pm_hzbat_batches_info \
		WHERE schedule_name = ?",
		R_pm_hzbat_batches_info.schedule_name,65,DT_STR,
		R_pm_hzbat_batches_info.batch_name,65,DT_STR,
		R_pm_hzbat_batches_info.batch_desc,65,DT_STR,
		&R_pm_hzbat_batches_info.view_pos_x,4,DT_ITG,
		&R_pm_hzbat_batches_info.view_pos_y,4,DT_ITG,
		&R_pm_hzbat_batches_info.interrupt_by_app,4,DT_ITG,
		R_pm_hzbat_batches_info.begin_datetime,20,DT_STR,
		R_pm_hzbat_batches_info.end_datetime,20,DT_STR,
		&R_pm_hzbat_batches_info.progress,4,DT_ITG,
		R_pm_hzbat_batches_info.pretask_program_and_params,257,DT_STR,
		&R_pm_hzbat_batches_info.pretask_timeout,4,DT_ITG,
		&R_pm_hzbat_batches_info.pretask_progress,4,DT_ITG,
		&R_pm_hzbat_batches_info.pretask_error,4,DT_ITG,
		&R_pm_hzbat_batches_info.pretask_status,4,DT_ITG,
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

int DB_pm_hzbat_batches_info_fetch_select( Select_Info *_a_sInfo,Pm_Hzbat_Batches_Info *_a_data)
{
        int     r;

	memset(&R_pm_hzbat_batches_info,0,sizeof(Pm_Hzbat_Batches_Info));
        r=dbCursFetch(_a_sInfo);
        if (r!=0)
        {
                goto E;
        };


	Pm_Hzbat_Batches_Info_EraseTailSpace(&R_pm_hzbat_batches_info);
        memcpy(_a_data,&R_pm_hzbat_batches_info,sizeof(Pm_Hzbat_Batches_Info));

  E:
        return(r);
}

int DB_pm_hzbat_batches_info_close_select( Select_Info *_a_sInfo)
{
        int     r;
        r=dbCursClose(_a_sInfo);
        return(r);
}

int DB_pm_hzbat_batches_info_read_by_schedule_name_and_batch_name( char *schedule_name__0,char *batch_name__1,Pm_Hzbat_Batches_Info *_a_data)
{
        int     r;
        DBcurs  _a_curs;

	memset(&R_pm_hzbat_batches_info,0,sizeof(Pm_Hzbat_Batches_Info));
        r=dbCursOpen(&_a_curs);
        if (r!=0)
                return(r);
        r=dbCursDefineSelect_va(&_a_curs,
                "SELECT  \n\
			 schedule_name \n\
			,batch_name \n\
			,batch_desc \n\
			,view_pos_x \n\
			,view_pos_y \n\
			,interrupt_by_app \n\
			,begin_datetime \n\
			,end_datetime \n\
			,progress \n\
			,pretask_program_and_params \n\
			,pretask_timeout \n\
			,pretask_progress \n\
			,pretask_error \n\
			,pretask_status \
		FROM pm_hzbat_batches_info \
		WHERE schedule_name = ? AND \
			batch_name = ?",
		R_pm_hzbat_batches_info.schedule_name,65,DT_STR,
		R_pm_hzbat_batches_info.batch_name,65,DT_STR,
		R_pm_hzbat_batches_info.batch_desc,65,DT_STR,
		&R_pm_hzbat_batches_info.view_pos_x,4,DT_ITG,
		&R_pm_hzbat_batches_info.view_pos_y,4,DT_ITG,
		&R_pm_hzbat_batches_info.interrupt_by_app,4,DT_ITG,
		R_pm_hzbat_batches_info.begin_datetime,20,DT_STR,
		R_pm_hzbat_batches_info.end_datetime,20,DT_STR,
		&R_pm_hzbat_batches_info.progress,4,DT_ITG,
		R_pm_hzbat_batches_info.pretask_program_and_params,257,DT_STR,
		&R_pm_hzbat_batches_info.pretask_timeout,4,DT_ITG,
		&R_pm_hzbat_batches_info.pretask_progress,4,DT_ITG,
		&R_pm_hzbat_batches_info.pretask_error,4,DT_ITG,
		&R_pm_hzbat_batches_info.pretask_status,4,DT_ITG,
		NULL,
		"1",schedule_name__0,75,DT_STR,
		"2",batch_name__1,75,DT_STR,
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


	Pm_Hzbat_Batches_Info_EraseTailSpace(&R_pm_hzbat_batches_info);
        memcpy(_a_data,&R_pm_hzbat_batches_info,sizeof(Pm_Hzbat_Batches_Info));

  E:
        dbCursClose(&_a_curs);
        return(r);
}

int DB_pm_hzbat_batches_info_update_for_begin_datetime_and_end_datetime_and_progress_by_schedule_name_and_batch_name( char *schedule_name__4,char *batch_name__5,Pm_Hzbat_Batches_Info *_a_data)
{
        int     r;
        memcpy(&R_pm_hzbat_batches_info,_a_data,sizeof(Pm_Hzbat_Batches_Info));
        r=dbExecSql_va("UPDATE pm_hzbat_batches_info \
			SET  \
			 begin_datetime=? \
			,end_datetime=? \
			,progress=?\
		WHERE schedule_name = ? AND \
			batch_name = ?",
		"1",R_pm_hzbat_batches_info.begin_datetime,20,DT_STR,
		"2",R_pm_hzbat_batches_info.end_datetime,20,DT_STR,
		"3",&R_pm_hzbat_batches_info.progress,4,DT_ITG,
		"4",schedule_name__4,65,DT_STR,
		"5",batch_name__5,65,DT_STR,
		NULL);
        return(r);
}

int DB_pm_hzbat_batches_info_update_for_pretask_progress_and_pretask_error_and_pretask_status_by_schedule_name_and_batch_name( char *schedule_name__4,char *batch_name__5,Pm_Hzbat_Batches_Info *_a_data)
{
        int     r;
        memcpy(&R_pm_hzbat_batches_info,_a_data,sizeof(Pm_Hzbat_Batches_Info));
        r=dbExecSql_va("UPDATE pm_hzbat_batches_info \
			SET  \
			 pretask_progress=? \
			,pretask_error=? \
			,pretask_status=?\
		WHERE schedule_name = ? AND \
			batch_name = ?",
		"1",&R_pm_hzbat_batches_info.pretask_progress,4,DT_ITG,
		"2",&R_pm_hzbat_batches_info.pretask_error,4,DT_ITG,
		"3",&R_pm_hzbat_batches_info.pretask_status,4,DT_ITG,
		"4",schedule_name__4,65,DT_STR,
		"5",batch_name__5,65,DT_STR,
		NULL);
        return(r);
}


int DB_pm_hzbat_batches_info_debug_log(char *reason,Pm_Hzbat_Batches_Info *adata, char *filename, int line_no,int priority)
{
	hzb_log_print(filename,line_no,priority,"TABLE [Pm_Hzbat_Batches_Info] REASON[%s] LOG",reason);

	hzb_log_print(filename,line_no,priority, "schedule_name: %s", adata->schedule_name );
	hzb_log_print(filename,line_no,priority, "batch_name: %s", adata->batch_name );
	hzb_log_print(filename,line_no,priority, "batch_desc: %s", adata->batch_desc );
	hzb_log_print(filename,line_no,priority, "view_pos_x: %d", adata->view_pos_x );
	hzb_log_print(filename,line_no,priority, "view_pos_y: %d", adata->view_pos_y );
	hzb_log_print(filename,line_no,priority, "interrupt_by_app: %d", adata->interrupt_by_app );
	hzb_log_print(filename,line_no,priority, "begin_datetime: %s", adata->begin_datetime );
	hzb_log_print(filename,line_no,priority, "end_datetime: %s", adata->end_datetime );
	hzb_log_print(filename,line_no,priority, "progress: %d", adata->progress );
	hzb_log_print(filename,line_no,priority, "pretask_program_and_params: %s", adata->pretask_program_and_params );
	hzb_log_print(filename,line_no,priority, "pretask_timeout: %d", adata->pretask_timeout );
	hzb_log_print(filename,line_no,priority, "pretask_progress: %d", adata->pretask_progress );
	hzb_log_print(filename,line_no,priority, "pretask_error: %d", adata->pretask_error );
	hzb_log_print(filename,line_no,priority, "pretask_status: %d", adata->pretask_status );

	return(0);
}


int DB_pm_hzbat_batches_info_debug_print(char *reason,Pm_Hzbat_Batches_Info *adata, char *filename, int line_no)
{
	DB_pm_hzbat_batches_info_debug_log(reason,adata,filename,line_no,HZB_LOG_ERROR);
	return(0);
}

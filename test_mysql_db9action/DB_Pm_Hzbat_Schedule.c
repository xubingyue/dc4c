/* 
 This C File "DB_Pm_Hzbat_Schedule.c" 
 Genenated By
 Application <db8action> for MySQL
 with the action file "DB_Pm_Hzbat_Schedule.act".
 HZBANK All Right reserved.
 Create: Thu Jul  2 14:35:27 2015
*/

#include <stdio.h>
#include "hzb_log.h"
#include "DB_Pm_Hzbat_Schedule.h"

static Pm_Hzbat_Schedule	R_pm_hzbat_schedule;

static int Pm_Hzbat_Schedule_EraseTailSpace(Pm_Hzbat_Schedule *_erase_data)
{
	ERASE_TAIL_SPACE(_erase_data->schedule_name);
	ERASE_TAIL_SPACE(_erase_data->schedule_desc);
	ERASE_TAIL_SPACE(_erase_data->begin_datetime);
	ERASE_TAIL_SPACE(_erase_data->end_datetime);
	return(0);
}
int DB_pm_hzbat_schedule_read_by_schedule_name( char *schedule_name__0,Pm_Hzbat_Schedule *_a_data)
{
        int     r;
        DBcurs  _a_curs;

	memset(&R_pm_hzbat_schedule,0,sizeof(Pm_Hzbat_Schedule));
        r=dbCursOpen(&_a_curs);
        if (r!=0)
                return(r);
        r=dbCursDefineSelect_va(&_a_curs,
                "SELECT  \n\
			 order_index \n\
			,schedule_name \n\
			,schedule_desc \n\
			,begin_datetime \n\
			,end_datetime \n\
			,progress \
		FROM pm_hzbat_schedule \
		WHERE schedule_name = ?",
		&R_pm_hzbat_schedule.order_index,4,DT_ITG,
		R_pm_hzbat_schedule.schedule_name,65,DT_STR,
		R_pm_hzbat_schedule.schedule_desc,257,DT_STR,
		R_pm_hzbat_schedule.begin_datetime,20,DT_STR,
		R_pm_hzbat_schedule.end_datetime,20,DT_STR,
		&R_pm_hzbat_schedule.progress,4,DT_ITG,
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


	Pm_Hzbat_Schedule_EraseTailSpace(&R_pm_hzbat_schedule);
        memcpy(_a_data,&R_pm_hzbat_schedule,sizeof(Pm_Hzbat_Schedule));

  E:
        dbCursClose(&_a_curs);
        return(r);
}

int DB_pm_hzbat_schedule_update_for_begin_datetime_and_progress_by_schedule_name( char *schedule_name__3,Pm_Hzbat_Schedule *_a_data)
{
        int     r;
        memcpy(&R_pm_hzbat_schedule,_a_data,sizeof(Pm_Hzbat_Schedule));
        r=dbExecSql_va("UPDATE pm_hzbat_schedule \
			SET  \
			 begin_datetime=? \
			,progress=?\
		WHERE schedule_name = ?",
		"1",R_pm_hzbat_schedule.begin_datetime,20,DT_STR,
		"2",&R_pm_hzbat_schedule.progress,4,DT_ITG,
		"3",schedule_name__3,65,DT_STR,
		NULL);
        return(r);
}

int DB_pm_hzbat_schedule_update_for_end_datetime_and_progress_by_schedule_name( char *schedule_name__3,Pm_Hzbat_Schedule *_a_data)
{
        int     r;
        memcpy(&R_pm_hzbat_schedule,_a_data,sizeof(Pm_Hzbat_Schedule));
        r=dbExecSql_va("UPDATE pm_hzbat_schedule \
			SET  \
			 end_datetime=? \
			,progress=?\
		WHERE schedule_name = ?",
		"1",R_pm_hzbat_schedule.end_datetime,20,DT_STR,
		"2",&R_pm_hzbat_schedule.progress,4,DT_ITG,
		"3",schedule_name__3,65,DT_STR,
		NULL);
        return(r);
}

int DB_pm_hzbat_schedule_open_select_by_order_index_GE_order_by_order_index( int order_index__GE_0,Select_Info *_a_sInfo)
{
        int     r;

        r=dbCursOpen(_a_sInfo);
        if (r!=0)
                return(r);
        r=dbCursDefineSelect_va(_a_sInfo,
                "SELECT  \n\
			 order_index \n\
			,schedule_name \n\
			,schedule_desc \n\
			,begin_datetime \n\
			,end_datetime \n\
			,progress \
		FROM pm_hzbat_schedule \
		WHERE order_index >= ? \
		ORDER BY order_index",
		&R_pm_hzbat_schedule.order_index,4,DT_ITG,
		R_pm_hzbat_schedule.schedule_name,65,DT_STR,
		R_pm_hzbat_schedule.schedule_desc,257,DT_STR,
		R_pm_hzbat_schedule.begin_datetime,20,DT_STR,
		R_pm_hzbat_schedule.end_datetime,20,DT_STR,
		&R_pm_hzbat_schedule.progress,4,DT_ITG,
		NULL,
		"1",&order_index__GE_0,4,DT_ITG,
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

int DB_pm_hzbat_schedule_fetch_select( Select_Info *_a_sInfo,Pm_Hzbat_Schedule *_a_data)
{
        int     r;

	memset(&R_pm_hzbat_schedule,0,sizeof(Pm_Hzbat_Schedule));
        r=dbCursFetch(_a_sInfo);
        if (r!=0)
        {
                goto E;
        };


	Pm_Hzbat_Schedule_EraseTailSpace(&R_pm_hzbat_schedule);
        memcpy(_a_data,&R_pm_hzbat_schedule,sizeof(Pm_Hzbat_Schedule));

  E:
        return(r);
}

int DB_pm_hzbat_schedule_close_select( Select_Info *_a_sInfo)
{
        int     r;
        r=dbCursClose(_a_sInfo);
        return(r);
}


int DB_pm_hzbat_schedule_debug_log(char *reason,Pm_Hzbat_Schedule *adata, char *filename, int line_no,int priority)
{
	hzb_log_print(filename,line_no,priority,"TABLE [Pm_Hzbat_Schedule] REASON[%s] LOG",reason);

	hzb_log_print(filename,line_no,priority, "order_index: %d", adata->order_index );
	hzb_log_print(filename,line_no,priority, "schedule_name: %s", adata->schedule_name );
	hzb_log_print(filename,line_no,priority, "schedule_desc: %s", adata->schedule_desc );
	hzb_log_print(filename,line_no,priority, "begin_datetime: %s", adata->begin_datetime );
	hzb_log_print(filename,line_no,priority, "end_datetime: %s", adata->end_datetime );
	hzb_log_print(filename,line_no,priority, "progress: %d", adata->progress );

	return(0);
}


int DB_pm_hzbat_schedule_debug_print(char *reason,Pm_Hzbat_Schedule *adata, char *filename, int line_no)
{
	DB_pm_hzbat_schedule_debug_log(reason,adata,filename,line_no,HZB_LOG_ERROR);
	return(0);
}

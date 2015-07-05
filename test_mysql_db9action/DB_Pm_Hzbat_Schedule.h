/*
 This Include File "DB_Pm_Hzbat_Schedule.h" 
 Genenated By
 Application <db9action> for MySQL
 with the action file "DB_Pm_Hzbat_Schedule.act".
 Create: Thu Jul  2 14:35:27 2015
*/

#ifndef _DB_pm_hzbat_schedule_INCLUDE_H_
#define _DB_pm_hzbat_schedule_INCLUDE_H_
#include "dbutil.h"

typedef struct {
	int	order_index;
	char	schedule_name[65];
	char	schedule_desc[257];
	char	begin_datetime[20];
	char	end_datetime[20];
	int	progress;
        } Pm_Hzbat_Schedule;

int DB_pm_hzbat_schedule_read_by_schedule_name( char *schedule_name__0,Pm_Hzbat_Schedule *_a_data);
int DB_pm_hzbat_schedule_update_for_begin_datetime_and_progress_by_schedule_name( char *schedule_name__3,Pm_Hzbat_Schedule *_a_data);
int DB_pm_hzbat_schedule_update_for_end_datetime_and_progress_by_schedule_name( char *schedule_name__3,Pm_Hzbat_Schedule *_a_data);
int DB_pm_hzbat_schedule_open_select_by_order_index_GE_order_by_order_index( int order_index__GE_0,Select_Info *_a_sInfo);
int DB_pm_hzbat_schedule_fetch_select( Select_Info *_a_sInfo,Pm_Hzbat_Schedule *_a_data);
int DB_pm_hzbat_schedule_close_select( Select_Info *_a_sInfo);

#endif

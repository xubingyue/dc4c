/*
 This Include File "DB_Pm_Hzbat_Batches_Tasks.h" 
 Genenated By
 Application <db9action> for MySQL
 with the action file "DB_Pm_Hzbat_Batches_Tasks.act".
 Create: Thu Jul  2 14:35:28 2015
*/

#ifndef _DB_pm_hzbat_batches_tasks_INCLUDE_H_
#define _DB_pm_hzbat_batches_tasks_INCLUDE_H_
#include "dbutil.h"

typedef struct {
	char	schedule_name[65];
	char	batch_name[65];
	int	order_index;
	char	program_and_params[257];
	int	timeout;
	char	begin_datetime[20];
	char	end_datetime[20];
	int	progress;
	int	error;
	int	status;
        } Pm_Hzbat_Batches_Tasks;

int DB_pm_hzbat_batches_tasks_open_select_by_schedule_name( char *schedule_name__0,Select_Info *_a_sInfo);
int DB_pm_hzbat_batches_tasks_open_select_by_schedule_name_and_batch_name_order_by_order_index( char *schedule_name__0,char *batch_name__1,Select_Info *_a_sInfo);
int DB_pm_hzbat_batches_tasks_fetch_select( Select_Info *_a_sInfo,Pm_Hzbat_Batches_Tasks *_a_data);
int DB_pm_hzbat_batches_tasks_close_select( Select_Info *_a_sInfo);
int DB_pm_hzbat_batches_tasks_read_by_schedule_name_and_batch_name_and_order_index( char *schedule_name__0,char *batch_name__1,int order_index__2,Pm_Hzbat_Batches_Tasks *_a_data);
int DB_pm_hzbat_batches_tasks_update_for_begin_datetime_and_end_datetime_and_progress_and_error_and_status_by_schedule_name_and_batch_name_and_order_index( char *schedule_name__6,char *batch_name__7,int order_index__8,Pm_Hzbat_Batches_Tasks *_a_data);
int DB_pm_hzbat_batches_tasks_add( Pm_Hzbat_Batches_Tasks *_a_data);

#endif

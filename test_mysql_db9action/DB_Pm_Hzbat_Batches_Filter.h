/*
 This Include File "DB_Pm_Hzbat_Batches_Filter.h" 
 Genenated By
 Application <db9action> for MySQL
 with the action file "DB_Pm_Hzbat_Batches_Filter.act".
 Create: Thu Jul  2 14:35:28 2015
*/

#ifndef _DB_pm_hzbat_batches_filter_INCLUDE_H_
#define _DB_pm_hzbat_batches_filter_INCLUDE_H_
#include "dbutil.h"

typedef struct {
	char	schedule_name[65];
	char	batch_name[65];
	char	filter_type[17];
	char	filter_param[1025];
        } Pm_Hzbat_Batches_Filter;

int DB_pm_hzbat_batches_filter_read_by_schedule_name_and_batch_name( char *schedule_name__0,char *batch_name__1,Pm_Hzbat_Batches_Filter *_a_data);

#endif

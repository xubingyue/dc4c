/*
 This Include File "DB_Pm_Hzbat_Batches_Direction.h" 
 Genenated By
 Application <db9action> for MySQL
 with the action file "DB_Pm_Hzbat_Batches_Direction.act".
 Create: Thu Jul  2 14:35:27 2015
*/

#ifndef _DB_pm_hzbat_batches_direction_INCLUDE_H_
#define _DB_pm_hzbat_batches_direction_INCLUDE_H_
#include "dbutil.h"

typedef struct {
	char	schedule_name[65];
	char	from_batch[65];
	char	to_batch[65];
        } Pm_Hzbat_Batches_Direction;

int DB_pm_hzbat_batches_direction_open_select_by_schedule_name( char *schedule_name__0,Select_Info *_a_sInfo);
int DB_pm_hzbat_batches_direction_fetch_select( Select_Info *_a_sInfo,Pm_Hzbat_Batches_Direction *_a_data);
int DB_pm_hzbat_batches_direction_close_select( Select_Info *_a_sInfo);

#endif

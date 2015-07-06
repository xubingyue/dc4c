/* 
 This C File "DB_Pm_Hzbat_Batches_Direction.c" 
 Genenated By
 Application <db8action> for MySQL
 with the action file "DB_Pm_Hzbat_Batches_Direction.act".
 HZBANK All Right reserved.
 Create: Thu Jul  2 14:35:27 2015
*/

#include <stdio.h>
#include "hzb_log.h"
#include "DB_Pm_Hzbat_Batches_Direction.h"

static Pm_Hzbat_Batches_Direction	R_pm_hzbat_batches_direction;

static int Pm_Hzbat_Batches_Direction_EraseTailSpace(Pm_Hzbat_Batches_Direction *_erase_data)
{
	ERASE_TAIL_SPACE(_erase_data->schedule_name);
	ERASE_TAIL_SPACE(_erase_data->from_batch);
	ERASE_TAIL_SPACE(_erase_data->to_batch);
	return(0);
}
int DB_pm_hzbat_batches_direction_open_select_by_schedule_name( char *schedule_name__0,Select_Info *_a_sInfo)
{
        int     r;

        r=dbCursOpen(_a_sInfo);
        if (r!=0)
                return(r);
        r=dbCursDefineSelect_va(_a_sInfo,
                "SELECT  \n\
			 schedule_name \n\
			,from_batch \n\
			,to_batch \
		FROM pm_hzbat_batches_direction \
		WHERE schedule_name = ?",
		R_pm_hzbat_batches_direction.schedule_name,65,DT_STR,
		R_pm_hzbat_batches_direction.from_batch,65,DT_STR,
		R_pm_hzbat_batches_direction.to_batch,65,DT_STR,
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

int DB_pm_hzbat_batches_direction_fetch_select( Select_Info *_a_sInfo,Pm_Hzbat_Batches_Direction *_a_data)
{
        int     r;

	memset(&R_pm_hzbat_batches_direction,0,sizeof(Pm_Hzbat_Batches_Direction));
        r=dbCursFetch(_a_sInfo);
        if (r!=0)
        {
                goto E;
        };


	Pm_Hzbat_Batches_Direction_EraseTailSpace(&R_pm_hzbat_batches_direction);
        memcpy(_a_data,&R_pm_hzbat_batches_direction,sizeof(Pm_Hzbat_Batches_Direction));

  E:
        return(r);
}

int DB_pm_hzbat_batches_direction_close_select( Select_Info *_a_sInfo)
{
        int     r;
        r=dbCursClose(_a_sInfo);
        return(r);
}


int DB_pm_hzbat_batches_direction_debug_log(char *reason,Pm_Hzbat_Batches_Direction *adata, char *filename, int line_no,int priority)
{
	hzb_log_print(filename,line_no,priority,"TABLE [Pm_Hzbat_Batches_Direction] REASON[%s] LOG",reason);

	hzb_log_print(filename,line_no,priority, "schedule_name: %s", adata->schedule_name );
	hzb_log_print(filename,line_no,priority, "from_batch: %s", adata->from_batch );
	hzb_log_print(filename,line_no,priority, "to_batch: %s", adata->to_batch );

	return(0);
}


int DB_pm_hzbat_batches_direction_debug_print(char *reason,Pm_Hzbat_Batches_Direction *adata, char *filename, int line_no)
{
	DB_pm_hzbat_batches_direction_debug_log(reason,adata,filename,line_no,HZB_LOG_ERROR);
	return(0);
}

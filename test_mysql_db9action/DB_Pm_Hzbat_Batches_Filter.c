/* 
 This C File "DB_Pm_Hzbat_Batches_Filter.c" 
 Genenated By
 Application <db8action> for MySQL
 with the action file "DB_Pm_Hzbat_Batches_Filter.act".
 HZBANK All Right reserved.
 Create: Thu Jul  2 14:35:28 2015
*/

#include <stdio.h>
#include "hzb_log.h"
#include "DB_Pm_Hzbat_Batches_Filter.h"

static Pm_Hzbat_Batches_Filter	R_pm_hzbat_batches_filter;

static int Pm_Hzbat_Batches_Filter_EraseTailSpace(Pm_Hzbat_Batches_Filter *_erase_data)
{
	ERASE_TAIL_SPACE(_erase_data->schedule_name);
	ERASE_TAIL_SPACE(_erase_data->batch_name);
	ERASE_TAIL_SPACE(_erase_data->filter_type);
	ERASE_TAIL_SPACE(_erase_data->filter_param);
	return(0);
}
int DB_pm_hzbat_batches_filter_read_by_schedule_name_and_batch_name( char *schedule_name__0,char *batch_name__1,Pm_Hzbat_Batches_Filter *_a_data)
{
        int     r;
        DBcurs  _a_curs;

	memset(&R_pm_hzbat_batches_filter,0,sizeof(Pm_Hzbat_Batches_Filter));
        r=dbCursOpen(&_a_curs);
        if (r!=0)
                return(r);
        r=dbCursDefineSelect_va(&_a_curs,
                "SELECT  \n\
			 schedule_name \n\
			,batch_name \n\
			,filter_type \n\
			,filter_param \
		FROM pm_hzbat_batches_filter \
		WHERE schedule_name = ? AND \
			batch_name = ?",
		R_pm_hzbat_batches_filter.schedule_name,65,DT_STR,
		R_pm_hzbat_batches_filter.batch_name,65,DT_STR,
		R_pm_hzbat_batches_filter.filter_type,17,DT_STR,
		R_pm_hzbat_batches_filter.filter_param,1025,DT_STR,
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


	Pm_Hzbat_Batches_Filter_EraseTailSpace(&R_pm_hzbat_batches_filter);
        memcpy(_a_data,&R_pm_hzbat_batches_filter,sizeof(Pm_Hzbat_Batches_Filter));

  E:
        dbCursClose(&_a_curs);
        return(r);
}


int DB_pm_hzbat_batches_filter_debug_log(char *reason,Pm_Hzbat_Batches_Filter *adata, char *filename, int line_no,int priority)
{
	hzb_log_print(filename,line_no,priority,"TABLE [Pm_Hzbat_Batches_Filter] REASON[%s] LOG",reason);

	hzb_log_print(filename,line_no,priority, "schedule_name: %s", adata->schedule_name );
	hzb_log_print(filename,line_no,priority, "batch_name: %s", adata->batch_name );
	hzb_log_print(filename,line_no,priority, "filter_type: %s", adata->filter_type );
	hzb_log_print(filename,line_no,priority, "filter_param: %s", adata->filter_param );

	return(0);
}


int DB_pm_hzbat_batches_filter_debug_print(char *reason,Pm_Hzbat_Batches_Filter *adata, char *filename, int line_no)
{
	DB_pm_hzbat_batches_filter_debug_log(reason,adata,filename,line_no,HZB_LOG_ERROR);
	return(0);
}

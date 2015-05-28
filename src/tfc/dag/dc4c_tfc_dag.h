/*
 * tfc_dag for dc4c - Tasks flow controler for dc4c
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#ifndef _H_DC4C_TFC_DAG_
#define _H_DC4C_TFC_DAG_

#include "dc4c_api.h"
#include "ListX.h"
#include "LOGC.h"

#include "IDL_dag_schedule_configfile.dsc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Dc4cDagSchedule ;

/********* ¸ß²ãº¯Êý *********/

int DC4CLoadDagScheduleFromFile( struct Dc4cDagSchedule **pp_sched , char *pathfilename , int options );
int DC4CLoadDagScheduleFromDatabase( struct Dc4cDagSchedule **pp_sched , char *schedule_name , int options );
void DC4CLogDagSchedule( struct Dc4cDagSchedule *p_sched );
int DC4CExecuteDagSchedule( struct Dc4cDagSchedule *p_sched , char *rservers_ip_port );
void DC4CUnloadDagSchedule( struct Dc4cDagSchedule **pp_sched );

#define DC4C_DAGSCHELDULE_PROGRESS_INIT		0
#define DC4C_DAGSCHELDULE_PROGRESS_EXECUTING	1
#define DC4C_DAGSCHELDULE_PROGRESS_FINISHED	2
int DC4CGetDagScheduleProgress( struct Dc4cDagSchedule *p_sched );
int DC4CGetDagScheduleResult( struct Dc4cDagSchedule *p_sched );

/********* µÍ²ãº¯Êý *********/

int DC4CLoadDagScheduleFromStruct( struct Dc4cDagSchedule **pp_sched , dag_schedule_configfile *p_config , int options );

int DC4CInitDagSchedule( struct Dc4cDagSchedule *p_sched , char *schedule_name , char *schedule_desc );
void DC4CCleanDagSchedule( struct Dc4cDagSchedule *p_sched );

struct Dc4cDagBatch *DC4CAllocDagBatch( struct Dc4cDagSchedule *p_sched , char *batch_name , char *batch_desc , int view_pos_x , int view_pos_y );
BOOL DC4CFreeDagBatch( void *pv );

int DC4CLinkDagBatch( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_parent_batch , struct Dc4cDagBatch *p_batch );

void DC4CSetDagBatchTasks( struct Dc4cDagBatch *p_batch , struct Dc4cBatchTask *a_tasks , int tasks_count );
struct Dc4cApiEnv **DC4CGetDagBatchApiEnvPPtr( struct Dc4cDagBatch *p_batch );
void DC4CGetDagBatchBeginDatetime( struct Dc4cDagBatch *p_batch , char begin_datetime[19+1] , long *p_begin_datetime_stamp );
void DC4CGetDagBatchEndDatetime( struct Dc4cDagBatch *p_batch , char end_datetime[19+1] , long *p_end_datetime_stamp );
#define DC4C_DAGBATCH_PROGRESS_INIT		0
#define DC4C_DAGBATCH_PROGRESS_EXECUTING	1
#define DC4C_DAGBATCH_PROGRESS_FINISHED		2
void DC4CGetDagBatchProgress( struct Dc4cDagBatch *p_batch , int *p_progress );
void DC4CGetDagBatchResult( struct Dc4cDagBatch *p_batch , int *p_result );

#ifdef __cplusplus
}
#endif

#endif

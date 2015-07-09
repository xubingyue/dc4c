/*
 * tfc_dag - DAG tasks engine for dc4c
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#ifndef _H_DC4C_TFC_DAG_
#define _H_DC4C_TFC_DAG_

#include "dc4c_util.h"
#include "dc4c_api.h"

#include "IDL_dag_schedule_configfile.dsc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DC4C_DAGSCHEDULE_PROGRESS_INIT			0
#define DC4C_DAGSCHEDULE_PROGRESS_EXECUTING		1
#define DC4C_DAGSCHEDULE_PROGRESS_FINISHED		2
#define DC4C_DAGSCHEDULE_PROGRESS_FINISHED_WITH_ERROR	4

struct Dc4cDagSchedule ;

#define DC4C_DAGBATCH_PROGRESS_INIT			0
#define DC4C_DAGBATCH_PROGRESS_EXECUTING		1
#define DC4C_DAGBATCH_PROGRESS_FINISHED			2
#define DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR	4

struct Dc4cDagBatch ;

#define DC4C_DAGTASK_PROGRESS_INIT			0
#define DC4C_DAGTASK_PROGRESS_EXECUTING			1
#define DC4C_DAGTASK_PROGRESS_FINISHED			2
#define DC4C_DAGTASK_PROGRESS_FINISHED_WITH_ERROR	4

/********* 高层函数类 *********/

int DC4CLoadDagScheduleFromFile( struct Dc4cDagSchedule **pp_sched , char *pathfilename , char *rservers_ip_port , int options );
void DC4CUnloadDagSchedule( struct Dc4cDagSchedule **pp_sched );

void DC4CLogDagSchedule( struct Dc4cDagSchedule *p_sched );

int DC4CExecuteDagSchedule( struct Dc4cDagSchedule *p_sched );
int DC4CDagScheduleInterruptCode( struct Dc4cDagSchedule *p_sched );

int DC4CBeginDagSchedule( struct Dc4cDagSchedule *p_sched );
int DC4CPerformDagSchedule( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch **pp_batch , struct Dc4cApiEnv **ppenv , int *p_task_index );

char *DC4CGetDagScheduleName( struct Dc4cDagSchedule *p_sched );
int DC4CGetDagScheduleProgress( struct Dc4cDagSchedule *p_sched );

char *DC4CGetDagBatchName( struct Dc4cDagBatch *p_batch );
void DC4CSetDagBatchTasks( struct Dc4cDagBatch *p_batch , struct Dc4cBatchTask *a_tasks , int tasks_count );
struct Dc4cApiEnv *DC4CGetDagBatchApiEnvPtr( struct Dc4cDagBatch *p_batch );
struct Dc4cApiEnv **DC4CGetDagBatchApiEnvPPtr( struct Dc4cDagBatch *p_batch );
char *DC4CGetDagBatchBeginDatetime( struct Dc4cDagBatch *p_batch );
char *DC4CGetDagBatchEndDatetime( struct Dc4cDagBatch *p_batch );
int DC4CGetDagBatchProgress( struct Dc4cDagBatch *p_batch );

/********* 低层函数类 *********/

int DC4CLoadDagScheduleFromStruct( struct Dc4cDagSchedule **pp_sched , dag_schedule_configfile *p_config , char *rservers_ip_port , int options );

struct Dc4cDagBatch *DC4CAllocDagBatch( struct Dc4cDagSchedule *p_sched , char *batch_name , char *batch_desc , int view_pos_x , int view_pos_y );
BOOL DC4CFreeDagBatch( void *pv );

int DC4CLinkDagBatch( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_parent_batch , struct Dc4cDagBatch *p_batch );

/********* 回调钩子类 *********/

void DC4CSetDagBatchProcDataPtr( struct Dc4cDagSchedule *p_sched , void *p1 );

typedef void funcDC4COnBeginDagBatchProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , void *p1 );
typedef void funcDC4COnFinishDagBatchProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , void *p1 );

void DC4CSetOnBeginDagBatchProc( struct Dc4cDagSchedule *p_sched , funcDC4COnBeginDagBatchProc *pfuncDC4COnBeginDagBatchProc );
void DC4CSetOnFinishDagBatchProc( struct Dc4cDagSchedule *p_sched , funcDC4COnFinishDagBatchProc *pfuncDC4COnFinishDagBatchProc );

typedef void funcDC4COnBeginDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , int task_index , void *p1 );
typedef void funcDC4COnCancelDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , int task_index , void *p1 );
typedef void funcDC4COnFinishDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_batch , struct Dc4cApiEnv *penv , int task_index , void *p1 );

void DC4CSetOnBeginDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , funcDC4COnBeginDagBatchTaskProc *pfuncDC4COnBeginDagBatchTaskProc );
void DC4CSetOnCancelDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , funcDC4COnCancelDagBatchTaskProc *pfuncDC4COnCancelDagBatchTaskProc );
void DC4CSetOnFinishDagBatchTaskProc( struct Dc4cDagSchedule *p_sched , funcDC4COnFinishDagBatchTaskProc *pfuncDC4COnFinishDagBatchTaskProc );

#ifdef __cplusplus
}
#endif

#endif

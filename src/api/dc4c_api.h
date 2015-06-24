/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#ifndef _H_DC4C_API_
#define _H_DC4C_API_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "LOGC.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DC4C_ERROR_INTERNAL			-101
#define DC4C_ERROR_SOCKET			-102
#define DC4C_ERROR_CONNECT			-103
#define DC4C_ERROR_ALLOC			-104
#define DC4C_ERROR_OPENFILE			-105
#define DC4C_ERROR_WRITEFILE			-106
#define DC4C_ERROR_CREATEPIPE			-107
#define DC4C_ERROR_FORK				-108
#define DC4C_ERROR_EXEC				-109
#define DC4C_ERROR_SEND_OR_RECEIVE		-110
#define DC4C_ERROR_PARAMETER			-201
#define DC4C_ERROR_FILE_NOT_FOUND		-202
#define DC4C_ERROR_NO_WORKER			-203
#define DC4C_ERROR_DATABASE			-204
#define DC4C_ERROR_MD5_NOT_MATCHED_TOO		-206
#define DC4C_ERROR_TERMSIG			-301
#define DC4C_ERROR_SIGNALED			-302
#define DC4C_ERROR_STOPPED			-303
#define DC4C_ERROR_UNKNOW_QUIT			-304
#define DC4C_ERROR_TIMEOUT			-401
#define DC4C_ERROR_APP				-402
#define DC4C_INFO_ALREADY_EXECUTING		501
#define DC4C_INFO_NO_RUNNING			602
#define DC4C_INFO_TASK_FINISHED			603
#define DC4C_INFO_BATCH_TASKS_FINISHED		604
#define DC4C_INFO_ALL_ENVS_FINISHED		605

#define DC4C_DEFAULT_TIMEOUT			0

#define DC4C_WORKER_COUNT_UNLIMITED		0

#define DC4C_OPTIONS_INTERRUPT_BY_APP		1
#define DC4C_OPTIONS_BIND_CPU			2

struct Dc4cApiEnv ;

/********* 用户节点接口 *********/

/* API环境类 */

int DC4CInitEnv( struct Dc4cApiEnv **ppenv , char *rservers_ip_port );
void DC4CCleanEnv( struct Dc4cApiEnv **ppenv );

void DC4CSetTimeout( struct Dc4cApiEnv *penv , int timeout );
int DC4CGetTimeout( struct Dc4cApiEnv *penv );

void DC4CSetOptions( struct Dc4cApiEnv *penv , unsigned long options );
unsigned long DC4CGetOptions( struct Dc4cApiEnv *penv );

/* 同步发起任务类 */

int DC4CDoTask( struct Dc4cApiEnv *penv , char *program_and_params , int timeout );

char *DC4CGetTaskIp( struct Dc4cApiEnv *penv );
long DC4CGetTaskPort( struct Dc4cApiEnv *penv );
char *DC4CGetTaskTid( struct Dc4cApiEnv *penv );
int DC4CGetTaskOrderIndex( struct Dc4cApiEnv *penv );
char *DC4CGetTaskProgramAndParams( struct Dc4cApiEnv *penv );
int DC4CGetTaskTimeout( struct Dc4cApiEnv *penv );
int DC4CGetTaskBeginTimestamp( struct Dc4cApiEnv *penv );
int DC4CGetTaskEndTimestamp( struct Dc4cApiEnv *penv );
int DC4CGetTaskElapse( struct Dc4cApiEnv *penv );
int DC4CGetTaskError( struct Dc4cApiEnv *penv );
int DC4CGetTaskStatus( struct Dc4cApiEnv *penv );
char *DC4CGetTaskInfo( struct Dc4cApiEnv *penv );

struct Dc4cBatchTask
{
	/* 必填字段 */
	char	program_and_params[ 256 + 1 ] ;
	int	timeout ;
	
	/* DAG必填字段 */
	int	order_index ;
	
	/* 预留字段 */
	char	ip[ 40 + 1 ] ;
} ;

int DC4CDoBatchTasks( struct Dc4cApiEnv *penv , int workers_count , struct Dc4cBatchTask *a_tasks , int tasks_count );

char *DC4CGetBatchTasksIp( struct Dc4cApiEnv *penv , int task_index ); /* task_index based 0 */
long DC4CGetBatchTasksPort( struct Dc4cApiEnv *penv , int task_index );
char *DC4CGetBatchTasksTid( struct Dc4cApiEnv *penv , int task_index );
int DC4CGetBatchTasksOrderIndex( struct Dc4cApiEnv *penv , int task_index );
char *DC4CGetBatchTasksProgramAndParams( struct Dc4cApiEnv *penv , int task_index );
int DC4CGetBatchTasksTimeout( struct Dc4cApiEnv *penv , int task_index );
int DC4CGetBatchTasksBeginTimestamp( struct Dc4cApiEnv *penv , int task_index );
int DC4CGetBatchTasksEndTimestamp( struct Dc4cApiEnv *penv , int task_index );
int DC4CGetBatchTasksElapse( struct Dc4cApiEnv *penv , int task_index );
int DC4CGetBatchTasksError( struct Dc4cApiEnv *penv , int task_index );
int DC4CGetBatchTasksStatus( struct Dc4cApiEnv *penv , int task_index );
char *DC4CGetBatchTasksInfo( struct Dc4cApiEnv *penv , int task_index );

int DC4CDoMultiBatchTasks( struct Dc4cApiEnv **a_penv , int envs_count , int *a_workers_count , struct Dc4cBatchTask **aa_tasks , int *a_tasks_count );

/* 低层函数类 */

int DC4CBeginBatchTasks( struct Dc4cApiEnv *penv , int workers_count , struct Dc4cBatchTask *a_tasks , int tasks_count );
int DC4CPerformBatchTasks( struct Dc4cApiEnv *penv , int *p_task_index );
int DC4CPerformMultiBatchTasks( struct Dc4cApiEnv **a_penv , int envs_count , struct Dc4cApiEnv **ppenv , int *p_task_index );

int DC4CQueryWorkers( struct Dc4cApiEnv *penv );
int DC4CSetTasksFds( struct Dc4cApiEnv *penv , struct Dc4cApiEnv *penv_QueryWorkers , fd_set *read_fds , fd_set *write_fds , fd_set *expect_fds , int *p_max_fd );
int DC4CSelectTasksFds( fd_set *p_read_fds , fd_set *write_fds , fd_set *expect_fds , int *p_max_fd , int select_timeout );
int DC4CProcessTasks( struct Dc4cApiEnv *penv , fd_set *p_read_fds , fd_set *write_fds , fd_set *expect_fds , int *p_task_index );

/* 其它类 */

int DC4CGetTasksCount( struct Dc4cApiEnv *penv );
int DC4CGetWorkersCount( struct Dc4cApiEnv *penv );
int DC4CGetPrepareTasksCount( struct Dc4cApiEnv *penv );
int DC4CGetRunningTasksCount( struct Dc4cApiEnv *penv );
int DC4CGetFinishedTasksCount( struct Dc4cApiEnv *penv );

void DC4CResetFinishedTasksWithError( struct Dc4cApiEnv *penv );
int DC4CGetUnusedWorkersCount( struct Dc4cApiEnv *penv );

/********* 计算节点接口 *********/

void DC4CSetAppLogFile( char *program );

int DC4CFormatReplyInfo( char *format , ... );
int DC4CSetReplyInfo( char *str );
int DC4CSetReplyInfoEx( char *buf , int len );

/********* 公共接口 *********/

#define SetAttribute(_param_,_attribute_)	(_param_)=(_attribute_);
#define AddAttribute(_param_,_attribute_)	(_param_)|=(_attribute_);
#define RemoveAttribute(_param_,_attribute_)	(_param_)&=(_attribute_);
#define TestAttribute(_param_,_attribute_)	( ((_param_)&(_attribute_)) == (_attribute_) ? 1 : 0 )

#define GENERAL_TIMEFORMAT	
char *ConvertTimeString( time_t tt , char *buf , size_t bufsize );
char *GetTimeStringNow( char *buf , size_t bufsize );

#ifdef __cplusplus
}
#endif

#endif

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

extern char *__DC4C_API_VERSION ;

#define DC4C_ERROR_INTERNAL			-11
#define DC4C_ERROR_SOCKET			-12
#define DC4C_ERROR_CONNECT			-13
#define DC4C_ERROR_ALLOC			-14
#define DC4C_ERROR_FILE_NOT_FOUND		-21
#define DC4C_ERROR_PARAMETER			-22
#define DC4C_ERROR_NO_WORKER			-23
#define DC4C_ERROR_DATABASE			-24
#define DC4C_INFO_NO_PREPARE			31
#define DC4C_INFO_NO_RUNNING			32
#define DC4C_INFO_NO_PREPARE_AND_RUNNING	35
#define DC4C_INFO_NO_UNFINISHED_ENVS		38
#define DC4C_INFO_ALREADY_EXECUTING		60
#define DC4C_ERROR_OPENFILE			-70
#define DC4C_ERROR_WRITEFILE			-71
#define DC4C_ERROR_CREATEPIPE			-72
#define DC4C_ERROR_FORK				-73
#define DC4C_ERROR_TIMEOUT			-74
#define DC4C_ERROR_MD5_NOT_MATCHED_TOO		-75
#define DC4C_ERROR_TERMSIG			-81
#define DC4C_ERROR_SIGNALED			-82
#define DC4C_ERROR_STOPPED			-83
#define DC4C_ERROR_APP				-84
#define DC4C_ERROR_UNKNOW_QUIT			-89

#define DC4C_WORKER_COUNT_UNLIMITED		0

#define DC4C_OPTIONS_INTERRUPT_BY_APP		1

struct Dc4cApiEnv ;

int DC4CInitEnv( struct Dc4cApiEnv **ppenv , char *rservers_ip_port );
int DC4CCleanEnv( struct Dc4cApiEnv **ppenv );

void DC4CSetTimeout( struct Dc4cApiEnv *penv , int timeout );
int DC4CGetTimeout( struct Dc4cApiEnv *penv );

void DC4CSetOptions( struct Dc4cApiEnv *penv , unsigned long options );
unsigned long DC4CGetOptions( struct Dc4cApiEnv *penv );

int DC4CDoTask( struct Dc4cApiEnv *penv , char *program_and_params , int timeout );

int DC4CGetTaskIp( struct Dc4cApiEnv *penv , char **pp_ip );
int DC4CGetTaskPort( struct Dc4cApiEnv *penv , long *p_port );
int DC4CGetTaskTid( struct Dc4cApiEnv *penv , char **pp_tid );
int DC4CGetTaskProgramAndParam( struct Dc4cApiEnv *penv , char **pp_program_and_params );
int DC4CGetTaskTimeout( struct Dc4cApiEnv *penv , int *p_timeout );
int DC4CGetTaskElapse( struct Dc4cApiEnv *penv , int *p_elapse );
int DC4CGetTaskError( struct Dc4cApiEnv *penv , int *p_error );
int DC4CGetTaskStatus( struct Dc4cApiEnv *penv , int *p_status );
int DC4CGetTaskInfo( struct Dc4cApiEnv *penv , char **pp_info );

struct Dc4cBatchTask
{
	char	program_and_params[ 256 + 1 ] ;
	int	timeout ;
} ;

int DC4CDoBatchTasks( struct Dc4cApiEnv *penv , int workers_count , struct Dc4cBatchTask *p_tasks , int tasks_count );

int DC4CPerformMultiBatchTasks( struct Dc4cApiEnv **ppenvs , int envs_count , struct Dc4cApiEnv **ppenv , int *p_remain_envs_count );

int DC4CBeginBatchTasks( struct Dc4cApiEnv *penv , int workers_count , struct Dc4cBatchTask *p_tasks , int tasks_count );
int DC4CQueryWorkers( struct Dc4cApiEnv *penv );
int DC4CSetBatchTasksFds( struct Dc4cApiEnv *penv , fd_set *read_fds , int *p_max_fd );
int DC4CSelectBatchTasksFds( fd_set *p_read_fds , int *p_max_fd , int select_timeout );
int DC4CProcessBatchTasks( struct Dc4cApiEnv *penv , fd_set *p_read_fds , int *p_max_fd );

int DC4CGetTasksCount( struct Dc4cApiEnv *penv );
int DC4CGetWorkersCount( struct Dc4cApiEnv *penv );
int DC4CGetPrepareTasksCount( struct Dc4cApiEnv *penv );
int DC4CGetRunningTasksCount( struct Dc4cApiEnv *penv );
int DC4CGetFinishedTasksCount( struct Dc4cApiEnv *penv );

/* index based 1 */
int DC4CGetBatchTasksIp( struct Dc4cApiEnv *penv , int index , char **pp_ip );
int DC4CGetBatchTasksPort( struct Dc4cApiEnv *penv , int index , long *p_port );
int DC4CGetBatchTasksTid( struct Dc4cApiEnv *penv , int index , char **pp_tid );
int DC4CGetBatchTasksProgramAndParam( struct Dc4cApiEnv *penv , int index , char **pp_program_and_params );
int DC4CGetBatchTasksTimeout( struct Dc4cApiEnv *penv , int index , int *p_timeout );
int DC4CGetBatchTasksElapse( struct Dc4cApiEnv *penv , int index , int *p_elapse );
int DC4CGetBatchTasksError( struct Dc4cApiEnv *penv , int index , int *p_error );
int DC4CGetBatchTasksStatus( struct Dc4cApiEnv *penv , int index , int *p_status );
int DC4CGetBatchTasksInfo( struct Dc4cApiEnv *penv , int index , char **pp_info );

void DC4CSetAppLogFile( char *program );

int DC4CFormatReplyInfo( char *format , ... );
int DC4CSetReplyInfo( char *str );
int DC4CSetReplyInfoEx( char *buf , int len );

int DC4CGetUnusedWorkersCount( struct Dc4cApiEnv *penv );

#ifdef __cplusplus
}
#endif

#endif

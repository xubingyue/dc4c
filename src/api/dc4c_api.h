/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 * LastVersion	: v1.0.0
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

extern char *__DC4C_API_VERSION ;

#define DC4C_ERROR_INTERNAL			-11
#define DC4C_ERROR_SOCKET			-12
#define DC4C_ERROR_CONNECT			-13
#define DC4C_ERROR_ALLOC			-14
#define DC4C_ERROR_FILE_NOT_EXIST		-21
#define DC4C_ERROR_PARAMETER			-22
#define DC4C_ERROR_NO_WORKER			-23
#define DC4C_INFO_NO_RUNNING			31
#define DC4C_INFO_NO_PREPARE_AND_RUNNING	32

#define DC4C_RETURNSTATUS_ALREADY_EXECUTING	100
#define DC4C_RETURNSTATUS_OPENFILE		110
#define DC4C_RETURNSTATUS_WRITEFILE		111
#define DC4C_RETURNSTATUS_CREATEPIPE		112
#define DC4C_RETURNSTATUS_FORK			114
#define DC4C_RETURNSTATUS_TIMEOUT		121

#define DC4C_WORKER_COUNT_UNLIMITED		0

struct Dc4cApiEnv ;

int DC4CInitEnv( struct Dc4cApiEnv **ppenv , char *rserver_ip_port );
int DC4CCleanEnv( struct Dc4cApiEnv **ppenv );

void DC4CSetTimeout( struct Dc4cApiEnv *penv , long timeout );

int DC4CDoTask( struct Dc4cApiEnv *penv , char *program_and_params );

int DC4CGetTaskProgramAndParam( struct Dc4cApiEnv *penv , char **pp_program_and_params );
int DC4CGetTaskTid( struct Dc4cApiEnv *penv , char **pp_tid );
int DC4CGetTaskIp( struct Dc4cApiEnv *penv , char **pp_ip );
int DC4CGetTaskPort( struct Dc4cApiEnv *penv , long *p_port );
int DC4CGetTaskResponseCode( struct Dc4cApiEnv *penv , int *p_response_code );
int DC4CGetTaskStatus( struct Dc4cApiEnv *penv , int *p_status );

int DC4CDoBatchTasks( struct Dc4cApiEnv *penv , int worker_count , char **program_and_params_array , int program_and_params_count );

int DC4CBeginBatchTasks( struct Dc4cApiEnv *penv , int worker_count , char **program_and_params_array , int program_and_params_count );
int DC4CSetBatchTasksFds( struct Dc4cApiEnv *penv , fd_set *read_fds , fd_set *write_fds , fd_set *expect_fds , int *p_max_fd );
int DC4CPerformBatchTasks( struct Dc4cApiEnv *penv );

int DC4CGetPrepareTaskCount( struct Dc4cApiEnv *penv );
int DC4CGetRunningTaskCount( struct Dc4cApiEnv *penv );
int DC4CGetFinishedTaskCount( struct Dc4cApiEnv *penv );

int DC4CGetBatchTasksProgramAndParam( struct Dc4cApiEnv *penv , int index , char **pp_program_and_params ); /* index based 1 */
int DC4CGetBatchTasksTid( struct Dc4cApiEnv *penv , int index , char **pp_tid );
int DC4CGetBatchTasksIp( struct Dc4cApiEnv *penv , int index , char **pp_ip );
int DC4CGetBatchTasksPort( struct Dc4cApiEnv *penv , int index , long *p_port );
int DC4CGetBatchTasksResponseCode( struct Dc4cApiEnv *penv , int index , int *p_response_code );
int DC4CGetBatchTasksStatus( struct Dc4cApiEnv *penv , int index , int *p_status );

void DC4CSetAppLogFile();

#endif

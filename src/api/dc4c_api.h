#ifndef _H_DC4C_API_
#define _H_DC4C_API_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define DC4C_ERROR_SOCKET		-11
#define DC4C_ERROR_CONNECT		-12
#define DC4C_ERROR_ALLOC		-13
#define DC4C_ERROR_FILE_NOT_EXIST	21
#define DC4C_ERROR_PARAMETER		-31

#define MAXCNT_PROGRAM_AND_PARAMS_ARRAY	1024

struct Dc4cApiEnv ;

int DC4CInitEnv( struct Dc4cApiEnv **ppenv , char *rserver_ip , int rserver_port );
int DC4CCleanEnv( struct Dc4cApiEnv **ppenv );

void DC4CSetTimeout( struct Dc4cApiEnv *penv , long timeout );

int DC4CDoTask( struct Dc4cApiEnv *penv , char *program_and_params );
int DC4CGetTaskResponseStatus( struct Dc4cApiEnv *penv , int *status );

int DC4CDoBatchTasks( struct Dc4cApiEnv *penv , int worker_count , char **program_and_params_array , int program_and_params_count );
int DC4CDoBatchTasksV( struct Dc4cApiEnv *penv , int worker_count , ... );
int DC4CGetBatchTasksResponseStatus( struct Dc4cApiEnv *penv , int index , int *status ); /* index based 1 */

#endif

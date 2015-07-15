/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "IDL_query_workers_request.dsc.h"
#include "IDL_query_workers_response.dsc.h"
#include "IDL_execute_program_request.dsc.h"
#include "IDL_execute_program_response.dsc.h"
#include "IDL_deploy_program_request.dsc.h"

#include "dc4c_util.h"
#include "dc4c_api.h"

#define RSERVER_ARRAYSIZE				8

#define WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING	0
#define WSERVER_SESSION_PROGRESS_EXECUTING		1
#define WSERVER_SESSION_PROGRESS_FINISHED		2
#define WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR	4

#define SELECT_TIMEOUT					10

struct Dc4cApiEnv
{
	char				rserver_ip[ RSERVER_ARRAYSIZE ][ 40 + 1 ] ;
	int				rserver_port[ RSERVER_ARRAYSIZE ] ;
	int				rserver_count ;
	int				rserver_index ;
	struct SocketSession		rserver_session ;
	
	int				tasks_count ;
	int				workers_count ;
	
	struct SocketSession		*tasks_session_array ;
	execute_program_request		*tasks_request_array ;
	execute_program_response	*tasks_response_array ;
	int				tasks_array_size ;
	
	int				timeout ;
	unsigned long			options ;
	
	int				prepare_count ;
	int				running_count ;
	int				finished_count ;
	
	void				*p1 ;
	void				*p2 ;
	funcDC4COnBeginTaskProc		*pfuncDC4COnBeginTaskProc ;
	funcDC4COnCancelTaskProc	*pfuncDC4COnCancelTaskProc ;
	funcDC4COnFinishTaskProc	*pfuncDC4COnFinishTaskProc ;
	
	query_workers_response		qwp ;
	int				query_count_used ;
	
	int				interrupt_code ;
	int				finished_flag ;
} ;

#define PREFIX_DSCLOG_query_workers_request	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_query_workers_request
#include "IDL_query_workers_request.dsc.LOG.c"

#define PREFIX_DSCLOG_query_workers_response	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_query_workers_response
#include "IDL_query_workers_response.dsc.LOG.c"

#define PREFIX_DSCLOG_execute_program_request	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_execute_program_request
#include "IDL_execute_program_request.dsc.LOG.c"

#define PREFIX_DSCLOG_execute_program_response	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_execute_program_response
#include "IDL_execute_program_response.dsc.LOG.c"

#define PREFIX_DSCLOG_deploy_program_request	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_deploy_program_request
#include "IDL_deploy_program_request.dsc.LOG.c"

static int proto_QueryWorkersRequest( struct SocketSession *psession , int want_count )
{
	struct utsname		uts ;
	
	int			msg_len ;
	
	int			nret = 0 ;
	
	query_workers_request	req ;
	
	CleanSendBuffer( psession );
	
	DSCINIT_query_workers_request( & req );
	
	nret = uname( & uts ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "uname failed[%d]" , nret );
		return -1;
	}
	strcpy( req.sysname , uts.sysname );
	strcpy( req.release , uts.release );
	req.bits = sizeof(long) * 8 ;
	req.count = want_count ;
	
	DSCLOG_query_workers_request( & req );
	
	msg_len = psession->send_buffer_size-1 - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
	nret = DSCSERIALIZE_JSON_COMPACT_query_workers_request( & req , NULL , psession->send_buffer + LEN_COMMHEAD + LEN_MSGHEAD , & msg_len ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_query_workers_request failed[%d]" , nret );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_query_workers_request ok , [%d]bytes" , msg_len );
	}
	
	FormatSendHead( psession , "QWQ" , msg_len );
	
	InfoLog( __FILE__ , __LINE__ , "output buffer [%d]bytes[%.*s]" , psession->total_send_len - LEN_COMMHEAD , psession->total_send_len - LEN_COMMHEAD , psession->send_buffer + LEN_COMMHEAD );
	
	return 0;
}

static int proto_QueryWorkersResponse( struct SocketSession *psession , query_workers_response *p_rsp )
{
	int				msg_len ;
	
	int				nret = 0 ;
	
	InfoLog( __FILE__ , __LINE__ , "input buffer [%d]bytes[%.*s]" , psession->recv_body_len , psession->recv_body_len , psession->recv_buffer + LEN_COMMHEAD );
	
	DSCINIT_query_workers_response( p_rsp );
	
	msg_len = psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
	nret = DSCDESERIALIZE_JSON_COMPACT_query_workers_response( NULL , psession->recv_buffer + LEN_COMMHEAD + LEN_MSGHEAD_MSGTYPE , & msg_len , p_rsp ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_query_workers_response failed[%d]" , nret );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_query_workers_response ok" );
	}
	
	DSCLOG_query_workers_response( p_rsp );
	
	return 0;
}

static int proto_ExecuteProgramRequest( struct SocketSession *psession , int order_index , char *program_and_params , int timeout , int options , execute_program_request *p_req )
{
	char			program[ MAXLEN_FILENAME + 1 ] ;
	char			pathfilename[ MAXLEN_FILENAME + 1 ] ;
	struct timeval		tv ;
	time_t			tt ;
	
	int			msg_len ;
	
	int			nret = 0 ;
	
	execute_program_request	req ;
	
	CleanSendBuffer( psession );
	
	DSCINIT_execute_program_request( & req );
	
	strcpy( req.ip , psession->ip );
	req.port = psession->port ;
	memset( & tv , 0x00 , sizeof(struct timeval) );
	gettimeofday( & tv , NULL );
	SNPRINTF( req.tid , sizeof(req.tid)-1 , "%010d%010d" , (int)(tv.tv_sec) , (int)(tv.tv_usec) );
	req.order_index = order_index ;
	strcpy( req.program_and_params , program_and_params );
	req.timeout = timeout ;
	time( & tt );
	req.begin_datetime_stamp = tt ;
	req.bind_cpu_flag = TestAttribute( options , DC4C_OPTIONS_BIND_CPU )?1:0 ;
	
	memset( program , 0x00 , sizeof(program) );
	sscanf( program_and_params , "%s" , program );
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	SNPRINTF( pathfilename , sizeof(pathfilename)-1 , "%s/bin/%s" , getenv("HOME") , program );
	memset( req.program_md5_exp , 0x00 , sizeof(req.program_md5_exp) );
	nret = FileMd5( pathfilename , req.program_md5_exp ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "FileMd5[%s] failed[%d]" , pathfilename , nret );
		return DC4C_ERROR_FILE_NOT_FOUND;
	}
	
	DSCLOG_execute_program_request( & req );
	
	msg_len = psession->send_buffer_size-1 - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
	nret = DSCSERIALIZE_JSON_COMPACT_execute_program_request( & req , NULL , psession->send_buffer + LEN_COMMHEAD + LEN_MSGHEAD , & msg_len ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_execute_program_request failed[%d]" , nret );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_execute_program_request ok , [%d]bytes" , msg_len );
	}
	
	FormatSendHead( psession , "EPQ" , msg_len );
	
	InfoLog( __FILE__ , __LINE__ , "output buffer [%d]bytes[%.*s]" , psession->total_send_len - LEN_COMMHEAD , psession->total_send_len - LEN_COMMHEAD , psession->send_buffer + LEN_COMMHEAD );
	
	if( p_req )
	{
		memcpy( p_req , & req , sizeof(execute_program_request) );
	}
	
	return 0;
}

static int proto_DeployProgramRequest( struct SocketSession *psession , deploy_program_request *p_rsp )
{
	int				msg_len ;
	
	int				nret = 0 ;
	
	InfoLog( __FILE__ , __LINE__ , "input buffer [%d]bytes[%.*s]" , psession->recv_body_len , psession->recv_body_len , psession->recv_buffer + LEN_COMMHEAD );
	
	DSCINIT_deploy_program_request( p_rsp );
	
	msg_len = psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
	nret = DSCDESERIALIZE_JSON_COMPACT_deploy_program_request( NULL , psession->recv_buffer + LEN_COMMHEAD + LEN_MSGHEAD_MSGTYPE , & msg_len , p_rsp ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_deploy_program_request failed[%d]" , nret );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_deploy_program_request ok" );
	}
	
	DSCLOG_deploy_program_request( p_rsp );
	
	return 0;
}

static int proto_DeployProgramResponse( struct SocketSession *psession , char *program )
{
	char			pathfilename[ MAXLEN_FILENAME + 1 ] ;
	FILE			*fp = NULL ;
	long			filesize ;
	
	int			nret = 0 ;
	
	CleanSendBuffer( psession );
	
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	SNPRINTF( pathfilename , sizeof(pathfilename)-1 , "%s/bin/%s" , getenv("HOME") , program );
	fp = fopen( pathfilename , "rb" ) ;
	if( fp == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "fopen[%s] failed , errno[%d]" , pathfilename , errno );
		return -1;
	}
	
	fseek( fp , 0L , SEEK_END );
	filesize = ftell( fp ) ;
	fseek( fp , 0L , SEEK_SET );
	
	if( LEN_COMMHEAD + LEN_MSGHEAD + filesize > psession->send_buffer_size-1 )
	{
		nret = ReallocSendBuffer( psession , LEN_COMMHEAD + LEN_MSGHEAD + filesize + 1 ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "ReallocSendBuffer ->[%d]bytes failed[%ld] , errno[%d]" , LEN_COMMHEAD + LEN_MSGHEAD + filesize + 1 , nret , errno );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "ReallocSendBuffer ->[%d]bytes ok" , psession->send_buffer_size );
		}
	}
	
	fread( psession->send_buffer + LEN_COMMHEAD + LEN_MSGHEAD , sizeof(char) , filesize , fp );
	fclose( fp );
	
	FormatSendHead( psession , "DPP" , filesize );
	
	InfoLog( __FILE__ , __LINE__ , "output buffer [%d]bytes[%.16s]" , psession->total_send_len - LEN_COMMHEAD , psession->send_buffer + LEN_COMMHEAD );
	
	return 0;
}

static int proto_ExecuteProgramResponse( struct SocketSession *psession , execute_program_response *p_rsp )
{
	int				msg_len ;
	
	int				nret = 0 ;
	
	InfoLog( __FILE__ , __LINE__ , "input buffer [%d]bytes[%.*s]" , psession->recv_body_len , psession->recv_body_len , psession->recv_buffer + LEN_COMMHEAD );
	
	DSCINIT_execute_program_response( p_rsp );
	
	msg_len = psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
	nret = DSCDESERIALIZE_JSON_COMPACT_execute_program_response( NULL , psession->recv_buffer + LEN_COMMHEAD + LEN_MSGHEAD_MSGTYPE , & msg_len , p_rsp ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_execute_program_response failed[%d]" , nret );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_execute_program_response ok" );
	}
	
	DSCLOG_execute_program_response( p_rsp );
	
	return 0;
}

static int ReformingTasksArray( struct Dc4cApiEnv *penv , int tasks_count )
{
	int			i ;
	
	int			nret = 0 ;
	
	if( tasks_count <= penv->tasks_array_size )
	{
		for( i = 0 ; i < tasks_count ; i++ )
		{
			DSCINIT_execute_program_request( & (penv->tasks_request_array[i]) );
		}
		
		for( i = 0 ; i < tasks_count ; i++ )
		{
			DSCINIT_execute_program_response( & (penv->tasks_response_array[i]) );
		}
		
		for( i = 0 ; i < tasks_count ; i++ )
		{
			CloseSocket( & (penv->tasks_session_array[i]) );
		}
	}
	else
	{
		struct SocketSession		*tasks_session_array ;
		execute_program_request		*tasks_request_array ;
		execute_program_response	*tasks_response_array ;
		
		tasks_request_array = (execute_program_request *)realloc( penv->tasks_request_array , sizeof(execute_program_request) * tasks_count ) ;
		if( tasks_request_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			return DC4C_ERROR_ALLOC;
		}
		for( i = 0 ; i < tasks_count ; i++ )
		{
			DSCINIT_execute_program_request( & (tasks_request_array[i]) );
		}
		
		tasks_response_array = (execute_program_response *)realloc( penv->tasks_response_array , sizeof(execute_program_response) * tasks_count ) ;
		if( tasks_response_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			return DC4C_ERROR_ALLOC;
		}
		for( i = 0 ; i < tasks_count ; i++ )
		{
			DSCINIT_execute_program_response( & (tasks_response_array[i]) );
		}
		
		tasks_session_array = (struct SocketSession *)realloc( penv->tasks_session_array , sizeof(struct SocketSession) * tasks_count ) ;
		if( tasks_session_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			free( tasks_request_array );
			return DC4C_ERROR_ALLOC;
		}
		
		for( i = 0 ; i < tasks_count ; i++ )
		{
			nret = InitSocketSession( & (tasks_session_array[i]) ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
				for( --i ; i >= 0 ; i-- )
				{
					CleanSocketSession( & (tasks_session_array[i]) );
				}
				free( tasks_request_array );
				free( tasks_response_array );
				return DC4C_ERROR_ALLOC;
			}
		}
		
		penv->tasks_request_array = tasks_request_array ;
		penv->tasks_response_array = tasks_response_array ;
		penv->tasks_session_array = tasks_session_array ;
		
		penv->tasks_array_size = tasks_count ;
	}
	
	return 0;
}

void DC4CCleanEnv( struct Dc4cApiEnv **ppenv )
{
	int		i ;
	
	if( (*ppenv) )
	{
		CloseSocket( & ((*ppenv)->rserver_session) );
		CleanSocketSession( & ((*ppenv)->rserver_session) );
		
		if( (*ppenv)->tasks_request_array )
		{
			free( (*ppenv)->tasks_request_array );
		}
		
		if( (*ppenv)->tasks_response_array )
		{
			free( (*ppenv)->tasks_response_array );
		}
		
		if( (*ppenv)->tasks_session_array )
		{
			for( i = 0 ; i < (*ppenv)->tasks_array_size ; i++ )
			{
				if( IsSocketEstablished( & ((*ppenv)->tasks_session_array[i]) ) )
				{
					CloseSocket( & ((*ppenv)->tasks_session_array[i]) );
				}
				CleanSocketSession( & ((*ppenv)->tasks_session_array[i]) );
			}
			
			free( (*ppenv)->tasks_session_array );
		}
		
		free( (*ppenv) );
		(*ppenv) = NULL ;
	}
	
	return;
}

int DC4CInitEnv( struct Dc4cApiEnv **ppenv , char *rservers_ip_port )
{
	char		rservers_ip_port_copy[ 1024 + 1 ] ;
	char		*p = NULL ;
	
	int		nret = 0 ;
	
	srand( (unsigned)time( NULL ) );
	
	(*ppenv) = (struct Dc4cApiEnv *)malloc( sizeof(struct Dc4cApiEnv) ) ;
	if( (*ppenv) == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return DC4C_ERROR_ALLOC;
	}
	memset( (*ppenv) , 0x00 , sizeof(struct Dc4cApiEnv) );
	
	if( rservers_ip_port && rservers_ip_port[0] )
	{
		memset( rservers_ip_port_copy , 0x00 , sizeof(rservers_ip_port_copy) );
		strncpy( rservers_ip_port_copy , rservers_ip_port , sizeof(rservers_ip_port_copy)-1 );
	}
	else if( ( p = getenv("DC4C_RSERVERS_IP_PORT") ) )
	{
		memset( rservers_ip_port_copy , 0x00 , sizeof(rservers_ip_port_copy) );
		strncpy( rservers_ip_port_copy , p , sizeof(rservers_ip_port_copy)-1 );
	}
	else
	{
		return DC4C_ERROR_PARAMETER;
	}
	
	p = strtok( rservers_ip_port_copy , "," ) ;
	if( p == NULL )
		p = rservers_ip_port_copy ;
	for( (*ppenv)->rserver_count = 0 ; (*ppenv)->rserver_count < RSERVER_ARRAYSIZE && p ; (*ppenv)->rserver_count++ )
	{
		sscanf( p , "%[^:]:%d" , (*ppenv)->rserver_ip[(*ppenv)->rserver_count] , & ((*ppenv)->rserver_port[(*ppenv)->rserver_count]) );
		p = strtok( NULL , "," ) ;
	}
	(*ppenv)->rserver_index = 0 ;
	
	nret = InitSocketSession( & ((*ppenv)->rserver_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_ALLOC;
	}
	
	(*ppenv)->timeout = DC4C_DEFAULT_TIMEOUT ;
	
	DSCINIT_query_workers_response( & ((*ppenv)->qwp) );
	(*ppenv)->query_count_used = 0 ;
	
	return 0;
}

void DC4CSetTimeout( struct Dc4cApiEnv *penv , int timeout )
{
	penv->timeout = timeout ;
	return;
}

int DC4CGetTimeout( struct Dc4cApiEnv *penv )
{
	return penv->timeout;
}

void DC4CSetOptions( struct Dc4cApiEnv *penv , unsigned long options )
{
	penv->options = options ;
	return;
}

unsigned long DC4CGetOptions( struct Dc4cApiEnv *penv )
{
	return penv->options;
}

int DC4CDoTask( struct Dc4cApiEnv *penv , char *program_and_params , int timeout , char *ip )
{
	struct Dc4cBatchTask	task ;
	
	memset( & task , 0x00 , sizeof(struct Dc4cBatchTask) );
	if( program_and_params == NULL || program_and_params[0] == '\0' )
		return 0;
	strncpy( task.program_and_params , program_and_params , sizeof(task.program_and_params)-1 );
	task.timeout = timeout ;
	task.order_index = 1 ;
	if( ip )
		strncpy( task.ip , ip , sizeof(task.ip)-1 );
	
	return DC4CDoBatchTasks( penv , 1 , & task , 1 );
}

int DC4CGetTaskProgress( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksProgress( penv , 0 );
}

char *DC4CGetTaskIp( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksIp( penv , 0 );
}

int DC4CGetTaskPort( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksPort( penv , 0 );
}

char *DC4CGetTaskTid( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksTid( penv , 0 );
}

int DC4CGetTaskOrderIndex( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksOrderIndex( penv , 0 );
}

char *DC4CGetTaskProgramAndParams( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksProgramAndParams( penv , 0 );
}

int DC4CGetTaskTimeout( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksTimeout( penv , 0 );
}

int DC4CGetTaskBeginTimestamp( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksBeginTimestamp( penv , 0 );
}

int DC4CGetTaskEndTimestamp( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksEndTimestamp( penv , 0 );
}

int DC4CGetTaskElapse( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksElapse( penv , 0 );
}

int DC4CGetTaskError( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksError( penv , 0 );
}

int DC4CGetTaskStatus( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksStatus( penv , 0 );
}

char *DC4CGetTaskInfo( struct Dc4cApiEnv *penv )
{
	return DC4CGetBatchTasksInfo( penv , 0 );
}

#define PREPARE_COUNT_INCREASE															\
	InfoLog( __FILE__ , __LINE__ , "[%p] COUNT prepare_count[%d]->[%d]" , penv , penv->prepare_count , penv->prepare_count+1 );	\
	penv->prepare_count++;															\

#define PREPARE_COUNT_DECREASE															\
	InfoLog( __FILE__ , __LINE__ , "[%p] COUNT prepare_count[%d]->[%d]" , penv , penv->prepare_count , penv->prepare_count-1 );	\
	penv->prepare_count--;															\

#define RUNNING_COUNT_INCREASE															\
	InfoLog( __FILE__ , __LINE__ , "[%p] COUNT running_count[%d]->[%d]" , penv , penv->running_count , penv->running_count+1 );	\
	penv->running_count++;															\

#define RUNNING_COUNT_DECREASE															\
	InfoLog( __FILE__ , __LINE__ , "[%p] COUNT running_count[%d]->[%d]" , penv , penv->running_count , penv->running_count-1 );	\
	penv->running_count--;															\

#define FINISHED_COUNT_INCREASE															\
	InfoLog( __FILE__ , __LINE__ , "[%p] COUNT finished_count[%d]->[%d]" , penv , penv->finished_count , penv->finished_count+1 );	\
	penv->finished_count++;															\

#define FINISHED_COUNT_DECREASE															\
	InfoLog( __FILE__ , __LINE__ , "[%p] COUNT finished_count[%d]->[%d]" , penv , penv->finished_count , penv->finished_count-1 );	\
	penv->finished_count--;															\

int DC4CDoBatchTasks( struct Dc4cApiEnv *penv , int workers_count , struct Dc4cBatchTask *a_tasks , int tasks_count )
{
	int		nret = 0 ;
	
	nret = DC4CBeginBatchTasks( penv , workers_count , a_tasks , tasks_count ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CBeginBatchTasks failed[%d]" , nret );
		return nret;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DC4CBeginBatchTasks ok" );
	}
	
	while(1)
	{
		nret = DC4CPerformBatchTasks( penv , NULL ) ;
		if( nret == DC4C_INFO_TASK_FINISHED )
		{
			InfoLog( __FILE__ , __LINE__ , "DC4CPerformBatchTasks return DC4C_INFO_TASK_FINISHED" );
		}
		else if( nret == DC4C_INFO_BATCH_TASKS_FINISHED )
		{
			InfoLog( __FILE__ , __LINE__ , "DC4CPerformBatchTasks return DC4C_INFO_BATCH_TASKS_FINISHED" );
			if( penv->interrupt_code )
				return penv->interrupt_code;
			else
				return 0;
		}
		else if( nret == DC4C_ERROR_TIMEOUT )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CPerformBatchTasks return DC4C_ERROR_TIMEOUT" );
		}
		else if( nret == DC4C_ERROR_APP )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CPerformBatchTasks return DC4C_ERROR_APP" );
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CPerformBatchTasks failed[%d]" , nret );
			return nret;
		}
	}
}

int DC4CBatchTasksInterruptCode( struct Dc4cApiEnv *penv )
{
	return penv->interrupt_code;
}

int DC4CGetBatchTasksProgress( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return 0;
	
	return penv->tasks_session_array[task_index].progress;
}

char *DC4CGetBatchTasksIp( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return NULL;
	
	return penv->tasks_request_array[task_index].ip;
}

int DC4CGetBatchTasksPort( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return 0;
	
	return penv->tasks_request_array[task_index].port;
}

char *DC4CGetBatchTasksTid( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return NULL;
	
	return penv->tasks_request_array[task_index].tid;
}

int DC4CGetBatchTasksOrderIndex( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return 0;
	
	return penv->tasks_request_array[task_index].order_index;
}

char *DC4CGetBatchTasksProgramAndParams( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return NULL;
	
	return penv->tasks_request_array[task_index].program_and_params;
}

int DC4CGetBatchTasksTimeout( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return 0;
	
	return penv->tasks_request_array[task_index].timeout;
}

int DC4CGetBatchTasksBeginTimestamp( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return 0;
	
	return penv->tasks_request_array[task_index].begin_datetime_stamp;
}

int DC4CGetBatchTasksEndTimestamp( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return 0;
	
	return penv->tasks_response_array[task_index].end_datetime_stamp;
}

int DC4CGetBatchTasksElapse( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return 0;
	
	return penv->tasks_response_array[task_index].elapse;
}

int DC4CGetBatchTasksError( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return 0;
	
	return penv->tasks_response_array[task_index].error;
}

int DC4CGetBatchTasksStatus( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return 0;
	
	return penv->tasks_response_array[task_index].status;
}

char *DC4CGetBatchTasksInfo( struct Dc4cApiEnv *penv , int task_index )
{
	if( task_index > penv->tasks_array_size )
		return NULL;
	
	return penv->tasks_response_array[task_index].info;
}

int DC4CDoMultiBatchTasks( struct Dc4cApiEnv **a_penv , int envs_count , int *a_workers_count , struct Dc4cBatchTask **aa_tasks , int *a_tasks_count )
{
	int			envs_index ;
	
	int			nret = 0 ;
	
	for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
	{
		nret = DC4CBeginBatchTasks( a_penv[envs_index] , a_workers_count[envs_index] , aa_tasks[envs_index] , a_tasks_count[envs_index] ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CBeginBatchTasks failed[%d]" , nret );
			return nret;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DC4CBeginBatchTasks ok" );
		}
	}
	
	while(1)
	{
		nret = DC4CPerformMultiBatchTasks( a_penv , envs_count , NULL , NULL ) ;
		if( nret == DC4C_INFO_TASK_FINISHED )
		{
			InfoLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks return DC4C_INFO_TASK_FINISHED" );
		}
		else if( nret == DC4C_INFO_BATCH_TASKS_FINISHED )
		{
			InfoLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks return DC4C_INFO_BATCH_TASKS_FINISHED" );
		}
		else if( nret == DC4C_INFO_ALL_ENVS_FINISHED )
		{
			InfoLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks return DC4C_INFO_ALL_ENVS_FINISHED" );
			for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
			{
				if( a_penv[envs_index]->interrupt_code )
					return a_penv[envs_index]->interrupt_code;
			}
			return 0;
		}
		else if( nret == DC4C_ERROR_TIMEOUT )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks return DC4C_ERROR_TIMEOUT" );
		}
		else if( nret == DC4C_ERROR_APP )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks return DC4C_ERROR_APP" );
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks failed[%d]" , nret );
			return nret;
		}
	}
}

int DC4CMultiBatchTasksInterruptCode( struct Dc4cApiEnv **a_penv , int envs_count )
{
	int			envs_index ;
	
	for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
	{
		if( a_penv[envs_index]->interrupt_code )
			return a_penv[envs_index]->interrupt_code;
	}
	
	return 0;
}

int DC4CBeginBatchTasks( struct Dc4cApiEnv *penv , int workers_count , struct Dc4cBatchTask *a_tasks , int tasks_count )
{
	struct Dc4cBatchTask		*p_task = NULL ;
	int				i ;
	
	int				nret = 0 ;
	
	InfoLog( __FILE__ , __LINE__ , "workers_count[%d] tasks_count[%d]" , workers_count , tasks_count );
	
	if( tasks_count == 0 )
		return 0;
	else if( workers_count < 0 || tasks_count < 0 )
		return DC4C_ERROR_PARAMETER;
	
	penv->tasks_count = tasks_count ;
	
	nret = ReformingTasksArray( penv , penv->tasks_count ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "ReformingTasksArray failed[%d] , errno[%d]" , nret , errno );
		return nret;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "ReformingTasksArray ok" );
	}
	
	for( i = 0 , p_task = a_tasks ; i < penv->tasks_count ; i++ , p_task++ )
	{
		strncpy( penv->tasks_request_array[i].program_and_params , p_task->program_and_params , sizeof(penv->tasks_request_array[i].program_and_params)-1 );
		penv->tasks_request_array[i].timeout = p_task->timeout ;
		if( penv->tasks_request_array[i].timeout == 0 )
		{
			penv->tasks_request_array[i].timeout = penv->timeout ;
		}
		penv->tasks_request_array[i].order_index = p_task->order_index ;
		InfoLog( __FILE__ , __LINE__ , "TASK[%d] [%s] [%d] [%d]" , i , penv->tasks_request_array[i].program_and_params , penv->tasks_request_array[i].timeout , penv->tasks_request_array[i].order_index );
	}
	
	if( workers_count == DC4C_WORKER_COUNT_UNLIMITED || workers_count > penv->tasks_count )
	{
		penv->workers_count = penv->tasks_count ;
	}
	else
	{
		penv->workers_count = workers_count ;
	}
	
	penv->prepare_count = penv->tasks_count ;
	penv->running_count = 0 ;
	penv->finished_count = 0 ;
	
	return 0;
}

int DC4CPerformBatchTasks( struct Dc4cApiEnv *penv , int *p_task_index )
{
	fd_set		read_fds , write_fds , expect_fds ;
	int		max_fd ;
	int		select_return_count = 0 ;
	int		task_index ;
	
	int		nret = 0 ;
	
	if( DC4CGetPrepareTasksCount(penv) == 0 && DC4CGetRunningTasksCount(penv) == 0 )
		return DC4C_INFO_BATCH_TASKS_FINISHED;
	if( penv->interrupt_code && DC4CGetRunningTasksCount(penv) == 0 )
		return DC4C_INFO_BATCH_TASKS_FINISHED;
	
	while(1)
	{
		if( DC4CGetRunningTasksCount(penv) < DC4CGetWorkersCount(penv) && DC4CGetPrepareTasksCount(penv) > 0 )
		{
			nret = DC4CQueryWorkers( penv ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "DC4CQueryWorkers failed[%d]" , nret );
				penv->interrupt_code = nret ;
				if( p_task_index )
					(*p_task_index) = -1 ;
				return nret;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "DC4CQueryWorkers ok" );
			}
		}
		
		FD_ZERO( & read_fds );
		FD_ZERO( & write_fds );
		FD_ZERO( & expect_fds );
		max_fd = -1 ;
		nret = DC4CSetTasksFds( penv , penv , & read_fds , & write_fds , & expect_fds , & max_fd ) ;
		if( nret == DC4C_INFO_NO_RUNNING )
		{
			DebugLog( __FILE__ , __LINE__ , "DC4CSetTasksFds return DC4C_INFO_NO_RUNNING" );
			sleep(1);
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CSetTasksFds failed[%d]" , nret );
			penv->interrupt_code = nret ;
			if( p_task_index )
				(*p_task_index) = -1 ;
			return nret;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DC4CSetTasksFds ok" );
			
			select_return_count = DC4CSelectTasksFds( & read_fds , & write_fds , & expect_fds , & max_fd , 1 ) ;
			if( select_return_count < 0 )
			{
				ErrorLog( __FILE__ , __LINE__ , "DC4CSelectTasksFds failed[%d]" , select_return_count );
				penv->interrupt_code = nret ;
				if( p_task_index )
					(*p_task_index) = -1 ;
				return select_return_count;
			}
			else if( select_return_count == 0 )
			{
				DebugLog( __FILE__ , __LINE__ , "DC4CSelectTasksFds timeout" );
			}
		}
		
		nret = DC4CProcessTasks( penv , & read_fds , & write_fds , & expect_fds , & task_index ) ;
		if( nret == DC4C_INFO_TASK_FINISHED )
		{
			DebugLog( __FILE__ , __LINE__ , "DC4CProcessTasks return DC4C_INFO_TASK_FINISHED" );
			if( p_task_index )
				(*p_task_index) = task_index ;
			return DC4C_INFO_TASK_FINISHED;
		}
		else if( nret == DC4C_INFO_BATCH_TASKS_FINISHED )
		{
			DebugLog( __FILE__ , __LINE__ , "DC4CProcessTasks return DC4C_INFO_BATCH_TASKS_FINISHED" );
			CloseSocket( & (penv->rserver_session) );
			if( p_task_index )
				(*p_task_index) = task_index ;
			return DC4C_INFO_BATCH_TASKS_FINISHED;
		}
		else if( nret == DC4C_ERROR_TIMEOUT )
		{
			DebugLog( __FILE__ , __LINE__ , "DC4CProcessTasks return DC4C_ERROR_TIMEOUT" );
			CloseSocket( & (penv->rserver_session) );
			penv->interrupt_code = DC4C_ERROR_TIMEOUT ;
			if( p_task_index )
				(*p_task_index) = task_index ;
			return DC4C_ERROR_TIMEOUT;
		}
		else if( nret == DC4C_ERROR_APP )
		{
			DebugLog( __FILE__ , __LINE__ , "DC4CProcessTasks return DC4C_ERROR_APP" );
			CloseSocket( & (penv->rserver_session) );
			penv->interrupt_code = DC4C_ERROR_APP ;
			return DC4C_ERROR_APP;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CProcessTasks failed[%d]" , nret );
			CloseSocket( & (penv->rserver_session) );
			penv->interrupt_code = nret ;
			if( p_task_index )
				(*p_task_index) = task_index ;
			return nret;
		}
	}
	
	return 0;
}

int DC4CPerformMultiBatchTasks( struct Dc4cApiEnv **a_penv , int envs_count , struct Dc4cApiEnv **p_penv , int *p_task_index )
{
	int			envs_index , begin_envs_index , c ;
	struct Dc4cApiEnv	*penv = NULL ;
	int			remain_envs_count ;
	
	fd_set			read_fds , write_fds , expect_fds ;
	int			max_fd ;
	int			select_return_count = 0 ;
	int			task_index ;
	
	int			nret = 0 ;
	
	begin_envs_index = ( rand() % envs_count ) ;
	remain_envs_count = 0 ;
	for( envs_index = 0  ; envs_index < envs_count ; envs_index++ )
	{
		penv = a_penv[envs_index] ;
		
		if( DC4CGetPrepareTasksCount(penv) == 0 && DC4CGetRunningTasksCount(penv) == 0 )
		{
			if( penv->finished_flag == 0 )
			{
				penv->finished_flag = 1 ;
				CloseSocket( & (penv->rserver_session) );
				if( p_penv )
					(*p_penv) = penv ;
				return DC4C_INFO_BATCH_TASKS_FINISHED;
			}
		}
		else if( penv->interrupt_code && DC4CGetRunningTasksCount(penv) == 0 )
		{
			if( penv->finished_flag == 0 )
			{
				penv->finished_flag = 1 ;
				if( p_penv )
					(*p_penv) = penv ;
				return DC4C_INFO_BATCH_TASKS_FINISHED;
			}
		}
		else
		{
			if( penv->finished_flag == 0 )
				remain_envs_count++;
		}
		
		if( DC4CGetPrepareTasksCount(penv) > DC4CGetPrepareTasksCount(a_penv[begin_envs_index]) )
		{
			begin_envs_index = envs_index ;
		}
	}
	if( remain_envs_count == 0 )
	{
		return DC4C_INFO_ALL_ENVS_FINISHED;
	}
	
	while(1)
	{
		FD_ZERO( & read_fds );
		FD_ZERO( & write_fds );
		FD_ZERO( & expect_fds );
		max_fd = -1 ;
		
		for( envs_index = begin_envs_index , c = 0 ; c < envs_count ; envs_index++ , c++ )
		{
			if( envs_index >= envs_count )
				envs_index = 0 ;
			
			penv = a_penv[envs_index] ;
			if( DC4CGetPrepareTasksCount(penv) == 0 && DC4CGetRunningTasksCount(penv) == 0 )
				continue;
			
			if( penv->finished_flag == 0 )
			{
				if( DC4CGetRunningTasksCount(penv) < DC4CGetWorkersCount(penv) && DC4CGetPrepareTasksCount(penv) > 0 )
				{
					nret = DC4CQueryWorkers( penv ) ;
					if( nret )
					{
						ErrorLog( __FILE__ , __LINE__ , "DC4CQueryWorkers failed[%d]" , nret );
						penv->interrupt_code = nret ;
						if( p_penv )
							(*p_penv) = penv ;
						if( p_task_index )
							(*p_task_index) = -1 ;
						return nret;
					}
					else
					{
						DebugLog( __FILE__ , __LINE__ , "DC4CQueryWorkers ok" );
					}
				}
			}
			
			if( penv->finished_flag == 0 )
			{
				nret = DC4CSetTasksFds( penv , NULL , & read_fds , & write_fds , & expect_fds , & max_fd ) ;
				if( nret == DC4C_INFO_NO_RUNNING )
				{
					DebugLog( __FILE__ , __LINE__ , "DC4CSetTasksFds return DC4C_INFO_NO_RUNNING" );
				}
				else if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "DC4CSetTasksFds failed[%d]" , nret );
					CloseSocket( & (penv->rserver_session) );
					penv->interrupt_code = nret ;
					if( p_penv )
						(*p_penv) = penv ;
					if( p_task_index )
						(*p_task_index) = -1 ;
					return nret;
				}
			}
		}
		
		if( max_fd != -1 )
		{
			select_return_count = DC4CSelectTasksFds( & read_fds , & write_fds , & expect_fds , & max_fd , 1 ) ;
			if( select_return_count < 0 )
			{
				ErrorLog( __FILE__ , __LINE__ , "DC4CSelectTasksFds failed[%d]" , select_return_count );
				penv->interrupt_code = nret ;
				if( p_penv )
					(*p_penv) = NULL ;
				if( p_task_index )
					(*p_task_index) = -1 ;
				return select_return_count;
			}
			else if( select_return_count == 0 )
			{
				DebugLog( __FILE__ , __LINE__ , "DC4CSelectTasksFds no event" );
			}
		}
		else
		{
			sleep(1);
		}
		
		for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
		{
			penv = a_penv[envs_index] ;
			if( DC4CGetPrepareTasksCount(penv) == 0 && DC4CGetRunningTasksCount(penv) == 0 )
				continue;
			
			if( penv->finished_flag == 0 )
			{
				nret = DC4CProcessTasks( penv , & read_fds , & write_fds , & expect_fds , & task_index ) ;
				if( nret == DC4C_INFO_TASK_FINISHED )
				{
					DebugLog( __FILE__ , __LINE__ , "DC4CProcessTasks return DC4C_INFO_TASK_FINISHED" );
					if( p_penv )
						(*p_penv) = penv ;
					if( p_task_index )
						(*p_task_index) = task_index ;
					return DC4C_INFO_TASK_FINISHED;
				}
				else if( nret == DC4C_INFO_BATCH_TASKS_FINISHED )
				{
					DebugLog( __FILE__ , __LINE__ , "DC4CProcessTasks return DC4C_INFO_BATCH_TASKS_FINISHED" );
					CloseSocket( & (penv->rserver_session) );
					if( p_penv )
						(*p_penv) = penv ;
					if( p_task_index )
						(*p_task_index) = task_index ;
					penv->finished_flag = 1 ;
					return DC4C_INFO_BATCH_TASKS_FINISHED;
				}
				else if( nret == DC4C_ERROR_TIMEOUT )
				{
					ErrorLog( __FILE__ , __LINE__ , "DC4CProcessTasks return DC4C_ERROR_TIMEOUT" );
					CloseSocket( & (penv->rserver_session) );
					penv->interrupt_code = DC4C_ERROR_TIMEOUT ;
					if( p_penv )
						(*p_penv) = penv ;
					if( p_task_index )
						(*p_task_index) = task_index ;
					return DC4C_ERROR_TIMEOUT;
				}
				else if( nret == DC4C_ERROR_APP )
				{
					ErrorLog( __FILE__ , __LINE__ , "DC4CProcessTasks return DC4C_ERROR_APP" );
					CloseSocket( & (penv->rserver_session) );
					penv->interrupt_code = DC4C_ERROR_APP ;
					if( p_penv )
						(*p_penv) = penv ;
					if( p_task_index )
						(*p_task_index) = task_index ;
					return DC4C_ERROR_APP;
				}
				else if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "DC4CProcessTasks failed[%d]" , nret );
					CloseSocket( & (penv->rserver_session) );
					penv->interrupt_code = nret ;
					if( p_penv )
						(*p_penv) = penv ;
					if( p_task_index )
						(*p_task_index) = task_index ;
					return nret;
				}
			}
		}
	}
}

int DC4CQueryWorkers( struct Dc4cApiEnv *penv )
{
	int			c ;
	
	int			query_count ;
	
	int			nret = 0 ;
	
	if( IsSocketEstablished( & (penv->rserver_session) ) == 0 )
	{
		for( c = 0 ; c < penv->rserver_count ; c++ )
		{
			nret = SyncConnectSocket( penv->rserver_ip[penv->rserver_index] , penv->rserver_port[penv->rserver_index] , & (penv->rserver_session) ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] failed[%d]errno[%d]" , penv->rserver_ip[penv->rserver_index] , penv->rserver_port[penv->rserver_index] , nret , errno );
				penv->rserver_index = (penv->rserver_index+1) % penv->rserver_count ;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] ok" , penv->rserver_ip[penv->rserver_index] , penv->rserver_port[penv->rserver_index] );
				break;
			}
		}
		if( c >= penv->rserver_count )
		{
			return DC4C_ERROR_CONNECT;
		}
	}
	
	if( penv->interrupt_code )
		return 0;
	
	if( penv->prepare_count == 0 )
	{
		if( penv->running_count == 0 && penv->finished_count == 0 )
			query_count = -1 ;
		else
			return 0;
	}
	else
	{
		if( penv->qwp._nodes_count - penv->query_count_used > 0 )
			return 0;
		
		query_count = penv->prepare_count ;
		if( query_count > penv->workers_count )
			query_count = penv->workers_count ;
	}
	
	penv->rserver_session.alive_timeout = 60 ;
	
	nret = proto_QueryWorkersRequest( & (penv->rserver_session) , query_count ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_QueryWorkersRequest failed[%d]errno[%d]" , nret , errno );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "proto_QueryWorkersRequest ok" );
	}
	
	nret = SyncSendSocketData( & (penv->rserver_session) , & (penv->rserver_session.alive_timeout) ) ; 
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncSendSocketData failed[%d]errno[%d]" , nret , errno );
		CloseSocket( & (penv->rserver_session) );
		return DC4C_ERROR_CONNECT;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "SyncSendSocketData ok" );
	}
	
	nret = SyncReceiveSocketData( & (penv->rserver_session) , & (penv->rserver_session.alive_timeout) ) ; 
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
		CloseSocket( & (penv->rserver_session) );
		return DC4C_ERROR_CONNECT;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
	}
	
	if( STRNCMP( penv->rserver_session.recv_buffer + LEN_COMMHEAD , != , "QWP" , 3 ) )
	{
		ErrorLog( __FILE__ , __LINE__ , "invalid msghead.msgtype[%.*s]" , 3 , penv->rserver_session.recv_buffer + LEN_COMMHEAD );
		CloseSocket( & (penv->rserver_session) );
		return DC4C_ERROR_INTERNAL;
	}
	
	nret = proto_QueryWorkersResponse( & (penv->rserver_session) , & (penv->qwp) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_QueryWorkersResponse failed[%d]errno[%d]" , nret , errno );
		CloseSocket( & (penv->rserver_session) );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "proto_QueryWorkersResponse ok" );
	}
	
	if( penv->qwp.error == 5 || penv->qwp._nodes_count == 0 )
	{
		InfoLog( __FILE__ , __LINE__ , "NO WORKER" );
		penv->query_count_used = 0 ;
	}
	else if( penv->qwp.error )
	{
		ErrorLog( __FILE__ , __LINE__ , "Query workers failed[%d]" , penv->qwp.error );
		return penv->qwp.error;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "GOT [%d]WORKERS" , penv->qwp._nodes_count );
		penv->query_count_used = 0 ;
	}
	
	return 0;
}

int DC4CSetTasksFds( struct Dc4cApiEnv *penv , struct Dc4cApiEnv *penv_QueryWorkers , fd_set *p_read_fds , fd_set *write_fds , fd_set *expect_fds , int *p_max_fd )
{
	execute_program_request		*task_request_ptr = NULL ;
	execute_program_response	*task_response_ptr = NULL ;
	struct SocketSession		*task_session_ptr = NULL ;
	int				task_index ;
	
	int				nret = 0 ;
	
	if( penv_QueryWorkers == NULL )
		penv_QueryWorkers = penv ;
	
	for( task_request_ptr = penv->tasks_request_array , task_response_ptr = penv->tasks_response_array , task_session_ptr = penv->tasks_session_array , task_index = 0
		; task_index < penv->tasks_count
		; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_index++ )
	{
		if( task_session_ptr->progress == WSERVER_SESSION_PROGRESS_EXECUTING )
		{
			FD_SET( task_session_ptr->sock , p_read_fds );
			if( task_session_ptr->sock > (*p_max_fd) )
				(*p_max_fd) = task_session_ptr->sock ;
			InfoLog( __FILE__ , __LINE__ , "[%p] FDSET ESTAB idx[%d] fd[%d] max[%d] task[%s]" , penv , task_index , task_session_ptr->sock , *p_max_fd , task_request_ptr->program_and_params );
		}
	}
	
	for( task_request_ptr = penv->tasks_request_array , task_response_ptr = penv->tasks_response_array , task_session_ptr = penv->tasks_session_array , task_index = 0
		; task_index < penv->tasks_count && penv_QueryWorkers->query_count_used < penv_QueryWorkers->qwp._nodes_count && penv->running_count < penv->workers_count && penv->interrupt_code == 0
		; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_index++ )
	{
		if( task_session_ptr->progress == WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING )
		{
			if( STRCMP( task_request_ptr->program_and_params , == , "" ) )
			{
				PREPARE_COUNT_DECREASE
				FINISHED_COUNT_INCREASE
				task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED ;
				task_response_ptr->error = DC4C_ERROR_PARAMETER ;
				continue;
			}
			
_GOTO_CONNECT :
			nret = SyncConnectSocket( penv_QueryWorkers->qwp.nodes[penv_QueryWorkers->query_count_used].node.ip , penv_QueryWorkers->qwp.nodes[penv_QueryWorkers->query_count_used].node.port , task_session_ptr ) ;
			if( nret )
			{
				WarnLog( __FILE__ , __LINE__ , "SyncConnectSocket failed[%d]" , nret );
				penv_QueryWorkers->query_count_used++;
				if( penv_QueryWorkers->query_count_used >= penv_QueryWorkers->qwp._nodes_count )
					break;
				goto _GOTO_CONNECT;
			}
			
			task_session_ptr->alive_timeout = task_request_ptr->timeout ;
			
			nret = proto_ExecuteProgramRequest( task_session_ptr , task_request_ptr->order_index , task_request_ptr->program_and_params , task_request_ptr->timeout , penv->options , task_request_ptr ) ;
			if( nret )
			{
				WarnLog( __FILE__ , __LINE__ , "proto_ExecuteProgramRequest failed[%d]errno[%d]" , nret , errno );
				CloseSocket( task_session_ptr );
				task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR ;
				task_response_ptr->error = DC4C_ERROR_APP ;
				return nret;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "proto_ExecuteProgramRequest ok" );
			}
			
			nret = SyncSendSocketData( task_session_ptr , & (task_session_ptr->alive_timeout) ) ; 
			if( nret )
			{
				WarnLog( __FILE__ , __LINE__ , "SyncSendSocketData failed[%d]errno[%d]" , nret , errno );
				CloseSocket( task_session_ptr );
				task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
				continue;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "SyncSendSocketData ok" );
			}
			
			FD_SET( task_session_ptr->sock , p_read_fds );
			if( task_session_ptr->sock > (*p_max_fd) )
				(*p_max_fd) = task_session_ptr->sock ;
			InfoLog( __FILE__ , __LINE__ , "[%p] FDSET CONN idx[%d] fd[%d] max[%d] task[%s]" , penv , task_index , task_session_ptr->sock , *p_max_fd , task_request_ptr->program_and_params );
			penv->query_count_used++;
			PREPARE_COUNT_DECREASE
			RUNNING_COUNT_INCREASE
			task_session_ptr->progress = WSERVER_SESSION_PROGRESS_EXECUTING ;
			if( penv->pfuncDC4COnBeginTaskProc )
			{
				penv->pfuncDC4COnBeginTaskProc( penv , task_index , penv->p1 , penv->p2 );
			}
		}
	}
	
	if( (*p_max_fd) == -1 )
		return DC4C_INFO_NO_RUNNING;
	else
		return 0;
}

int DC4CSelectTasksFds( fd_set *p_read_fds , fd_set *write_fds , fd_set *expect_fds , int *p_max_fd , int select_timeout )
{
	struct timeval			tv ;
	int				select_return_count ;
	
	DebugLog( __FILE__ , __LINE__ , "select ..." );
	tv.tv_sec = select_timeout ;
	tv.tv_usec = 0 ;
	select_return_count = select( (*p_max_fd)+1 , p_read_fds , NULL , NULL , & tv ) ;
	DebugLog( __FILE__ , __LINE__ , "select return[%d]" , select_return_count );
	if( select_return_count < 0 )
		return DC4C_ERROR_SELECT;
	else
		return 0;
}

int DC4CProcessTasks( struct Dc4cApiEnv *penv , fd_set *p_read_fds , fd_set *write_fds , fd_set *expect_fds , int *p_task_index )
{
	execute_program_request		*task_request_ptr = NULL ;
	execute_program_response	*task_response_ptr = NULL ;
	struct SocketSession		*task_session_ptr = NULL ;
	int				task_index ;
	time_t				tt ;
	
	deploy_program_request		dpq ;
	
	int				nret = 0 ;
	
	if( penv->prepare_count == 0 && penv->running_count == 0 )
		return DC4C_INFO_BATCH_TASKS_FINISHED;
	
	time( & tt );
	
	for( task_request_ptr = penv->tasks_request_array , task_response_ptr = penv->tasks_response_array , task_session_ptr = penv->tasks_session_array , task_index = 0
		; task_index < penv->tasks_count
		; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_index++ )
	{
		if( IsSocketEstablished( task_session_ptr ) && task_session_ptr->progress == WSERVER_SESSION_PROGRESS_EXECUTING )
		{
			task_session_ptr->alive_timeout -= tt - task_session_ptr->active_timestamp ;
			time( & (task_session_ptr->active_timestamp) );
			if( task_request_ptr->timeout > 0 && task_session_ptr->alive_timeout <= 0 && TestAttribute( penv->options , DC4C_OPTIONS_INTERRUPT_BY_APP ) )
			{
				ErrorLog( __FILE__ , __LINE__ , "task session timeout" );
				ErrorLog( __FILE__ , __LINE__ , "[%p] FDSET TIMEOUT idx[%d] fd[%d] task[%s]" , penv , task_index , task_session_ptr->sock , task_request_ptr->program_and_params );
				CloseSocket( task_session_ptr );
				RUNNING_COUNT_DECREASE
				PREPARE_COUNT_INCREASE
				task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR ;
				task_response_ptr->error = DC4C_ERROR_TIMEOUT ;
				if( penv->pfuncDC4COnFinishTaskProc )
				{
					penv->pfuncDC4COnFinishTaskProc( penv , task_index , penv->p1 , penv->p2 );
				}
				if( p_task_index )
					(*p_task_index) = task_index ;
				return DC4C_ERROR_TIMEOUT;
			}
		}
	}
	
	for( task_request_ptr = penv->tasks_request_array , task_response_ptr = penv->tasks_response_array , task_session_ptr = penv->tasks_session_array , task_index = 0
		; task_index < penv->tasks_count
		; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_index++ )
	{
		if( FD_ISSET( task_session_ptr->sock , p_read_fds ) && IsSocketEstablished( task_session_ptr ) && task_session_ptr->progress == WSERVER_SESSION_PROGRESS_EXECUTING )
		{
			nret = SyncReceiveSocketData( task_session_ptr , & (task_session_ptr->alive_timeout) ) ; 
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
				ErrorLog( __FILE__ , __LINE__ , "[%p] FDSET ERROR idx[%d] fd[%d] task[%s] nret[%d]" , penv , task_index , task_session_ptr->sock , task_request_ptr->program_and_params , nret );
				CloseSocket( task_session_ptr );
				RUNNING_COUNT_DECREASE
				PREPARE_COUNT_INCREASE
				task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR ;
				task_response_ptr->error = DC4C_ERROR_SEND_OR_RECEIVE ;
				if( penv->pfuncDC4COnFinishTaskProc )
				{
					penv->pfuncDC4COnFinishTaskProc( penv , task_index , penv->p1 , penv->p2 );
				}
				if( p_task_index )
					(*p_task_index) = task_index ;
				return task_response_ptr->error;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
			}
			
			if( STRNCMP( task_session_ptr->recv_buffer + LEN_COMMHEAD , == , "DPQ" , 3 ) )
			{
				nret = proto_DeployProgramRequest( task_session_ptr , & dpq ) ;
				if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest failed[%d]errno[%d]" , nret , errno );
					ErrorLog( __FILE__ , __LINE__ , "[%p] FDSET ERROR idx[%d] fd[%d] task[%s] nret[%d]" , penv , task_index , task_session_ptr->sock , task_request_ptr->program_and_params , nret );
					CloseSocket(task_session_ptr );
					RUNNING_COUNT_DECREASE
					PREPARE_COUNT_INCREASE
					task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR ;
					task_response_ptr->error = nret ;
					if( penv->pfuncDC4COnFinishTaskProc )
					{
						penv->pfuncDC4COnFinishTaskProc( penv , task_index , penv->p1 , penv->p2 );
					}
					if( p_task_index )
						(*p_task_index) = task_index ;
					return task_response_ptr->error;
				}
				else
				{
					InfoLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest ok" );
				}
				
				nret = proto_DeployProgramResponse( task_session_ptr , dpq.program ) ;
				if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest failed[%d]errno[%d]" , nret , errno );
					ErrorLog( __FILE__ , __LINE__ , "[%p] FDSET ERROR idx[%d] fd[%d] task[%s] nret[%d]" , penv , task_index , task_session_ptr->sock , task_request_ptr->program_and_params , nret );
					CloseSocket( task_session_ptr );
					RUNNING_COUNT_DECREASE
					PREPARE_COUNT_INCREASE
					task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR ;
					task_response_ptr->error = nret ;
					if( penv->pfuncDC4COnFinishTaskProc )
					{
						penv->pfuncDC4COnFinishTaskProc( penv , task_index , penv->p1 , penv->p2 );
					}
					if( p_task_index )
						(*p_task_index) = task_index ;
					return task_response_ptr->error;
				}
				else
				{
					InfoLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest ok" );
				}
				
				nret = SyncSendSocketData( task_session_ptr , & (task_session_ptr->alive_timeout) ) ; 
				if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
					ErrorLog( __FILE__ , __LINE__ , "[%p] FDSET ERROR idx[%d] fd[%d] task[%s] nret[%d]" , penv , task_index , task_session_ptr->sock , task_request_ptr->program_and_params , nret );
					CloseSocket( task_session_ptr );
					RUNNING_COUNT_DECREASE
					PREPARE_COUNT_INCREASE
					task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR ;
					task_response_ptr->error = DC4C_ERROR_SEND_OR_RECEIVE ;
					if( penv->pfuncDC4COnFinishTaskProc )
					{
						penv->pfuncDC4COnFinishTaskProc( penv , task_index , penv->p1 , penv->p2 );
					}
					if( p_task_index )
						(*p_task_index) = task_index ;
					return task_response_ptr->error;
				}
				else
				{
					InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
				}
			}
			else if( STRNCMP( task_session_ptr->recv_buffer + LEN_COMMHEAD , == , "EPP" , 3 ) )
			{
				nret = proto_ExecuteProgramResponse( task_session_ptr , task_response_ptr ) ;
				if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse failed[%d]errno[%d]" , nret , errno );
					ErrorLog( __FILE__ , __LINE__ , "[%p] FDSET ERROR idx[%d] fd[%d] task[%s] nret[%d]" , penv , task_index , task_session_ptr->sock , task_request_ptr->program_and_params , nret );
					CloseSocket( task_session_ptr );
					RUNNING_COUNT_DECREASE
					PREPARE_COUNT_INCREASE
					task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR ;
					task_response_ptr->error = nret ;
					if( penv->pfuncDC4COnFinishTaskProc )
					{
						penv->pfuncDC4COnFinishTaskProc( penv , task_index , penv->p1 , penv->p2 );
					}
					if( p_task_index )
						(*p_task_index) = task_index ;
					return task_response_ptr->error;
				}
				else
				{
					InfoLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse ok" );
				}
				
				if( task_response_ptr->error == DC4C_INFO_ALREADY_EXECUTING )
				{
					InfoLog( __FILE__ , __LINE__ , "ejected by worker" );
					InfoLog( __FILE__ , __LINE__ , "[%p] FDSET ALREADY idx[%d] fd[%d] task[%s] error[%d]" , penv , task_index , task_session_ptr->sock , task_request_ptr->program_and_params , task_response_ptr->error );
					CloseSocket( task_session_ptr );
					RUNNING_COUNT_DECREASE
					PREPARE_COUNT_INCREASE
					task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
					if( penv->pfuncDC4COnCancelTaskProc )
					{
						penv->pfuncDC4COnCancelTaskProc( penv , task_index , penv->p1 , penv->p2 );
					}
				}
				else
				{
					CloseSocket( task_session_ptr );
					if( task_response_ptr->status )
					{
						if( TestAttribute( penv->options , DC4C_OPTIONS_INTERRUPT_BY_APP ) )
						{
							ErrorLog( __FILE__ , __LINE__ , "[%p] FDSET CLOSE idx[%d] fd[%d] task[%s] status[%d] WEXITSTATUS(status)[%d] options[%d]" , penv , task_index , task_session_ptr->sock , task_request_ptr->program_and_params , task_response_ptr->status , WEXITSTATUS(task_response_ptr->status) , penv->options );
							RUNNING_COUNT_DECREASE
							PREPARE_COUNT_INCREASE
							task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR ;
							task_response_ptr->error = DC4C_ERROR_APP ;
							if( penv->pfuncDC4COnFinishTaskProc )
							{
								penv->pfuncDC4COnFinishTaskProc( penv , task_index , penv->p1 , penv->p2 );
							}
							if( p_task_index )
								(*p_task_index) = task_index ;
							return DC4C_ERROR_APP;
						}
						else
						{
							WarnLog( __FILE__ , __LINE__ , "[%p] FDSET CLOSE idx[%d] fd[%d] task[%s] status[%d] WEXITSTATUS(status)[%d] options[%d]" , penv , task_index , task_session_ptr->sock , task_request_ptr->program_and_params , task_response_ptr->status , WEXITSTATUS(task_response_ptr->status) , penv->options );
							RUNNING_COUNT_DECREASE
							FINISHED_COUNT_INCREASE
							task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR ;
							if( penv->pfuncDC4COnFinishTaskProc )
							{
								penv->pfuncDC4COnFinishTaskProc( penv , task_index , penv->p1 , penv->p2 );
							}
							if( p_task_index )
								(*p_task_index) = task_index ;
							return DC4C_INFO_TASK_FINISHED;
						}
					}
					else
					{
						InfoLog( __FILE__ , __LINE__ , "[%p] FDSET CLOSE idx[%d] fd[%d] task[%s] status[%d] WEXITSTATUS(status)[%d] options[%d]" , penv , task_index , task_session_ptr->sock , task_request_ptr->program_and_params , task_response_ptr->status , WEXITSTATUS(task_response_ptr->status) , penv->options );
						RUNNING_COUNT_DECREASE
						FINISHED_COUNT_INCREASE
						task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED ;
						if( penv->pfuncDC4COnFinishTaskProc )
						{
							penv->pfuncDC4COnFinishTaskProc( penv , task_index , penv->p1 , penv->p2 );
						}
						if( p_task_index )
							(*p_task_index) = task_index ;
						return DC4C_INFO_TASK_FINISHED;
					}
				}
			}
		}
	}
	
	return 0;
}

void DC4CSetProcDataPtr( struct Dc4cApiEnv *penv , void *p1 , void *p2 )
{
	penv->p1 = p1 ;
	penv->p2 = p2 ;
	return;
}

void DC4CSetOnBeginTaskProc( struct Dc4cApiEnv *penv , funcDC4COnBeginTaskProc *pfuncDC4COnBeginTaskProc )
{
	penv->pfuncDC4COnBeginTaskProc = pfuncDC4COnBeginTaskProc ;
	return;
}

void DC4CSetOnCancelTaskProc( struct Dc4cApiEnv *penv , funcDC4COnBeginTaskProc *pfuncDC4COnCancelTaskProc )
{
	penv->pfuncDC4COnCancelTaskProc = pfuncDC4COnCancelTaskProc ;
	return;
}

void DC4CSetOnFinishTaskProc( struct Dc4cApiEnv *penv , funcDC4COnFinishTaskProc *pfuncDC4COnFinishTaskProc )
{
	penv->pfuncDC4COnFinishTaskProc = pfuncDC4COnFinishTaskProc ;
	return;
}

int DC4CGetTasksCount( struct Dc4cApiEnv *penv )
{
	return penv->tasks_count;
}

int DC4CGetWorkersCount( struct Dc4cApiEnv *penv )
{
	return penv->workers_count;
}

int DC4CGetPrepareTasksCount( struct Dc4cApiEnv *penv )
{
	return penv->prepare_count;
}

void DC4CSetPrepareTasksCount( struct Dc4cApiEnv *penv , int prepare_count )
{
	penv->prepare_count = prepare_count ;
	return;
}

int DC4CGetRunningTasksCount( struct Dc4cApiEnv *penv )
{
	return penv->running_count;
}

void DC4CSetRunningTasksCount( struct Dc4cApiEnv *penv , int running_count )
{
	penv->running_count = running_count ;
	return;
}

int DC4CGetFinishedTasksCount( struct Dc4cApiEnv *penv )
{
	return penv->finished_count;
}

void DC4CSetFinishedTasksCount( struct Dc4cApiEnv *penv , int finished_count )
{
	penv->finished_count = finished_count ;
	return;
}

void DC4CResetFinishedTasksWithError( struct Dc4cApiEnv *penv )
{
	struct SocketSession	*task_session_ptr = NULL ;
	int			task_index ;
	
	for( task_session_ptr = penv->tasks_session_array , task_index = 0
		; task_index < penv->tasks_count
		; task_session_ptr++ , task_index++ )
	{
		if( task_session_ptr->progress == WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR )
		{
			task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
			penv->finished_flag = 0 ;
			penv->interrupt_code = 0 ;
		}
	}
	
	return;
}

int DC4CGetUnusedWorkersCount( struct Dc4cApiEnv *penv )
{
	return penv->qwp._nodes_count - penv->query_count_used ;
}

void DC4CSetAppLogFile( char *program )
{
	char	*WSERVER_INDEX = NULL ;
	char	*WSERVER_IP_PORT = NULL ;
	
	WSERVER_INDEX = getenv("WSERVER_INDEX") ;
	WSERVER_IP_PORT = getenv("WSERVER_IP_PORT") ;
	
	if( program[0] == '#' )
		SetLogFile( "%s" , program );
	else if( WSERVER_INDEX == NULL || WSERVER_IP_PORT == NULL )
		SetLogFile( "%s/log/%s.log" , getenv("HOME") , program );
	else
		SetLogFile( "%s/log/dc4c_wserver_%d_%s.%s.log" , getenv("HOME") , atoi(WSERVER_INDEX) , WSERVER_IP_PORT , program );
	
	return;
}

int DC4CFormatReplyInfo( char *format , ... )
{
	va_list				valist ;
	execute_program_response	rsp ;
	int				size ;
	
	va_start( valist , format );
	memset( & rsp , 0x00 , sizeof(execute_program_response) );
	size = (int)vsnprintf( rsp.info , sizeof(rsp.info)-1 , format , valist ) ;
	va_end( valist );
	if( size <= 0 )
		return DC4C_ERROR_PARAMETER;
	
	return DC4CSetReplyInfoEx( rsp.info , size );
}

int DC4CSetReplyInfo( char *str )
{
	return DC4CSetReplyInfoEx( str , strlen(str) );
}

int DC4CSetReplyInfoEx( char *buf , int len )
{
	char		*envptr = NULL ;
	int		info_pipe ;
	
	envptr = getenv( "WSERVER_INFO_PIPE" ) ;
	if( envptr == NULL )
		return DC4C_ERROR_INTERNAL;
	info_pipe = atoi(envptr) ;
	
	return (int)write( info_pipe , buf , (size_t)len );
}

char *ConvertTimeString( time_t tt , char *buf , size_t bufsize )
{
	struct tm	stime ;
	
	memset( & stime , 0x00 , sizeof(struct tm) );
	localtime_r( & tt , & stime );
	memset( buf , 0x00 , bufsize );
	strftime( buf , bufsize , "%Y-%m-%d %H:%M:%S" , & stime );
	
	return buf;
}

char *GetTimeStringNow( char *buf , size_t bufsize )
{
	time_t	tt ;
	time( & tt );
	return ConvertTimeString( tt , buf , bufsize );
}

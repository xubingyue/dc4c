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

char __DC4C_VERSION_1_1_4[] = "1.1.4" ;
char *__DC4C_VERSION = __DC4C_VERSION_1_1_4 ;

#define RSERVER_ARRAYSIZE				8

#define WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING	0
#define WSERVER_SESSION_PROGRESS_EXECUTING		1
#define WSERVER_SESSION_PROGRESS_FINISHED		2
#define WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR	3

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
	
	int				prepare_count ;
	int				running_count ;
	int				finished_count ;
	
	query_workers_response		qwp ;
	int				query_count_used ;
	
	unsigned long			options ;
	int				interrupted_flag ;
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

static int proto_ExecuteProgramRequest( struct SocketSession *psession , char *program_and_params , int timeout , int options , execute_program_request *p_req )
{
	char			program[ MAXLEN_FILENAME + 1 ] ;
	char			pathfilename[ MAXLEN_FILENAME + 1 ] ;
	struct timeval		tv ;
	
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
	strcpy( req.program_and_params , program_and_params );
	req.timeout = timeout ;
	req.bind_cpu_flag = TestAttribute( options , DC4C_OPTIONS_BIND_CPU )?1:0 ;
	
	memset( program , 0x00 , sizeof(program) );
	sscanf( program_and_params , "%s" , program );
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	SNPRINTF( pathfilename , sizeof(pathfilename)-1 , "%s/bin/%s" , getenv("HOME") , program );
	memset( req.program_md5_exp , 0x00 , sizeof(req.program_md5_exp) );
	nret = FileMd5( pathfilename , req.program_md5_exp ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "FileMd5 failed[%d]" , nret );
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

int DC4CCleanEnv( struct Dc4cApiEnv **ppenv )
{
	int		i ;
	
	if( IsSocketEstablished( & ((*ppenv)->rserver_session) ) )
	{
		CloseSocket( & ((*ppenv)->rserver_session) );
	}
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
	
	return 0;
}

int DC4CInitEnv( struct Dc4cApiEnv **ppenv , char *rservers_ip_port )
{
	char		rservers_ip_port_copy[ 1024 + 1 ] ;
	char		*p = NULL ;
	
	int		nret = 0 ;
	
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
	else if( ( p = getenv("DC4C_RSERVER_IP_PORT") ) )
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

int DC4CDoTask( struct Dc4cApiEnv *penv , char *program_and_params , int timeout )
{
	struct Dc4cBatchTask	task ;
	
	strcpy( task.program_and_params , program_and_params );
	task.timeout = timeout ;
	
	return DC4CDoBatchTasks( penv , 1 , & task , 1 );
}

int DC4CGetTaskIp( struct Dc4cApiEnv *penv , char **pp_ip )
{
	return DC4CGetBatchTasksIp( penv , 0 , pp_ip );
}

int DC4CGetTaskPort( struct Dc4cApiEnv *penv , long *p_port )
{
	return DC4CGetBatchTasksPort( penv , 0 , p_port );
}

int DC4CGetTaskTid( struct Dc4cApiEnv *penv , char **pp_tid )
{
	return DC4CGetBatchTasksTid( penv , 0 , pp_tid );
}

int DC4CGetTaskProgramAndParam( struct Dc4cApiEnv *penv , char **pp_program_and_params )
{
	return DC4CGetBatchTasksProgramAndParam( penv , 0 , pp_program_and_params );
}

int DC4CGetTaskTimeout( struct Dc4cApiEnv *penv , int *p_timeout )
{
	return DC4CGetBatchTasksTimeout( penv , 0 , p_timeout );
}

int DC4CGetTaskElapse( struct Dc4cApiEnv *penv , int *p_elapse )
{
	return DC4CGetBatchTasksElapse( penv , 0 , p_elapse );
}

int DC4CGetTaskError( struct Dc4cApiEnv *penv , int *p_error )
{
	return DC4CGetBatchTasksError( penv , 0 , p_error );
}

int DC4CGetTaskStatus( struct Dc4cApiEnv *penv , int *p_status )
{
	return DC4CGetBatchTasksStatus( penv , 0 , p_status );
}

int DC4CGetTaskInfo( struct Dc4cApiEnv *penv , char **pp_info )
{
	return DC4CGetBatchTasksInfo( penv , 0 , pp_info );
}

#define PREPARE_COUNT_INCREASE													\
	DebugLog( __FILE__ , __LINE__ , "COUNT prepare_count[%d]->[%d]" , penv->prepare_count , penv->prepare_count+1 );	\
	penv->prepare_count++;													\

#define PREPARE_COUNT_DECREASE													\
	DebugLog( __FILE__ , __LINE__ , "COUNT prepare_count[%d]->[%d]" , penv->prepare_count , penv->prepare_count-1 );	\
	penv->prepare_count--;													\

#define RUNNING_COUNT_INCREASE													\
	DebugLog( __FILE__ , __LINE__ , "COUNT running_count[%d]->[%d]" , penv->running_count , penv->running_count+1 );	\
	penv->running_count++;													\

#define RUNNING_COUNT_DECREASE													\
	DebugLog( __FILE__ , __LINE__ , "COUNT running_count[%d]->[%d]" , penv->running_count , penv->running_count-1 );	\
	penv->running_count--;													\

#define FINISHED_COUNT_INCREASE													\
	DebugLog( __FILE__ , __LINE__ , "COUNT finished_count[%d]->[%d]" , penv->finished_count , penv->finished_count+1 );	\
	penv->finished_count++;													\

#define FINISHED_COUNT_DECREASE													\
	DebugLog( __FILE__ , __LINE__ , "COUNT finished_count[%d]->[%d]" , penv->finished_count , penv->finished_count-1 );	\
	penv->finished_count--;													\

int DC4CDoBatchTasks( struct Dc4cApiEnv *penv , int workers_count , struct Dc4cBatchTask *p_tasks , int tasks_count )
{
	fd_set		read_fds ;
	int		max_fd ;
	int		select_return_count ;
	
	int		nret = 0 ;
	
	if( tasks_count == 0 )
		return 0;
	
	nret = DC4CBeginBatchTasks( penv , workers_count , p_tasks , tasks_count ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CBeginBatchTasks failed[%d]" , nret );
		return nret;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DC4CBeginBatchTasks ok" );
	}
	
	penv->interrupted_flag = 0 ;
	
	while(1)
	{
		if(	penv->interrupted_flag == 0
			&& DC4CGetRunningTasksCount(penv) < DC4CGetWorkersCount(penv)
			&& DC4CGetPrepareTasksCount(penv) > 0
			&& penv->qwp._nodes_count - penv->query_count_used == 0 )
		{
			nret = DC4CQueryWorkers( penv ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "DC4CQueryWorkers failed[%d]" , nret );
				return nret;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "DC4CQueryWorkers ok" );
			}
		}
		
		FD_ZERO( & read_fds );
		max_fd = -1 ;
		nret = DC4CSetBatchTasksFds( penv , & read_fds , & max_fd ) ;
		if( nret == DC4C_INFO_NO_RUNNING )
		{
			DebugLog( __FILE__ , __LINE__ , "DC4CSetBatchTasksFds return DC4C_INFO_NO_RUNNING" );
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CSetBatchTasksFds failed[%d]" , nret );
			return nret;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DC4CSetBatchTasksFds ok" );
			
			select_return_count = DC4CSelectBatchTasksFds( & read_fds , & max_fd , 1 ) ;
			if( select_return_count < 0 )
			{
				ErrorLog( __FILE__ , __LINE__ , "DC4CSelectBatchTasksFds failed[%d]" , select_return_count );
				return select_return_count;
			}
			else if( select_return_count > 0 )
			{
				nret = DC4CProcessBatchTasks( penv , & read_fds , & max_fd ) ;
				if( nret == DC4C_INFO_NO_PREPARE_AND_RUNNING )
				{
					DebugLog( __FILE__ , __LINE__ , "DC4CProcessBatchTasks return DC4C_INFO_NO_PREPARE_AND_RUNNING" );
					return 0;
				}
				else if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "DC4CProcessBatchTasks failed[%d]" , nret );
					return nret;
				}
			}
		}
		
		if( penv->interrupted_flag == 1 && DC4CGetRunningTasksCount(penv) == 0 )
			return DC4C_ERROR_APP;
		
		if( penv->qwp._nodes_count == 0 )
		{
			sleep(1);
		}
	}
}

int DC4CPerformMultiBatchTasks( struct Dc4cApiEnv **ppenvs , int envs_count , struct Dc4cApiEnv **p_penv , int *p_remain_envs_count )
{
	int			envs_index ;
	struct Dc4cApiEnv	*penv = NULL ;
	struct SocketSession	*task_session_ptr = NULL ;
	int			task_idx ;
	int			remain_envs_count ;
	
	fd_set			read_fds ;
	int			max_fd ;
	int			select_return_count ;
	
	int			nret = 0 ;
	
	remain_envs_count = 0 ;
	for( envs_index = 0  ; envs_index < envs_count ; envs_index++ )
	{
		penv = ppenvs[envs_index] ;
		
		if( DC4CGetPrepareTasksCount(penv) > 0 || DC4CGetRunningTasksCount(penv) > 0 )
			remain_envs_count++;
		
		for( task_session_ptr = penv->tasks_session_array , task_idx = 0
			; task_idx < penv->tasks_count
			; task_session_ptr++ , task_idx++ )
		{
			if( task_session_ptr->progress == WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR )
				task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
		}
		
		penv->interrupted_flag = 0 ;
	}
	if( remain_envs_count == 0 )
	{
		if( p_remain_envs_count )
			(*p_remain_envs_count) = 0 ;
		return DC4C_INFO_NO_UNFINISHED_ENVS;
	}
	
	while(1)
	{
		for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
		{
			penv = ppenvs[envs_index] ;
			if( DC4CGetPrepareTasksCount(penv) == 0 && DC4CGetRunningTasksCount(penv) == 0 )
				continue;
			
			if(	penv->interrupted_flag == 0
				&& DC4CGetRunningTasksCount(penv) < DC4CGetWorkersCount(penv)
				&& DC4CGetPrepareTasksCount(penv) > 0
				&& penv->qwp._nodes_count - penv->query_count_used == 0 )
			{
				nret = DC4CQueryWorkers( penv ) ;
				if( nret )
				{
					(*p_penv) = penv ;
					ErrorLog( __FILE__ , __LINE__ , "DC4CQueryWorkers failed[%d]" , nret );
					return nret;
				}
				else
				{
					DebugLog( __FILE__ , __LINE__ , "DC4CQueryWorkers ok" );
				}
			}
		}
		
		FD_ZERO( & read_fds );
		max_fd = -1 ;
		for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
		{
			penv = ppenvs[envs_index] ;
			if( DC4CGetPrepareTasksCount(penv) == 0 && DC4CGetRunningTasksCount(penv) == 0 )
				continue;
			
			nret = DC4CSetBatchTasksFds( penv , & read_fds , & max_fd ) ;
			if( nret == DC4C_INFO_NO_RUNNING )
			{
				DebugLog( __FILE__ , __LINE__ , "DC4CSetBatchTasksFds return DC4C_INFO_NO_RUNNING" );
			}
			else if( nret )
			{
				(*p_penv) = penv ;
				ErrorLog( __FILE__ , __LINE__ , "DC4CSetBatchTasksFds failed[%d]" , nret );
				return nret;
			}
		}
		if( max_fd != -1 )
		{
			select_return_count = DC4CSelectBatchTasksFds( & read_fds , & max_fd , 1 ) ;
			if( select_return_count < 0 )
			{
				ErrorLog( __FILE__ , __LINE__ , "DC4CSelectBatchTasksFds failed[%d]" , select_return_count );
				(*p_penv) = NULL ;
				return select_return_count;
			}
			else if( select_return_count > 0 )
			{
				for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
				{
					penv = ppenvs[envs_index] ;
					if( DC4CGetPrepareTasksCount(penv) == 0 && DC4CGetRunningTasksCount(penv) == 0 )
						continue;
					
					nret = DC4CProcessBatchTasks( penv , & read_fds , & max_fd ) ;
					if( nret == DC4C_INFO_NO_PREPARE_AND_RUNNING )
					{
						DebugLog( __FILE__ , __LINE__ , "DC4CSelectBatchTasksFds return DC4C_INFO_NO_PREPARE_AND_RUNNING" );
						if( p_penv )
							(*p_penv) = penv ;
						if( p_remain_envs_count )
							(*p_remain_envs_count) = remain_envs_count - 1 ;
						return 0;
					}
					else if( nret )
					{
						ErrorLog( __FILE__ , __LINE__ , "DC4CProcessBatchTasks failed[%d]" , nret );
						(*p_penv) = penv ;
						return nret;
					}
				}
			}
		}
		
		for( envs_index = 0 ; envs_index < envs_count ; envs_index++ )
		{
			penv = ppenvs[envs_index] ;
			if( DC4CGetPrepareTasksCount(penv) == 0 && DC4CGetRunningTasksCount(penv) == 0 )
				continue;
			
			if( penv->interrupted_flag == 1 )
			{
				int			envs_index2 ;
				struct Dc4cApiEnv	*penv2 = NULL ;
				for( envs_index2 = 0 ; envs_index2 < envs_count ; envs_index2++ )
				{
					penv2 = ppenvs[envs_index2] ;
					if( DC4CGetPrepareTasksCount(penv2) == 0 && DC4CGetRunningTasksCount(penv2) == 0 )
						continue;
					
					if( DC4CGetRunningTasksCount(penv2) > 0 )
						break;
				}
				if( envs_index2 >= envs_count )
				{
					if( p_penv )
						(*p_penv) = penv ;
					if( p_remain_envs_count )
						(*p_remain_envs_count) = remain_envs_count - 1 ;
					return DC4C_ERROR_APP;
				}
			}
		}
		
		if( penv->qwp._nodes_count == 0 )
		{
			sleep(1);
		}
	}
}

int DC4CBeginBatchTasks( struct Dc4cApiEnv *penv , int workers_count , struct Dc4cBatchTask *p_tasks , int tasks_count )
{
	struct Dc4cBatchTask		*p_task = NULL ;
	int				i ;
	int				c ;
	
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
	
	for( i = 0 , p_task = p_tasks ; i < penv->tasks_count ; i++ , p_task++ )
	{
		strcpy( penv->tasks_request_array[i].program_and_params , p_task->program_and_params );
		penv->tasks_request_array[i].timeout = p_task->timeout ;
		if( penv->timeout > penv->tasks_request_array[i].timeout )
			penv->tasks_request_array[i].timeout = penv->timeout ;
		InfoLog( __FILE__ , __LINE__ , "TASK[%d] [%s] [%d]" , i , penv->tasks_request_array[i].program_and_params , penv->tasks_request_array[i].timeout );
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
	
	DSCINIT_query_workers_response( & (penv->qwp) );
	penv->query_count_used = 0 ;
	
	return 0;
}

int DC4CQueryWorkers( struct Dc4cApiEnv *penv )
{
	int			query_count ;
	
	int			nret = 0 ;
	
	if( penv->interrupted_flag == 1 )
		return 0;
	
	query_count = penv->prepare_count ;
	if( query_count > penv->workers_count )
		query_count = penv->workers_count ;
	
	if( query_count > 0 )
	{
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
			return 0;
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
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

int DC4CSetBatchTasksFds( struct Dc4cApiEnv *penv , fd_set *p_read_fds , int *p_max_fd )
{
	execute_program_request		*task_request_ptr = NULL ;
	execute_program_response	*task_response_ptr = NULL ;
	struct SocketSession		*task_session_ptr = NULL ;
	int				task_idx ;
	
	int				nret = 0 ;
	
	for( task_request_ptr = penv->tasks_request_array , task_response_ptr = penv->tasks_response_array , task_session_ptr = penv->tasks_session_array , task_idx = 0
		; task_idx < penv->tasks_count
		; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_idx++ )
	{
		if( task_session_ptr->progress == WSERVER_SESSION_PROGRESS_EXECUTING )
		{
			FD_SET( task_session_ptr->sock , p_read_fds );
			if( task_session_ptr->sock > (*p_max_fd) )
				(*p_max_fd) = task_session_ptr->sock ;
			InfoLog( __FILE__ , __LINE__ , "FDSET ESTAB task_idx[%d] FD_SET[%d] max_fd[%d]" , task_idx , task_session_ptr->sock , *p_max_fd );
		}
	}
	
	for( task_request_ptr = penv->tasks_request_array , task_response_ptr = penv->tasks_response_array , task_session_ptr = penv->tasks_session_array , task_idx = 0
		; task_idx < penv->tasks_count && penv->query_count_used < penv->qwp._nodes_count && penv->running_count < penv->workers_count && penv->interrupted_flag == 0
		; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_idx++ )
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
			nret = SyncConnectSocket( penv->qwp.nodes[penv->query_count_used].node.ip , penv->qwp.nodes[penv->query_count_used].node.port , task_session_ptr ) ;
			if( nret )
			{
				WarnLog( __FILE__ , __LINE__ , "SyncConnectSocket failed[%d]" , nret );
				penv->query_count_used++;
				if( penv->query_count_used >= penv->qwp._nodes_count )
					break;
				goto _GOTO_CONNECT;
			}
			
			task_session_ptr->alive_timeout = task_request_ptr->timeout ;
			
			nret = proto_ExecuteProgramRequest( task_session_ptr , task_request_ptr->program_and_params , task_request_ptr->timeout , penv->options , task_request_ptr ) ;
			if( nret )
			{
				WarnLog( __FILE__ , __LINE__ , "proto_ExecuteProgramRequest failed[%d]errno[%d]" , nret , errno );
				CloseSocket( task_session_ptr );
				task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
				continue;
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
			InfoLog( __FILE__ , __LINE__ , "FDSET CONN task_idx[%d] FD_SET[%d] max_fd[%d]" , task_idx , task_session_ptr->sock , *p_max_fd );
			penv->query_count_used++;
			PREPARE_COUNT_DECREASE
			RUNNING_COUNT_INCREASE
			task_session_ptr->progress = WSERVER_SESSION_PROGRESS_EXECUTING ;
		}
	}
	
	if( (*p_max_fd) == -1 )
		return DC4C_INFO_NO_RUNNING;
	else
		return 0;
}

int DC4CSelectBatchTasksFds( fd_set *p_read_fds , int *p_max_fd , int select_timeout )
{
	struct timeval		tv ;
	int			select_return_count ;
	
	DebugLog( __FILE__ , __LINE__ , "select ..." );
	tv.tv_sec = select_timeout ;
	tv.tv_usec = 0 ;
	select_return_count = select( (*p_max_fd)+1 , p_read_fds , NULL , NULL , & tv ) ;
	DebugLog( __FILE__ , __LINE__ , "select return[%d]" , select_return_count );
	
	return select_return_count;
}

int DC4CProcessBatchTasks( struct Dc4cApiEnv *penv , fd_set *p_read_fds , int *p_max_fd )
{
	execute_program_request		*task_request_ptr = NULL ;
	execute_program_response	*task_response_ptr = NULL ;
	struct SocketSession		*task_session_ptr = NULL ;
	int				task_idx ;
	time_t				tt ;
	
	deploy_program_request		dpq ;
	
	int				nret = 0 ;
	
	time( & tt );
	
	for( task_request_ptr = penv->tasks_request_array , task_response_ptr = penv->tasks_response_array , task_session_ptr = penv->tasks_session_array , task_idx = 0
		; task_idx < penv->tasks_count
		; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_idx++ )
	{
		if( FD_ISSET( task_session_ptr->sock , p_read_fds ) && IsSocketEstablished( task_session_ptr ) && task_session_ptr->progress == WSERVER_SESSION_PROGRESS_EXECUTING )
		{
			InfoLog( __FILE__ , __LINE__ , "FDSET READY task_idx[%d] FD_SET[%d]" , task_idx , task_session_ptr->sock );
			
			task_session_ptr->alive_timeout -= tt - task_session_ptr->active_timestamp ;
			if( task_session_ptr->alive_timeout <= 0 )
			{
				ErrorLog( __FILE__ , __LINE__ , "task session timeout" );
				CloseSocket( task_session_ptr );
				RUNNING_COUNT_DECREASE
				FINISHED_COUNT_INCREASE
				task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED ;
				task_response_ptr->status = (DC4C_ERROR_TIMEOUT<<8) ;
				continue;
			}
			
			nret = SyncReceiveSocketData( task_session_ptr , & (task_session_ptr->alive_timeout) ) ; 
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
				CloseSocket( task_session_ptr );
				RUNNING_COUNT_DECREASE
				PREPARE_COUNT_INCREASE
				task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
				continue;
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
					CloseSocket(task_session_ptr );
					RUNNING_COUNT_DECREASE
					PREPARE_COUNT_INCREASE
					task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
					continue;
				}
				else
				{
					InfoLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest ok" );
				}
				
				nret = proto_DeployProgramResponse( task_session_ptr , dpq.program ) ;
				if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest failed[%d]errno[%d]" , nret , errno );
					CloseSocket( task_session_ptr );
					RUNNING_COUNT_DECREASE
					PREPARE_COUNT_INCREASE
					task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
					continue;
				}
				else
				{
					InfoLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest ok" );
				}
				
				nret = SyncSendSocketData( task_session_ptr , & (task_session_ptr->alive_timeout) ) ; 
				if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
					CloseSocket( task_session_ptr );
					RUNNING_COUNT_DECREASE
					PREPARE_COUNT_INCREASE
					task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
					continue;
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
					CloseSocket( task_session_ptr );
					RUNNING_COUNT_DECREASE
					PREPARE_COUNT_INCREASE
					task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
					continue;
				}
				else
				{
					InfoLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse ok" );
				}
				
				if( WEXITSTATUS(task_response_ptr->status) == DC4C_INFO_ALREADY_EXECUTING )
				{
					DebugLog( __FILE__ , __LINE__ , "ejected by worker" );
					CloseSocket( task_session_ptr );
					RUNNING_COUNT_DECREASE
					PREPARE_COUNT_INCREASE
					task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
				}
				else
				{
					InfoLog( __FILE__ , __LINE__ , "FDSET CLOSE task_idx[%d] FD_SET[%d]" , task_idx , task_session_ptr->sock );
					CloseSocket( task_session_ptr );
					if( task_response_ptr->status && TestAttribute( penv->options , DC4C_OPTIONS_INTERRUPT_BY_APP ) )
					{
						ErrorLog( __FILE__ , __LINE__ , "task_idx[%d] status[%d] WEXITSTATUS(status)[%d]" , task_idx , task_response_ptr->status , WEXITSTATUS(task_response_ptr->status) );
						PREPARE_COUNT_INCREASE
						RUNNING_COUNT_DECREASE
						task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR ;
						penv->interrupted_flag = 1 ;
					}
					else
					{
						InfoLog( __FILE__ , __LINE__ , "task_idx[%d] status[%d] WEXITSTATUS(status)[%d]" , task_idx , task_response_ptr->status , WEXITSTATUS(task_response_ptr->status) );
						RUNNING_COUNT_DECREASE
						FINISHED_COUNT_INCREASE
						task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED ;
					}
				}
			}
		}
	}
	
	for( task_request_ptr = penv->tasks_request_array , task_response_ptr = penv->tasks_response_array , task_session_ptr = penv->tasks_session_array , task_idx = 0
		; task_idx < penv->tasks_count
		; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_idx++ )
	{
		if( IsSocketEstablished( task_session_ptr ) )
		{
			task_session_ptr->alive_timeout -= tt - task_session_ptr->active_timestamp ;
			time( & (task_session_ptr->active_timestamp) );
			if( task_session_ptr->alive_timeout <= 0 )
			{
				ErrorLog( __FILE__ , __LINE__ , "task session timeout" );
				CloseSocket( task_session_ptr );
				RUNNING_COUNT_DECREASE
				FINISHED_COUNT_INCREASE
				task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED ;
				task_response_ptr->error = DC4C_ERROR_TIMEOUT ;
			}
		}
	}
	
	if( penv->prepare_count == 0 && penv->running_count == 0 )
		return DC4C_INFO_NO_PREPARE_AND_RUNNING;
	else
		return 0;
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

int DC4CGetRunningTasksCount( struct Dc4cApiEnv *penv )
{
	return penv->running_count;
}

int DC4CGetFinishedTasksCount( struct Dc4cApiEnv *penv )
{
	return penv->finished_count;
}

int DC4CGetBatchTasksIp( struct Dc4cApiEnv *penv , int index , char **pp_ip )
{
	if( index > penv->tasks_array_size )
		return -1;
	
	(*pp_ip) = penv->tasks_request_array[index].ip ;
	return 0;
}

int DC4CGetBatchTasksPort( struct Dc4cApiEnv *penv , int index , long *p_port )
{
	if( index > penv->tasks_array_size )
		return -1;
	
	(*p_port) = penv->tasks_request_array[index].port ;
	return 0;
}

int DC4CGetBatchTasksTid( struct Dc4cApiEnv *penv , int index , char **pp_tid )
{
	if( index > penv->tasks_array_size )
		return -1;
	
	(*pp_tid) = penv->tasks_request_array[index].tid ;
	return 0;
}

int DC4CGetBatchTasksProgramAndParam( struct Dc4cApiEnv *penv , int index , char **pp_program_and_params )
{
	if( index > penv->tasks_array_size )
		return -1;
	
	(*pp_program_and_params) = penv->tasks_request_array[index].program_and_params ;
	return 0;
}

int DC4CGetBatchTasksTimeout( struct Dc4cApiEnv *penv , int index , int *p_timeout )
{
	if( index > penv->tasks_array_size )
		return -1;
	
	(*p_timeout) = penv->tasks_request_array[index].timeout ;
	return 0;
}

int DC4CGetBatchTasksElapse( struct Dc4cApiEnv *penv , int index , int *p_elapse )
{
	if( index > penv->tasks_array_size )
		return -1;
	
	(*p_elapse) = penv->tasks_response_array[index].elapse ;
	return 0;
}

int DC4CGetBatchTasksError( struct Dc4cApiEnv *penv , int index , int *p_error )
{
	if( index > penv->tasks_array_size )
		return -1;
	
	(*p_error) = penv->tasks_response_array[index].error ;
	return 0;
}

int DC4CGetBatchTasksStatus( struct Dc4cApiEnv *penv , int index , int *p_status )
{
	if( index > penv->tasks_array_size )
		return -1;
	
	(*p_status) = penv->tasks_response_array[index].status ;
	return 0;
}

int DC4CGetBatchTasksInfo( struct Dc4cApiEnv *penv , int index , char **pp_info )
{
	if( index > penv->tasks_array_size )
		return -1;
	
	(*pp_info) = penv->tasks_response_array[index].info ;
	return 0;
}

void DC4CResetFinishedTasksWithError( struct Dc4cApiEnv *penv )
{
	struct SocketSession	*task_session_ptr = NULL ;
	int			task_idx ;
	
	for( task_session_ptr = penv->tasks_session_array , task_idx = 0
		; task_idx < penv->tasks_count
		; task_session_ptr++ , task_idx++ )
	{
		if( task_session_ptr->progress == WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR )
			task_session_ptr->progress = WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING ;
	}
	
	return;
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

int DC4CGetUnusedWorkersCount( struct Dc4cApiEnv *penv )
{
	return penv->qwp._nodes_count - penv->query_count_used ;
}


/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 * LastVersion	: v1.0.0
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

char __DC4C_API_VERSION_1_0_0[] = "1.0.0" ;
char *__DC4C_API_VERSION = __DC4C_API_VERSION_1_0_0 ;

#define RSERVER_ARRAYSIZE				2

#define WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING	0
#define WSERVER_SESSION_PROGRESS_EXECUTING		1
#define WSERVER_SESSION_PROGRESS_FINISHED		2

#define SELECT_TIMEOUT					10

int proto_QueryWorkersRequest( struct SocketSession *psession , int want_count );
int proto_QueryWorkersResponse( struct SocketSession *psession , query_workers_response *p_rsp );
int proto_ExecuteProgramRequest( struct SocketSession *psession , char *program_and_params , execute_program_request *p_req );
int proto_ExecuteProgramResponse( struct SocketSession *psession , execute_program_response *p_rsp );
int proto_DeployProgramRequest( struct SocketSession *psession , deploy_program_request *p_rsp );
int proto_DeployProgramResponse( struct SocketSession *psession , char *program );

struct Dc4cApiEnv
{
	char				rserver_ip[ RSERVER_ARRAYSIZE ][ 40 + 1 ] ;
	int				rserver_port[ RSERVER_ARRAYSIZE ] ;
	int				rserver_count ;
	int				rserver_index ;
	struct SocketSession		rserver_session ;
	
	int				program_and_params_count ;
	int				worker_count ;
	
	struct SocketSession		*task_session_array ;
	execute_program_request		*task_request_array ;
	execute_program_response	*task_response_array ;
	int				task_array_size ;
	
	long				timeout ;
	
	int				prepare_count ;
	int				running_count ;
	int				finished_count ;
} ;

static int PerformTasksArray( struct Dc4cApiEnv *penv , int program_and_params_count )
{
	int			i ;
	
	int			nret = 0 ;
	
	if( program_and_params_count <= penv->task_array_size )
	{
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			DSCINIT_execute_program_request( & (penv->task_request_array[i]) );
		}
		
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			DSCINIT_execute_program_response( & (penv->task_response_array[i]) );
		}
		
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			CloseSocket( & (penv->task_session_array[i]) );
		}
	}
	else
	{
		struct SocketSession		*task_session_array ;
		execute_program_request		*task_request_array ;
		execute_program_response	*task_response_array ;
		
		task_request_array = (execute_program_request *)realloc( penv->task_request_array , sizeof(execute_program_request) * program_and_params_count ) ;
		if( task_request_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			return DC4C_ERROR_ALLOC;
		}
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			DSCINIT_execute_program_request( & (task_request_array[i]) );
		}
		
		task_response_array = (execute_program_response *)realloc( penv->task_response_array , sizeof(execute_program_response) * program_and_params_count ) ;
		if( task_response_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			return DC4C_ERROR_ALLOC;
		}
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			DSCINIT_execute_program_response( & (task_response_array[i]) );
		}
		
		task_session_array = (struct SocketSession *)realloc( penv->task_session_array , sizeof(struct SocketSession) * program_and_params_count ) ;
		if( task_session_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			free( task_request_array );
			return DC4C_ERROR_ALLOC;
		}
		
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			nret = InitSocketSession( & (task_session_array[i]) ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
				for( --i ; i >= 0 ; i-- )
				{
					CleanSocketSession( & (task_session_array[i]) );
				}
				free( task_request_array );
				free( task_response_array );
				return DC4C_ERROR_ALLOC;
			}
		}
		
		penv->task_request_array = task_request_array ;
		penv->task_response_array = task_response_array ;
		penv->task_session_array = task_session_array ;
		
		penv->task_array_size = program_and_params_count ;
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
	
	if( (*ppenv)->task_request_array )
	{
		free( (*ppenv)->task_request_array );
	}
	
	if( (*ppenv)->task_response_array )
	{
		free( (*ppenv)->task_response_array );
	}
	
	if( (*ppenv)->task_session_array )
	{
		for( i = 0 ; i < (*ppenv)->task_array_size ; i++ )
		{
			CloseSocket( & ((*ppenv)->task_session_array[i]) );
		}
		
		free( (*ppenv)->task_session_array );
	}
	
	free( (*ppenv) );
	
	return 0;
}

int DC4CInitEnv( struct Dc4cApiEnv **ppenv , char *rserver_ip_port )
{
	char		buf[ 1024 + 1 ] ;
	char		*p = NULL ;
	int		c ;
	
	int		nret = 0 ;
	
	SetLogFile( "%s/log/dc4c_api.log" , getenv("HOME") );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	InfoLog( __FILE__ , __LINE__ , "dc4c_api v%s ( util v%s )" , __DC4C_API_VERSION , __DC4C_UTIL_VERSION );
	
	(*ppenv) = (struct Dc4cApiEnv *)malloc( sizeof(struct Dc4cApiEnv) ) ;
	if( (*ppenv) == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return DC4C_ERROR_ALLOC;
	}
	memset( (*ppenv) , 0x00 , sizeof(struct Dc4cApiEnv) );
	
	strcpy( buf , rserver_ip_port );
	p = strtok( buf , "," ) ;
	if( p == NULL )
		p = buf ;
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
	
#if 0
	(*ppenv)->task_request_array = (execute_program_request *)malloc( sizeof(execute_program_request) ) ;
	if( (*ppenv)->task_request_array == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_ALLOC;
	}
	DSCINIT_execute_program_request( & ((*ppenv)->task_request_array[0]) );
	
	(*ppenv)->task_response_array = (execute_program_response *)malloc( sizeof(execute_program_response) ) ;
	if( (*ppenv)->task_response_array == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_ALLOC;
	}
	DSCINIT_execute_program_response( & ((*ppenv)->task_response_array[0]) );
	
	(*ppenv)->task_session_array = (struct SocketSession *)malloc( sizeof(struct SocketSession) ) ;
	if( (*ppenv)->task_session_array == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_ALLOC;
	}
	memset( (*ppenv)->task_session_array , 0x00 , sizeof(struct SocketSession) );
	
	nret = InitSocketSession( & ((*ppenv)->task_session_array[0]) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_ALLOC;
	}
	
	(*ppenv)->task_array_size = 1 ;
#endif
	
	for( c = 0 ; c < (*ppenv)->rserver_count ; c++ )
	{
		nret = SyncConnectSocket( (*ppenv)->rserver_ip[(*ppenv)->rserver_index] , (*ppenv)->rserver_port[(*ppenv)->rserver_index] , & ((*ppenv)->rserver_session) ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] failed[%d]errno[%d]" , (*ppenv)->rserver_ip[(*ppenv)->rserver_index] , (*ppenv)->rserver_port[(*ppenv)->rserver_index] , nret , errno );
			(*ppenv)->rserver_index = ((*ppenv)->rserver_index+1) % (*ppenv)->rserver_count ;
			continue;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] ok" , (*ppenv)->rserver_ip[(*ppenv)->rserver_index] , (*ppenv)->rserver_port[(*ppenv)->rserver_index] );
			break;
		}
	}
	if( c >= (*ppenv)->rserver_count )
	{
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_CONNECT;
	}
	
	return 0;
}

void DC4CSetTimeout( struct Dc4cApiEnv *penv , long timeout )
{
	penv->timeout = timeout ;
	return;
}

int DC4CDoTask( struct Dc4cApiEnv *penv , char *program_and_params )
{
	return DC4CDoBatchTasks( penv , 1 , & program_and_params , 1 );
}

int DC4CGetTaskProgramAndParam( struct Dc4cApiEnv *penv , char **pp_program_and_params )
{
	return DC4CGetBatchTasksProgramAndParam( penv , 1 , pp_program_and_params );
}

int DC4CGetTaskTid( struct Dc4cApiEnv *penv , char **pp_tid )
{
	return DC4CGetBatchTasksTid( penv , 1 , pp_tid );
}

int DC4CGetTaskIp( struct Dc4cApiEnv *penv , char **pp_ip )
{
	return DC4CGetBatchTasksIp( penv , 1 , pp_ip );
}

int DC4CGetTaskPort( struct Dc4cApiEnv *penv , long *p_port )
{
	return DC4CGetBatchTasksPort( penv , 1 , p_port );
}

int DC4CGetTaskResponseCode( struct Dc4cApiEnv *penv , int *p_response_code )
{
	return DC4CGetBatchTasksResponseCode( penv , 1 , p_response_code );
}

int DC4CGetTaskStatus( struct Dc4cApiEnv *penv , int *p_status )
{
	return DC4CGetBatchTasksStatus( penv , 1 , p_status );
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

int DC4CDoBatchTasks( struct Dc4cApiEnv *penv , int worker_count , char **program_and_params_array , int program_and_params_count )
{
	int		nret = 0 ;
	
	nret = DC4CBeginBatchTasks( penv , worker_count , program_and_params_array , program_and_params_count ) ;
	if( nret )
		return nret;
	
	while(1)
	{
		nret = DC4CPerformBatchTasks( penv ) ;
		if( nret == DC4C_INFO_NO_RUNNING )
		{
			sleep(1);
		}
		else if( nret == DC4C_INFO_NO_PREPARE_AND_RUNNING )
		{
			break;
		}
		else if( nret )
		{
			return nret;
		}
	}
	
	return 0;
}

int DC4CBeginBatchTasks( struct Dc4cApiEnv *penv , int worker_count , char **program_and_params_array , int program_and_params_count )
{
	int				i ;
	int				c ;
	
	int				nret = 0 ;
	
	InfoLog( __FILE__ , __LINE__ , "worker_count[%d] program_and_params_count[%d]" , worker_count , program_and_params_count );
	
	penv->program_and_params_count = program_and_params_count ;
	
	nret = PerformTasksArray( penv , penv->program_and_params_count ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "PerformTasksArray failed[%d] , errno[%d]" , nret , errno );
		return nret;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "PerformTasksArray ok" );
	}
	
	for( i = 0 ; i < penv->program_and_params_count ; i++ )
	{
		strcpy( penv->task_request_array[i].program_and_params , program_and_params_array[i] );
		InfoLog( __FILE__ , __LINE__ , "TASK[%d] [%s]" , i , penv->task_request_array[i].program_and_params );
	}
	
	if( worker_count == DC4C_WORKER_COUNT_UNLIMITED || worker_count > penv->program_and_params_count )
	{
		penv->worker_count = penv->program_and_params_count ;
	}
	else
	{
		penv->worker_count = worker_count ;
	}
	
	penv->prepare_count = penv->program_and_params_count ;
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
	
	return 0;
}

int DC4CSetBatchTasksFds( struct Dc4cApiEnv *penv , fd_set *p_read_fds , fd_set *p_write_fds , fd_set *p_expect_fds , int *p_max_fd )
{
	execute_program_request		*task_request_ptr = NULL ;
	execute_program_response	*task_response_ptr = NULL ;
	struct SocketSession		*task_session_ptr = NULL ;
	int				task_idx ;
	
	for( task_request_ptr = penv->task_request_array , task_response_ptr = penv->task_response_array , task_session_ptr = penv->task_session_array , task_idx = 0
		; task_idx < penv->program_and_params_count
		; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_idx++ )
	{
		if( task_session_ptr->progress == WSERVER_SESSION_PROGRESS_EXECUTING )
		{
			FD_SET( task_session_ptr->sock , p_read_fds );
			if( task_session_ptr->sock > (*p_max_fd) )
				(*p_max_fd) = task_session_ptr->sock ;
			InfoLog( __FILE__ , __LINE__ , "ESTAB task_idx[%d] FD_SET[%d] max_fd[%d]" , task_idx , task_session_ptr->sock , (*p_max_fd) );
		}
	}
	
	return 0;
}

int DC4CPerformBatchTasks( struct Dc4cApiEnv *penv )
{
	int				query_count ;
	int				query_count_used = 0 ;
	execute_program_request		*task_request_ptr = NULL ;
	execute_program_response	*task_response_ptr = NULL ;
	struct SocketSession		*task_session_ptr = NULL ;
	int				task_idx ;
	fd_set				read_fds ;
	int				max_fd ;
	time_t				tt ;
	struct timeval			tv ;
	int				select_return_count = 0 ;
	
	query_workers_response		qwp ;
	deploy_program_request		dpq ;
	
	int				nret = 0 ;
	
	query_count = penv->prepare_count ;
	if( query_count > penv->worker_count )
		query_count = penv->worker_count ;
	
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
			return nret;
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
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
		}
		
		if( STRNCMP( penv->rserver_session.recv_buffer + LEN_COMMHEAD , != , "QWP" , 3 ) )
		{
			ErrorLog( __FILE__ , __LINE__ , "invalid msghead.msgtype[%.*s]" , 3 , penv->rserver_session.recv_buffer + LEN_COMMHEAD );
			CloseSocket( & (penv->rserver_session) );
			return nret;
		}
		
		nret = proto_QueryWorkersResponse( & (penv->rserver_session) , & qwp ) ;
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
		
		if( qwp.response_code == 5 || qwp._nodes_count == 0 )
		{
			InfoLog( __FILE__ , __LINE__ , "no worker" );
		}
		else if( qwp.response_code )
		{
			ErrorLog( __FILE__ , __LINE__ , "Query workers failed[%d]" , qwp.response_code );
			return qwp.response_code;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "Query [%d]workers" , qwp._nodes_count );
		}
		
		query_count_used = 0 ;
	}
	
	FD_ZERO( & read_fds );
	max_fd = -1 ;
	
	for( task_request_ptr = penv->task_request_array , task_response_ptr = penv->task_response_array , task_session_ptr = penv->task_session_array , task_idx = 0
		; task_idx < penv->program_and_params_count
		; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_idx++ )
	{
		if( task_session_ptr->progress == WSERVER_SESSION_PROGRESS_EXECUTING )
		{
			FD_SET( task_session_ptr->sock , & read_fds );
			if( task_session_ptr->sock > max_fd )
				max_fd = task_session_ptr->sock ;
			InfoLog( __FILE__ , __LINE__ , "FDSET ESTAB task_idx[%d] FD_SET[%d] max_fd[%d]" , task_idx , task_session_ptr->sock , max_fd );
		}
	}
	
	for( task_request_ptr = penv->task_request_array , task_response_ptr = penv->task_response_array , task_session_ptr = penv->task_session_array , task_idx = 0
		; task_idx < penv->program_and_params_count && query_count_used < qwp._nodes_count && penv->running_count < penv->worker_count
		; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_idx++ )
	{
		if( task_session_ptr->progress == WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING )
		{
_GOTO_CONNECT :
			nret = SyncConnectSocket( qwp.nodes[query_count_used].node.ip , qwp.nodes[query_count_used].node.port , task_session_ptr ) ;
			if( nret )
			{
				WarnLog( __FILE__ , __LINE__ , "SyncConnectSocket failed[%d]" , nret );
				query_count_used++;
				if( query_count_used >= qwp._nodes_count )
					break;
				goto _GOTO_CONNECT;
			}
			
			task_session_ptr->alive_timeout = penv->timeout ;
			
			nret = proto_ExecuteProgramRequest( task_session_ptr , task_request_ptr->program_and_params , task_request_ptr ) ;
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
			
			FD_SET( task_session_ptr->sock , & read_fds );
			if( task_session_ptr->sock > max_fd )
				max_fd = task_session_ptr->sock ;
			InfoLog( __FILE__ , __LINE__ , "FDSET CONN task_idx[%d] FD_SET[%d] max_fd[%d]" , task_idx , task_session_ptr->sock , max_fd );
			query_count_used++;
			PREPARE_COUNT_DECREASE
			RUNNING_COUNT_INCREASE
			task_session_ptr->progress = WSERVER_SESSION_PROGRESS_EXECUTING ;
		}
	}
	
	DebugLog( __FILE__ , __LINE__ , "select ..." );
	tv.tv_sec = 1 ;
	tv.tv_usec = 0 ;
	select_return_count = select( max_fd+1 , & read_fds , NULL , NULL , & tv ) ;
	DebugLog( __FILE__ , __LINE__ , "select return[%d]" , select_return_count );
	
	time( & tt );
	
	if( select_return_count > 0 )
	{
		for( task_request_ptr = penv->task_request_array , task_response_ptr = penv->task_response_array , task_session_ptr = penv->task_session_array , task_idx = 0
			; task_idx < penv->program_and_params_count
			; task_request_ptr++ , task_response_ptr++ , task_session_ptr++ , task_idx++ )
		{
			if( FD_ISSET( task_session_ptr->sock , & read_fds ) && IsSocketEstablished( task_session_ptr ) && task_session_ptr->progress == WSERVER_SESSION_PROGRESS_EXECUTING )
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
					task_response_ptr->status = (DC4C_RETURNSTATUS_TIMEOUT<<8) ;
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
					
					if( WEXITSTATUS(task_response_ptr->status) == DC4C_RETURNSTATUS_ALREADY_EXECUTING )
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
						RUNNING_COUNT_DECREASE
						FINISHED_COUNT_INCREASE
						task_session_ptr->progress = WSERVER_SESSION_PROGRESS_FINISHED ;
					}
				}
			}
		}
	}
	
	for( task_request_ptr = penv->task_request_array , task_response_ptr = penv->task_response_array , task_session_ptr = penv->task_session_array , task_idx = 0
		; task_idx < penv->program_and_params_count
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
				task_response_ptr->response_code = DC4C_RETURNSTATUS_TIMEOUT ;
			}
		}
	}
	
	if( penv->prepare_count == 0 && penv->running_count == 0 )
		return DC4C_INFO_NO_PREPARE_AND_RUNNING;
	else if( max_fd == -1 )
		return DC4C_INFO_NO_RUNNING;
	else
		return 0;
}

int DC4CGetPrepareTaskCount( struct Dc4cApiEnv *penv )
{
	return penv->prepare_count;
}

int DC4CGetRunningTaskCount( struct Dc4cApiEnv *penv )
{
	return penv->running_count;
}

int DC4CGetFinishedTaskCount( struct Dc4cApiEnv *penv )
{
	return penv->finished_count;
}

int DC4CGetBatchTasksProgramAndParam( struct Dc4cApiEnv *penv , int index , char **pp_program_and_params )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*pp_program_and_params) = penv->task_request_array[index-1].program_and_params ;
	return 0;
}

int DC4CGetBatchTasksTid( struct Dc4cApiEnv *penv , int index , char **pp_tid )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*pp_tid) = penv->task_request_array[index-1].tid ;
	return 0;
}

int DC4CGetBatchTasksIp( struct Dc4cApiEnv *penv , int index , char **pp_ip )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*pp_ip) = penv->task_request_array[index-1].ip ;
	return 0;
}

int DC4CGetBatchTasksPort( struct Dc4cApiEnv *penv , int index , long *p_port )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*p_port) = penv->task_request_array[index-1].port ;
	return 0;
}

int DC4CGetBatchTasksResponseCode( struct Dc4cApiEnv *penv , int index , int *p_response_code )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*p_response_code) = penv->task_response_array[index-1].response_code ;
	return 0;
}

int DC4CGetBatchTasksStatus( struct Dc4cApiEnv *penv , int index , int *p_status )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*p_status) = penv->task_response_array[index-1].status ;
	return 0;
}

void DC4CSetAppLogFile()
{
	char	*WSERVER_INDEX = NULL ;
	int	wserver_index ;
	char	*WSERVER_IP_PORT = NULL ;
	
	WSERVER_INDEX = getenv("WSERVER_INDEX") ;
	if( WSERVER_INDEX == NULL )
		wserver_index = 0 ;
	else
		wserver_index = atoi(WSERVER_INDEX) ;
	
	WSERVER_IP_PORT = getenv("WSERVER_IP_PORT") ;
	if( WSERVER_IP_PORT == NULL )
		WSERVER_IP_PORT = "0.0.0.0:0" ;
	
	SetLogFile( "%s/log/wserver_%d_%s.dc4c_test_worker_sleep.log" , getenv("HOME") , wserver_index , WSERVER_IP_PORT );
	return;
}

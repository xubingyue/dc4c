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

char __DC4C_API_VERSION_1_0_0[] = "1.0.0" ;
char *__DC4C_API_VERSION = __DC4C_API_VERSION_1_0_0 ;

#define RSERVER_ARRAYSIZE				8

#define WSERVER_SESSION_PROGRESS_WAITFOR_CONNECTING	0
#define WSERVER_SESSION_PROGRESS_EXECUTING		1
#define WSERVER_SESSION_PROGRESS_FINISHED		2
#define WSERVER_SESSION_PROGRESS_FINISHED_WITH_ERROR	3

#define SELECT_TIMEOUT					10

int proto_QueryWorkersRequest( struct SocketSession *psession , int want_count );
int proto_QueryWorkersResponse( struct SocketSession *psession , query_workers_response *p_rsp );
int proto_ExecuteProgramRequest( struct SocketSession *psession , char *program_and_params , int timeout , execute_program_request *p_req );
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
	
	int				tasks_count ;
	int				workers_count ;
	
	struct SocketSession		*task_session_array ;
	execute_program_request		*task_request_array ;
	execute_program_response	*task_response_array ;
	int				task_array_size ;
	
	int				timeout ;
	
	int				prepare_count ;
	int				running_count ;
	int				finished_count ;
	
	query_workers_response		qwp ;
	int				query_count_used ;
	
	unsigned long			options ;
	int				interrupted_flag ;
} ;

static int ReformingTasksArray( struct Dc4cApiEnv *penv , int tasks_count )
{
	int			i ;
	
	int			nret = 0 ;
	
	if( tasks_count <= penv->task_array_size )
	{
		for( i = 0 ; i < tasks_count ; i++ )
		{
			DSCINIT_execute_program_request( & (penv->task_request_array[i]) );
		}
		
		for( i = 0 ; i < tasks_count ; i++ )
		{
			DSCINIT_execute_program_response( & (penv->task_response_array[i]) );
		}
		
		for( i = 0 ; i < tasks_count ; i++ )
		{
			CloseSocket( & (penv->task_session_array[i]) );
		}
	}
	else
	{
		struct SocketSession		*task_session_array ;
		execute_program_request		*task_request_array ;
		execute_program_response	*task_response_array ;
		
		task_request_array = (execute_program_request *)realloc( penv->task_request_array , sizeof(execute_program_request) * tasks_count ) ;
		if( task_request_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			return DC4C_ERROR_ALLOC;
		}
		for( i = 0 ; i < tasks_count ; i++ )
		{
			DSCINIT_execute_program_request( & (task_request_array[i]) );
		}
		
		task_response_array = (execute_program_response *)realloc( penv->task_response_array , sizeof(execute_program_response) * tasks_count ) ;
		if( task_response_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			return DC4C_ERROR_ALLOC;
		}
		for( i = 0 ; i < tasks_count ; i++ )
		{
			DSCINIT_execute_program_response( & (task_response_array[i]) );
		}
		
		task_session_array = (struct SocketSession *)realloc( penv->task_session_array , sizeof(struct SocketSession) * tasks_count ) ;
		if( task_session_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			free( task_request_array );
			return DC4C_ERROR_ALLOC;
		}
		
		for( i = 0 ; i < tasks_count ; i++ )
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
		
		penv->task_array_size = tasks_count ;
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
			if( IsSocketEstablished( & ((*ppenv)->task_session_array[i]) ) )
			{
				CloseSocket( & ((*ppenv)->task_session_array[i]) );
			}
			CleanSocketSession( & ((*ppenv)->task_session_array[i]) );
		}
		
		free( (*ppenv)->task_session_array );
	}
	
	free( (*ppenv) );
	
	return 0;
}

int DC4CInitEnv( struct Dc4cApiEnv **ppenv , char *rservers_ip_port )
{
	char		buf[ 1024 + 1 ] ;
	char		*p = NULL ;
	
	int		nret = 0 ;
	
	InfoLog( __FILE__ , __LINE__ , "dc4c_api v%s ( util v%s )" , __DC4C_API_VERSION , __DC4C_UTIL_VERSION );
	
	(*ppenv) = (struct Dc4cApiEnv *)malloc( sizeof(struct Dc4cApiEnv) ) ;
	if( (*ppenv) == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return DC4C_ERROR_ALLOC;
	}
	memset( (*ppenv) , 0x00 , sizeof(struct Dc4cApiEnv) );
	
	strcpy( buf , rservers_ip_port );
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
	return DC4CGetBatchTasksIp( penv , 1 , pp_ip );
}

int DC4CGetTaskPort( struct Dc4cApiEnv *penv , long *p_port )
{
	return DC4CGetBatchTasksPort( penv , 1 , p_port );
}

int DC4CGetTaskTid( struct Dc4cApiEnv *penv , char **pp_tid )
{
	return DC4CGetBatchTasksTid( penv , 1 , pp_tid );
}

int DC4CGetTaskProgramAndParam( struct Dc4cApiEnv *penv , char **pp_program_and_params )
{
	return DC4CGetBatchTasksProgramAndParam( penv , 1 , pp_program_and_params );
}

int DC4CGetTaskTimeout( struct Dc4cApiEnv *penv , int *p_timeout )
{
	return DC4CGetBatchTasksTimeout( penv , 1 , p_timeout );
}

int DC4CGetTaskElapse( struct Dc4cApiEnv *penv , int *p_elapse )
{
	return DC4CGetBatchTasksElapse( penv , 1 , p_elapse );
}

int DC4CGetTaskError( struct Dc4cApiEnv *penv , int *p_error )
{
	return DC4CGetBatchTasksError( penv , 1 , p_error );
}

int DC4CGetTaskStatus( struct Dc4cApiEnv *penv , int *p_status )
{
	return DC4CGetBatchTasksStatus( penv , 1 , p_status );
}

int DC4CGetTaskInfo( struct Dc4cApiEnv *penv , char **pp_info )
{
	return DC4CGetBatchTasksInfo( penv , 1 , pp_info );
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
		return nret;
	
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
				return nret;
		}
		
		FD_ZERO( & read_fds );
		max_fd = -1 ;
		nret = DC4CSetBatchTasksFds( penv , & read_fds , & max_fd ) ;
		if( nret == DC4C_INFO_NO_RUNNING )
		{
			;
		}
		else if( nret )
		{
			return nret;
		}
		else
		{
			select_return_count = DC4CSelectBatchTasksFds( & read_fds , & max_fd , 1 ) ;
			if( select_return_count < 0 )
			{
				return select_return_count;
			}
			else if( select_return_count > 0 )
			{
				nret = DC4CProcessBatchTasks( penv , & read_fds , & max_fd ) ;
				if( nret == DC4C_INFO_NO_PREPARE_AND_RUNNING )
				{
					return 0;
				}
				else if( nret )
				{
					return nret;
				}
			}
		}
		
		if( penv->interrupted_flag == 1 && DC4CGetRunningTasksCount(penv) == 0 )
			return DC4C_ERROR_APP;
		
		if( penv->qwp._nodes_count == 0 )
			sleep(1);
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
		
		for( task_session_ptr = penv->task_session_array , task_idx = 0
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
					return nret;
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
				;
			}
			else if( nret )
			{
				(*p_penv) = penv ;
				return nret;
			}
		}
		if( max_fd != -1 )
		{
			select_return_count = DC4CSelectBatchTasksFds( & read_fds , & max_fd , 1 ) ;
			if( select_return_count < 0 )
			{
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
						if( p_penv )
							(*p_penv) = penv ;
						if( p_remain_envs_count )
							(*p_remain_envs_count) = remain_envs_count - 1 ;
						return 0;
					}
					else if( nret )
					{
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
			sleep(1);
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
		strcpy( penv->task_request_array[i].program_and_params , p_task->program_and_params );
		penv->task_request_array[i].timeout = p_task->timeout ;
		if( penv->timeout > penv->task_request_array[i].timeout )
			penv->task_request_array[i].timeout = penv->timeout ;
		InfoLog( __FILE__ , __LINE__ , "TASK[%d] [%s] [%d]" , i , penv->task_request_array[i].program_and_params , penv->task_request_array[i].timeout );
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
	
	for( task_request_ptr = penv->task_request_array , task_response_ptr = penv->task_response_array , task_session_ptr = penv->task_session_array , task_idx = 0
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
	
	for( task_request_ptr = penv->task_request_array , task_response_ptr = penv->task_response_array , task_session_ptr = penv->task_session_array , task_idx = 0
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
			
			nret = proto_ExecuteProgramRequest( task_session_ptr , task_request_ptr->program_and_params , task_request_ptr->timeout , task_request_ptr ) ;
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
	
	for( task_request_ptr = penv->task_request_array , task_response_ptr = penv->task_response_array , task_session_ptr = penv->task_session_array , task_idx = 0
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
	
	for( task_request_ptr = penv->task_request_array , task_response_ptr = penv->task_response_array , task_session_ptr = penv->task_session_array , task_idx = 0
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
	if( index > penv->task_array_size )
		return -1;
	
	(*pp_ip) = penv->task_request_array[index].ip ;
	return 0;
}

int DC4CGetBatchTasksPort( struct Dc4cApiEnv *penv , int index , long *p_port )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*p_port) = penv->task_request_array[index].port ;
	return 0;
}

int DC4CGetBatchTasksTid( struct Dc4cApiEnv *penv , int index , char **pp_tid )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*pp_tid) = penv->task_request_array[index].tid ;
	return 0;
}

int DC4CGetBatchTasksProgramAndParam( struct Dc4cApiEnv *penv , int index , char **pp_program_and_params )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*pp_program_and_params) = penv->task_request_array[index].program_and_params ;
	return 0;
}

int DC4CGetBatchTasksTimeout( struct Dc4cApiEnv *penv , int index , int *p_timeout )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*p_timeout) = penv->task_request_array[index].timeout ;
	return 0;
}

int DC4CGetBatchTasksElapse( struct Dc4cApiEnv *penv , int index , int *p_elapse )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*p_elapse) = penv->task_response_array[index].elapse ;
	return 0;
}

int DC4CGetBatchTasksError( struct Dc4cApiEnv *penv , int index , int *p_error )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*p_error) = penv->task_response_array[index].error ;
	return 0;
}

int DC4CGetBatchTasksStatus( struct Dc4cApiEnv *penv , int index , int *p_status )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*p_status) = penv->task_response_array[index].status ;
	return 0;
}

int DC4CGetBatchTasksInfo( struct Dc4cApiEnv *penv , int index , char **pp_info )
{
	if( index > penv->task_array_size )
		return -1;
	
	(*pp_info) = penv->task_response_array[index].info ;
	return 0;
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
	
	InfoLog( __FILE__ , __LINE__ , "%s ( api v%s )" , program , __DC4C_API_VERSION );
	
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


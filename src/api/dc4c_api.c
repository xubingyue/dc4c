#include "IDL_query_workers_request.dsc.h"
#include "IDL_query_workers_response.dsc.h"
#include "IDL_execute_program_request.dsc.h"
#include "IDL_execute_program_response.dsc.h"
#include "IDL_deploy_program_request.dsc.h"

#include "dc4c_util.h"
#include "dc4c_api.h"

int proto_QueryWorkersRequest( struct SocketSession *psession , int want_count );
int proto_QueryWorkersResponse( struct SocketSession *psession , query_workers_response *p_rsp );
int proto_ExecuteProgramRequest( struct SocketSession *psession , char *program_and_params , execute_program_request *p_req );
int proto_ExecuteProgramResponse( struct SocketSession *psession , execute_program_response *p_rsp );
int proto_DeployProgramRequest( struct SocketSession *psession , deploy_program_request *p_rsp );
int proto_DeployProgramResponse( struct SocketSession *psession , char *program );

struct Dc4cApiEnv
{
	char				rserver_ip[ 40 + 1 ] ;
	int				rserver_port ;
	
	struct SocketSession		rserver_session ;
	
	struct SocketSession		*wserver_session_array ;
	execute_program_request		*wserver_request_array ;
	execute_program_response	*wserver_response_array ;
	int				wserver_array_size ;
	
	long				timeout ;
} ;

int DC4CCleanEnv( struct Dc4cApiEnv **ppenv )
{
	if( IsSocketEstablished( & ((*ppenv)->rserver_session) ) == 1 )
	{
		CloseSocket( & ((*ppenv)->rserver_session) );
	}
	CleanSocketSession( & ((*ppenv)->rserver_session) );
	
	if( (*ppenv)->wserver_response_array )
	{
		free( (*ppenv)->wserver_response_array );
	}
	
	if( (*ppenv)->wserver_session_array )
	{
		free( (*ppenv)->wserver_session_array );
	}
	
	if( (*ppenv)->wserver_request_array )
	{
		free( (*ppenv)->wserver_request_array );
	}
	
	free( (*ppenv) );
	
	return 0;
}

int DC4CInitEnv( struct Dc4cApiEnv **ppenv , char *rserver_ip , int rserver_port )
{
	int		nret = 0 ;
	
	SetLogFile( "%s/log/dc4c_api.log" , getenv("HOME") );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	(*ppenv) = (struct Dc4cApiEnv *)malloc( sizeof(struct Dc4cApiEnv) ) ;
	if( (*ppenv) == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return DC4C_ERROR_ALLOC;
	}
	memset( (*ppenv) , 0x00 , sizeof(struct Dc4cApiEnv) );
	
	nret = InitSocketSession( & ((*ppenv)->rserver_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_ALLOC;
	}
	
	(*ppenv)->wserver_request_array = (execute_program_request *)malloc( sizeof(execute_program_request) ) ;
	if( (*ppenv)->wserver_request_array == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_ALLOC;
	}
	DSCINIT_execute_program_request( & ((*ppenv)->wserver_request_array[0]) );
	
	(*ppenv)->wserver_response_array = (execute_program_response *)malloc( sizeof(execute_program_response) ) ;
	if( (*ppenv)->wserver_response_array == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_ALLOC;
	}
	DSCINIT_execute_program_response( & ((*ppenv)->wserver_response_array[0]) );
	
	(*ppenv)->wserver_session_array = (struct SocketSession *)malloc( sizeof(struct SocketSession) ) ;
	if( (*ppenv)->wserver_session_array == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_ALLOC;
	}
	memset( (*ppenv)->wserver_session_array , 0x00 , sizeof(struct SocketSession) );
	
	nret = InitSocketSession( & ((*ppenv)->wserver_session_array[0]) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_ALLOC;
	}
	
	(*ppenv)->wserver_array_size = 1 ;
	
	strcpy( (*ppenv)->rserver_ip , rserver_ip );
	(*ppenv)->rserver_port = rserver_port ;
	nret = SyncConnectSocket( (*ppenv)->rserver_ip , (*ppenv)->rserver_port , & ((*ppenv)->rserver_session) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] failed[%d]errno[%d]" , (*ppenv)->rserver_ip , (*ppenv)->rserver_port , nret , errno );
		DC4CCleanEnv( ppenv );
		return DC4C_ERROR_CONNECT;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] ok" , (*ppenv)->rserver_ip , (*ppenv)->rserver_port );
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
	query_workers_response	qwp ;
	deploy_program_request	dpq ;
	long			timeout ;
	
	int			nret = 0 ;
	
	if( IsSocketEstablished( & (penv->rserver_session) ) == 0 )
	{
		nret = SyncConnectSocket( penv->rserver_ip , penv->rserver_port , & (penv->rserver_session) ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] failed[%d]errno[%d]" , penv->rserver_ip , penv->rserver_port , nret , errno );
			return DC4C_ERROR_CONNECT;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] ok" , penv->rserver_ip , penv->rserver_port );
		}
	}
	
_GOTO_QUERY_WORKER :
	
	timeout = penv->timeout ;
	
	nret = proto_QueryWorkersRequest( & (penv->rserver_session) , 1 ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_QueryWorkersRequest failed[%d]errno[%d]" , nret , errno );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "proto_QueryWorkersRequest ok" );
	}
	
	nret = SyncSendSocketData( & (penv->rserver_session) , & timeout ) ; 
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
	
	nret = SyncReceiveSocketData( & (penv->rserver_session) , & timeout ) ; 
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
	
	if( qwp.response_code == 5 || qwp.count == 0 || qwp._nodes_count == 0 )
	{
		InfoLog( __FILE__ , __LINE__ , "no worker" );
		sleep(1);
		goto _GOTO_QUERY_WORKER;
	}
	else if( qwp.response_code )
	{
		ErrorLog( __FILE__ , __LINE__ , "Query workers failed[%d]" , qwp.response_code );
		return qwp.response_code;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "Query [%d]workers" , qwp.count );
	}
	
	timeout = penv->timeout ;
	
	nret = SyncConnectSocket( qwp.nodes[0].node.ip , qwp.nodes[0].node.port , & (penv->wserver_session_array[0]) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] failed[%d]errno[%d]" , qwp.nodes[0].node.ip , qwp.nodes[0].node.port , nret , errno );
		return DC4C_ERROR_CONNECT;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] ok" , qwp.nodes[0].node.ip , qwp.nodes[0].node.port );
	}
	
	nret = proto_ExecuteProgramRequest( & (penv->wserver_session_array[0]) , program_and_params , & (penv->wserver_request_array[0]) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_ExecuteProgramRequest failed[%d]errno[%d]" , nret , errno );
		CloseSocket( & (penv->wserver_session_array[0]) );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "proto_ExecuteProgramRequest ok" );
	}
	
	nret = SyncSendSocketData( & (penv->wserver_session_array[0]) , & timeout ) ; 
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
		CloseSocket( & (penv->wserver_session_array[0]) );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
	}
	
	nret = SyncReceiveSocketData( & (penv->wserver_session_array[0]) , & timeout ) ; 
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
		CloseSocket( & (penv->wserver_session_array[0]) );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
	}
	
	if( STRNCMP( penv->wserver_session_array[0].recv_buffer + LEN_COMMHEAD , == , "DPQ" , 3 ) )
	{
		nret = proto_DeployProgramRequest( & (penv->wserver_session_array[0]) , & dpq ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest failed[%d]errno[%d]" , nret , errno );
			CloseSocket( & (penv->wserver_session_array[0]) );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest ok" );
		}
		
		nret = proto_DeployProgramResponse( & (penv->wserver_session_array[0]) , dpq.program ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest failed[%d]errno[%d]" , nret , errno );
			CloseSocket( & (penv->wserver_session_array[0]) );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest ok" );
		}
		
		nret = SyncSendSocketData( & (penv->wserver_session_array[0]) , & timeout ) ; 
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
			CloseSocket( & (penv->wserver_session_array[0]) );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
		}
		
		nret = SyncReceiveSocketData( & (penv->wserver_session_array[0]) , & timeout ) ; 
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
			CloseSocket( & (penv->wserver_session_array[0]) );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
		}
	}
	
	if( STRNCMP( penv->wserver_session_array[0].recv_buffer + LEN_COMMHEAD , != , "EPP" , 3 ) )
	{
		ErrorLog( __FILE__ , __LINE__ , "invalid response[%.*s]" , penv->wserver_session_array[0].total_recv_len , penv->wserver_session_array[0].recv_buffer );
		CloseSocket( & (penv->wserver_session_array[0]) );
		return -1;
	}
	
	nret = proto_ExecuteProgramResponse( & (penv->wserver_session_array[0]) , & (penv->wserver_response_array[0]) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse failed[%d]errno[%d]" , nret , errno );
		CloseSocket( & (penv->wserver_session_array[0]) );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse ok" );
	}
	
	CloseSocket( & (penv->wserver_session_array[0]) );
	
	return 0;
}

int DC4CGetTaskResponseStatus( struct Dc4cApiEnv *penv , int *status )
{
	if( penv->wserver_array_size >= 1 )
	{
		(*status) = penv->wserver_response_array[0].status ;
		return 0;
	}
	else
	{
		return -1;
	}
}

static int PerformSessionArray( struct Dc4cApiEnv *penv , int program_and_params_count )
{
	int			nret = 0 ;
	
	if( penv->wserver_array_size < program_and_params_count )
	{
		struct SocketSession		*wserver_session_array ;
		execute_program_request		*wserver_request_array ;
		execute_program_response	*wserver_response_array ;
		int				i ;
		
		wserver_request_array = (execute_program_request *)malloc( sizeof(execute_program_request) * program_and_params_count ) ;
		if( wserver_request_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			return DC4C_ERROR_ALLOC;
		}
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			DSCINIT_execute_program_request( & (wserver_request_array[i]) );
		}
		
		wserver_response_array = (execute_program_response *)malloc( sizeof(execute_program_response) * program_and_params_count ) ;
		if( wserver_response_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			return DC4C_ERROR_ALLOC;
		}
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			DSCINIT_execute_program_response( & (wserver_response_array[i]) );
		}
		
		wserver_session_array = (struct SocketSession *)malloc( sizeof(struct SocketSession) * program_and_params_count ) ;
		if( wserver_session_array == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			free( wserver_request_array );
			return DC4C_ERROR_ALLOC;
		}
		memset( wserver_session_array , 0x00 , sizeof(struct SocketSession) * program_and_params_count );
		
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			nret = InitSocketSession( & (wserver_session_array[i]) ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "InitSocketSession failed[%d]errno[%d]" , nret , errno );
				for( i-- ; i >= 0 ; i-- )
				{
					CleanSocketSession( & (wserver_session_array[i]) );
				}
				free( wserver_request_array );
				free( wserver_response_array );
				return DC4C_ERROR_ALLOC;
			}
		}
		
		free( penv->wserver_request_array );
		free( penv->wserver_response_array );
		for( i = 0 ; i < penv->wserver_array_size ; i++ )
			CleanSocketSession( & (penv->wserver_session_array[i]) );
		
		penv->wserver_request_array = wserver_request_array ;
		penv->wserver_response_array = wserver_response_array ;
		penv->wserver_session_array = wserver_session_array ;
		
		penv->wserver_array_size = program_and_params_count ;
	}
	else
	{
		int		i ;
		
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			DSCINIT_execute_program_request( & (penv->wserver_request_array[i]) );
		}
		
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			DSCINIT_execute_program_response( & (penv->wserver_response_array[i]) );
		}
		
		for( i = 0 ; i < program_and_params_count ; i++ )
		{
			ResetSocketSession( & (penv->wserver_session_array[i]) );
		}
	}
	
	return 0;
}

int DC4CDoBatchTasks( struct Dc4cApiEnv *penv , int worker_count , char **program_and_params_array , int program_and_params_count )
{
	int				i ;
	
	int				running_count ;
	int				undone_count ;
	int				query_count ;
	int				query_count_used = 0 ;
	execute_program_request		*wserver_request_ptr = NULL ;
	execute_program_response	*wserver_response_ptr = NULL ;
	struct SocketSession		*wserver_session_ptr = NULL ;
	int				wserver_idx ;
	fd_set				read_fds ;
	fd_set				expt_fds ;
	int				max_fd ;
	struct timeval			tv ;
	int				select_return_count ;
	
	long				timeout ;
	query_workers_response		qwp ;
	deploy_program_request		dpq ;
	
	int				nret = 0 ;
	
	InfoLog( __FILE__ , __LINE__ , "worker_count[%d] program_and_params_count[%d]" , worker_count , program_and_params_count );
	
	if( worker_count > program_and_params_count )
	{
		worker_count = program_and_params_count ;
	}
	
	nret = PerformSessionArray( penv , program_and_params_count ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "PerformSessionArray failed[%d] , errno[%d]" , nret , errno );
		return nret;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "PerformSessionArray ok" );
	}
	
	for( i = 0 ; i < program_and_params_count ; i++ )
	{
		strcpy( penv->wserver_request_array[i].program_and_params , program_and_params_array[i] );
		InfoLog( __FILE__ , __LINE__ , "TASK[%d] [%s]" , i , penv->wserver_request_array[i].program_and_params );
	}
	
	if( IsSocketEstablished( & (penv->rserver_session) ) == 0 )
	{
		nret = SyncConnectSocket( penv->rserver_ip , penv->rserver_port , & (penv->rserver_session) ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] failed[%d]errno[%d]" , penv->rserver_ip , penv->rserver_port , nret , errno );
			return DC4C_ERROR_CONNECT;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] ok" , penv->rserver_ip , penv->rserver_port );
		}
	}
	
	running_count = 0 ;
	undone_count = program_and_params_count ;
	while( undone_count > 0 )
	{
		timeout = penv->timeout ;
		
		query_count = undone_count - running_count ;
		query_count = (query_count<worker_count?query_count:worker_count) ;
		if( query_count > 0 )
		{
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
			
			nret = SyncSendSocketData( & (penv->rserver_session) , & timeout ) ; 
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
			
			nret = SyncReceiveSocketData( & (penv->rserver_session) , & timeout ) ; 
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
			
			if( qwp.response_code == 5 || qwp.count == 0 )
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
				InfoLog( __FILE__ , __LINE__ , "Query [%d]workers" , qwp.count );
			}
			
			query_count_used = 0 ;
		}
		
		do
		{
			FD_ZERO( & read_fds );
			FD_ZERO( & expt_fds );
			max_fd = -1 ;
			
			/*
			任务数据特征
			program_and_params[0] != '\0' && wserver_request_ptr->program_md5_exp[0] == '\0' 等待连接
			program_and_params[0] != '\0' && wserver_request_ptr->program_md5_exp[0] != '\0' 已连接
			program_and_params[0] == '\0' && wserver_request_ptr->program_md5_exp[0] != '\0' 已处理
			*/
			
			for( wserver_request_ptr = penv->wserver_request_array , wserver_response_ptr = penv->wserver_response_array , wserver_session_ptr = penv->wserver_session_array , wserver_idx = 0
				; wserver_idx < program_and_params_count
				; wserver_request_ptr++ , wserver_response_ptr++ , wserver_session_ptr++ , wserver_idx++ )
			{
				if( wserver_request_ptr->program_and_params[0] != '\0' && wserver_request_ptr->program_md5_exp[0] != '\0' )
				{
					FD_SET( wserver_session_ptr->sock , & read_fds );
					FD_SET( wserver_session_ptr->sock , & expt_fds );
					if( wserver_session_ptr->sock > max_fd )
						max_fd = wserver_session_ptr->sock ;
					InfoLog( __FILE__ , __LINE__ , "ESTAB wserver_idx[%d] FD_SET[%d] max_fd[%d]" , wserver_idx , wserver_session_ptr->sock , max_fd );
				}
			}
			
			for( wserver_request_ptr = penv->wserver_request_array , wserver_response_ptr = penv->wserver_response_array , wserver_session_ptr = penv->wserver_session_array , wserver_idx = 0
				; wserver_idx < program_and_params_count && query_count_used < qwp.count && running_count < worker_count
				; wserver_request_ptr++ , wserver_response_ptr++ , wserver_session_ptr++ , wserver_idx++ )
			{
				if( wserver_request_ptr->program_and_params[0] != '\0' && wserver_request_ptr->program_md5_exp[0] == '\0' )
				{
					nret = SyncConnectSocket( qwp.nodes[query_count_used].node.ip , qwp.nodes[query_count_used].node.port , wserver_session_ptr ) ;
					if( nret )
					{
						WarnLog( __FILE__ , __LINE__ , "Query workers failed[%d]" , qwp.response_code );
						continue;
					}
					
					nret = proto_ExecuteProgramRequest( wserver_session_ptr , wserver_request_ptr->program_and_params , wserver_request_ptr ) ;
					if( nret )
					{
						WarnLog( __FILE__ , __LINE__ , "proto_ExecuteProgramRequest failed[%d]errno[%d]" , nret , errno );
						CloseSocket( wserver_session_ptr );
						continue;
					}
					else
					{
						InfoLog( __FILE__ , __LINE__ , "proto_ExecuteProgramRequest ok" );
					}
					
					nret = SyncSendSocketData( wserver_session_ptr , & timeout ) ; 
					if( nret )
					{
						WarnLog( __FILE__ , __LINE__ , "SyncSendSocketData failed[%d]errno[%d]" , nret , errno );
						CloseSocket( wserver_session_ptr );
						continue;
					}
					else
					{
						InfoLog( __FILE__ , __LINE__ , "SyncSendSocketData ok" );
					}
					
					FD_SET( wserver_session_ptr->sock , & read_fds );
					FD_SET( wserver_session_ptr->sock , & expt_fds );
					if( wserver_session_ptr->sock > max_fd )
						max_fd = wserver_session_ptr->sock ;
					InfoLog( __FILE__ , __LINE__ , "CONN wserver_idx[%d] FD_SET[%d] max_fd[%d]" , wserver_idx , wserver_session_ptr->sock , max_fd );
					
					wserver_request_ptr->program_md5_exp[0] = 'X' ;
					
					query_count_used++;
					DebugLog( __FILE__ , __LINE__ , "running_count[%d]->[%d]" , running_count , running_count+1 );
					running_count++;
				}
			}
			
			if( max_fd != -1 )
			{
				DebugLog( __FILE__ , __LINE__ , "select ..." );
				tv.tv_sec = 10 ;
				tv.tv_usec = 0 ;
				select_return_count = select( max_fd+1 , & read_fds , NULL , NULL , & tv ) ;
				DebugLog( __FILE__ , __LINE__ , "select return[%d]" , select_return_count );
				if( select_return_count > 0 )
				{
					for( wserver_request_ptr = penv->wserver_request_array , wserver_response_ptr = penv->wserver_response_array , wserver_session_ptr = penv->wserver_session_array , wserver_idx = 0
						; wserver_idx < program_and_params_count
						; wserver_request_ptr++ , wserver_response_ptr++ , wserver_session_ptr++ , wserver_idx++ )
					{
						if( wserver_session_ptr->established_flag == 1 && FD_ISSET( wserver_session_ptr->sock , & read_fds ) )
						{
							InfoLog( __FILE__ , __LINE__ , "READY wserver_idx[%d] FD_SET[%d]" , wserver_idx , wserver_session_ptr->sock );
							
							timeout = penv->timeout ;
							
							nret = SyncReceiveSocketData( wserver_session_ptr , & timeout ) ; 
							if( nret )
							{
								ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
								CloseSocket( wserver_session_ptr );
								wserver_request_ptr->program_md5_exp[0] = '\0' ;
								DebugLog( __FILE__ , __LINE__ , "running_count[%d]->[%d]" , running_count , running_count-1 );
								running_count--;
								break;
							}
							else
							{
								InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
							}
							
							if( STRNCMP( wserver_session_ptr->recv_buffer + LEN_COMMHEAD , == , "DPQ" , 3 ) )
							{
								nret = proto_DeployProgramRequest( wserver_session_ptr , & dpq ) ;
								if( nret )
								{
									ErrorLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest failed[%d]errno[%d]" , nret , errno );
									CloseSocket(wserver_session_ptr );
									wserver_request_ptr->program_md5_exp[0] = '\0' ;
									DebugLog( __FILE__ , __LINE__ , "running_count[%d]->[%d]" , running_count , running_count-1 );
									running_count--;
									break;
								}
								else
								{
									InfoLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest ok" );
								}
								
								nret = proto_DeployProgramResponse( wserver_session_ptr , dpq.program ) ;
								if( nret )
								{
									ErrorLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest failed[%d]errno[%d]" , nret , errno );
									CloseSocket( wserver_session_ptr );
									wserver_request_ptr->program_md5_exp[0] = '\0' ;
									DebugLog( __FILE__ , __LINE__ , "running_count[%d]->[%d]" , running_count , running_count-1 );
									running_count--;
									break;
								}
								else
								{
									InfoLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest ok" );
								}
								
								nret = SyncSendSocketData( wserver_session_ptr , & timeout ) ; 
								if( nret )
								{
									ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
									CloseSocket( wserver_session_ptr );
									wserver_request_ptr->program_md5_exp[0] = '\0' ;
									DebugLog( __FILE__ , __LINE__ , "running_count[%d]->[%d]" , running_count , running_count-1 );
									running_count--;
									break;
								}
								else
								{
									InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
								}
							}
							else if( STRNCMP( wserver_session_ptr->recv_buffer + LEN_COMMHEAD , == , "EPP" , 3 ) )
							{
								nret = proto_ExecuteProgramResponse( wserver_session_ptr , wserver_response_ptr ) ;
								if( nret )
								{
									ErrorLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse failed[%d]errno[%d]" , nret , errno );
									CloseSocket( wserver_session_ptr );
									wserver_request_ptr->program_md5_exp[0] = '\0' ;
									DebugLog( __FILE__ , __LINE__ , "running_count[%d]->[%d]" , running_count , running_count-1 );
									running_count--;
									break;
								}
								else
								{
									InfoLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse ok" );
								}
								
								InfoLog( __FILE__ , __LINE__ , "CLOSE wserver_idx[%d] FD_SET[%d]" , wserver_idx , wserver_session_ptr->sock );
								CloseSocket( wserver_session_ptr );
								wserver_request_ptr->program_and_params[0] = '\0' ;
								
								DebugLog( __FILE__ , __LINE__ , "running_count[%d]->[%d]" , running_count , running_count-1 );
								running_count--;
								DebugLog( __FILE__ , __LINE__ , "undone_count[%d]->[%d]" , undone_count , undone_count-1 );
								undone_count--;
							}
						}
						
						/*
						if( wserver_session_ptr->established_flag == 1 && FD_ISSET( wserver_session_ptr->sock , & expt_fds ) )
						{
							WarnLog( __FILE__ , __LINE__ , "EXPECT wserver_idx[%d] FD_SET[%d]" , wserver_idx , wserver_session_ptr->sock );
							CloseSocket( wserver_session_ptr );
							wserver_request_ptr->program_md5_exp[0] = '\0' ;
							DebugLog( __FILE__ , __LINE__ , "running_count[%d]->[%d]" , running_count , running_count-1 );
							running_count--;
							break;
						}
						*/
					}
				}
			}
			
			if( undone_count == 0 && max_fd == -1 )
				break;
		}
		while( running_count == worker_count || undone_count == 0 );
	}
	
	return 0;
}

int DC4CDoBatchTasksV( struct Dc4cApiEnv *penv , int worker_count , ... )
{
	char		**program_and_params_array = NULL ; 
	int		program_and_params_count ;
	va_list		valist ;
	
	int		nret = 0 ;
	
	program_and_params_array = (char**)malloc( sizeof(char*) * MAXCNT_PROGRAM_AND_PARAMS_ARRAY ) ;
	if( program_and_params_array == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return DC4C_ERROR_ALLOC;
	}
	memset( program_and_params_array , 0x00 , sizeof(char*) * MAXCNT_PROGRAM_AND_PARAMS_ARRAY );
	
	va_start( valist , worker_count );
	for( program_and_params_count = 0 ; program_and_params_count < MAXCNT_PROGRAM_AND_PARAMS_ARRAY ; program_and_params_count++ )
	{
		program_and_params_array[program_and_params_count] = va_arg( valist , char* ) ;
		if( program_and_params_array[program_and_params_count] == NULL )
			break;
	}
	va_end( valist );
	if( program_and_params_count >= MAXCNT_PROGRAM_AND_PARAMS_ARRAY )
	{
		free( program_and_params_array );
		return DC4C_ERROR_PARAMETER;
	}
	
	nret = DC4CDoBatchTasks( penv , worker_count , program_and_params_array , program_and_params_count ) ;
	free( program_and_params_array );
	return nret;
}

int DC4CGetBatchTasksResponseStatus( struct Dc4cApiEnv *penv , int index , int *status )
{
	if( index <= penv->wserver_array_size )
	{
		(*status) = penv->wserver_response_array[index-1].status ;
		return 0;
	}
	else
	{
		return -1;
	}
}

/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "server.h"

#define PREFIX_DSCLOG_worker_register_request	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_worker_register_request
#include "IDL_worker_register_request.dsc.LOG.c"

#define PREFIX_DSCLOG_worker_register_response	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_worker_register_response
#include "IDL_worker_register_response.dsc.LOG.c"

#define PREFIX_DSCLOG_execute_program_request	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_execute_program_request
#include "IDL_execute_program_request.dsc.LOG.c"

#define PREFIX_DSCLOG_execute_program_response	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_execute_program_response
#include "IDL_execute_program_response.dsc.LOG.c"

#define PREFIX_DSCLOG_deploy_program_request	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_deploy_program_request
#include "IDL_deploy_program_request.dsc.LOG.c"

#define PREFIX_DSCLOG_worker_notice_request	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_worker_notice_request
#include "IDL_worker_notice_request.dsc.LOG.c"

int proto_WorkerRegisterRequest( struct ServerEnv *penv , struct SocketSession *psession )
{
	struct utsname			uts ;
	worker_register_request		req ;
	int				msg_len ;
	
	int				nret = 0 ;
	
	CleanSendBuffer( psession );
	
	DSCINIT_worker_register_request( & req );
	uname( & uts );
	strcpy( req.sysname , uts.sysname );
	strcpy( req.release , uts.release );
	req.bits = sizeof(long) * 8 ;
	strcpy( req.ip , penv->param.wserver_ip );
	req.port = penv->param.wserver_port ;
	
	DSCLOG_worker_register_request( & req );
	
	msg_len = psession->send_buffer_size-1 - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
	nret = DSCSERIALIZE_JSON_COMPACT_worker_register_request( & req , NULL , psession->send_buffer + LEN_COMMHEAD + LEN_MSGHEAD , & msg_len ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_worker_register_request failed[%d]" , nret );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_worker_register_request ok , [%d]bytes" , msg_len );
	}
	
	FormatSendHead( psession , "WRQ" , msg_len );
	
	InfoLog( __FILE__ , __LINE__ , "OUTPUT [%d]bytes[%.*s]" , psession->total_send_len - LEN_COMMHEAD , psession->total_send_len - LEN_COMMHEAD , psession->send_buffer + LEN_COMMHEAD );
	
	return 0;
}

int proto_WorkerNoticeRequest( struct ServerEnv *penv , struct SocketSession *psession )
{
	struct utsname			uts ;
	worker_notice_request		req ;
	int				msg_len ;
	
	int				nret = 0 ;
	
	CleanSendBuffer( psession );
	
	DSCINIT_worker_notice_request( & req );
	uname( & uts );
	strcpy( req.sysname , uts.sysname );
	strcpy( req.release , uts.release );
	req.bits = sizeof(long) * 8 ;
	strcpy( req.ip , penv->param.wserver_ip );
	req.port = penv->param.wserver_port ;
	req.is_working = IsSocketEstablished( & (penv->info_session) ) ;
	
	DSCLOG_worker_notice_request( & req );
	
	msg_len = psession->send_buffer_size-1 - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
	nret = DSCSERIALIZE_JSON_COMPACT_worker_notice_request( & req , NULL , psession->send_buffer + LEN_COMMHEAD + LEN_MSGHEAD , & msg_len ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_worker_notice_request failed[%d]" , nret );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_worker_notice_request ok , [%d]bytes" , msg_len );
	}
	
	FormatSendHead( psession , "WNQ" , msg_len );
	
	InfoLog( __FILE__ , __LINE__ , "OUTPUT [%d]bytes[%.*s]" , psession->total_send_len - LEN_COMMHEAD , psession->total_send_len - LEN_COMMHEAD , psession->send_buffer + LEN_COMMHEAD );
	
	return 0;
}

int proto_ExecuteProgramResponse( struct ServerEnv *penv , struct SocketSession *psession , int error , int status )
{
	execute_program_response	rsp ;
	time_t				end_timestamp ;
	int				msg_len ;
	
	int				nret = 0 ;
	
	CleanSendBuffer( psession );
	
	DSCINIT_execute_program_response( & rsp );
	if( error == 0 )
	{
		strcpy( rsp.tid , penv->epq.tid );
	}
	time( & end_timestamp );
	rsp.elapse = end_timestamp - penv->begin_timestamp ;
	rsp.error = error ;
	rsp.status = status ;
	if( error == 0 )
	{
		strcpy( rsp.info , penv->epp.info );
	}
	
	DSCLOG_execute_program_response( & rsp );
	
	msg_len = psession->send_buffer_size-1 - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
	nret = DSCSERIALIZE_JSON_COMPACT_execute_program_response( & rsp , NULL , psession->send_buffer + LEN_COMMHEAD + LEN_MSGHEAD , & msg_len ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_execute_program_response failed[%d]" , nret );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_execute_program_response ok , [%d]bytes" , msg_len );
	}
	
	FormatSendHead( psession , "EPP" , msg_len );
	
	InfoLog( __FILE__ , __LINE__ , "OUTPUT [%d]bytes[%.*s]" , psession->total_send_len - LEN_COMMHEAD , psession->total_send_len - LEN_COMMHEAD , psession->send_buffer + LEN_COMMHEAD );
	
	return 0;
}

int proto_DeployProgramRequest( struct ServerEnv *penv , struct SocketSession *psession , execute_program_request *p_req )
{
	int				msg_len ;
	
	int				nret = 0 ;
	
	deploy_program_request		req ;
	
	CleanSendBuffer( psession );
	
	DSCINIT_deploy_program_request( & req );
	sscanf( p_req->program_and_params , "%s" , req.program );
	
	DSCLOG_deploy_program_request( & req );
	
	msg_len = psession->send_buffer_size-1 - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
	nret = DSCSERIALIZE_JSON_COMPACT_deploy_program_request( & req , NULL , psession->send_buffer + LEN_COMMHEAD + LEN_MSGHEAD , & msg_len ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_deploy_program_request failed[%d]" , nret );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_deploy_program_request ok , [%d]bytes" , msg_len );
	}
	
	FormatSendHead( psession , "DPQ" , msg_len );
	
	return 0;
}

funcDoProtocol proto ;
int proto( void *_penv , struct SocketSession *psession )
{
	struct ServerEnv	*penv = (struct ServerEnv *) _penv ;
	int			msg_len ;
	
	int			nret = 0 ;
	
	CleanSendBuffer( psession );
	
	if( STRNCMP( psession->recv_buffer + LEN_COMMHEAD , == , "DPP" , LEN_MSGHEAD_MSGTYPE ) )
		InfoLog( __FILE__ , __LINE__ , "INPUT [%d]bytes[%.16s]" , psession->recv_body_len , psession->recv_buffer + LEN_COMMHEAD );
	else
		InfoLog( __FILE__ , __LINE__ , "INPUT [%d]bytes[%.*s]" , psession->recv_body_len , psession->recv_body_len , psession->recv_buffer + LEN_COMMHEAD );
	
	if( psession->recv_body_len < LEN_COMMHEAD )
	{
		ErrorLog( __FILE__ , __LINE__ , "body data is too short" );
		return -1;
	}
	
	if( STRNCMP( psession->recv_buffer + LEN_COMMHEAD , == , "WRP" , LEN_MSGHEAD_MSGTYPE ) )
	{
		worker_register_response	rsp ;
		
		msg_len = psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
		DSCINIT_worker_register_response( & rsp );
		nret = DSCDESERIALIZE_JSON_COMPACT_worker_register_response( NULL , psession->recv_buffer + LEN_COMMHEAD + LEN_MSGHEAD , & msg_len , & rsp ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_worker_register_response failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_worker_register_response ok" );
		}
		
		DSCLOG_worker_register_response( & rsp );
		
		nret = app_WorkerRegisterResponse( penv , psession , & rsp ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "app_WorkerRegisterRequest failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "app_WorkerRegisterRequest ok" );
		}
	}
	else if( STRNCMP( psession->recv_buffer + LEN_COMMHEAD , == , "EPQ" , LEN_MSGHEAD_MSGTYPE ) )
	{
		execute_program_request		req ;
		
		DSCINIT_execute_program_request( & req );
		
		msg_len = psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
		nret = DSCDESERIALIZE_JSON_COMPACT_execute_program_request( NULL , psession->recv_buffer + LEN_COMMHEAD + LEN_MSGHEAD , & msg_len , & req ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_execute_program_request failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_execute_program_request ok" );
		}
		
		DSCLOG_execute_program_request( & req );
		
		nret = app_ExecuteProgramRequest( penv , psession , & req ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "app_WorkerRegisterRequest failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "app_WorkerRegisterRequest ok" );
		}
	}
	else if( STRNCMP( psession->recv_buffer + LEN_COMMHEAD , == , "DPP" , LEN_MSGHEAD_MSGTYPE ) )
	{
		if( IsSocketEstablished( & (penv->info_session) ) )
		{
			nret = proto_ExecuteProgramResponse( penv , psession , DC4C_INFO_ALREADY_EXECUTING , 0 ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse failed[%d]" , nret );
				return -1;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse ok" );
			}
		}
		else
		{
			nret = app_DeployProgramResponse( penv , psession ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "app_DeployProgramResponse failed[%d]" , nret );
				return -1;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "app_DeployProgramResponse ok" );
			}
		}
	}
	else if( STRNCMP( psession->recv_buffer + LEN_COMMHEAD , == , "HBQ" , LEN_MSGHEAD_MSGTYPE ) )
	{
		FormatSendHead( psession , "HBP" , 0 );
	}
	else if( STRNCMP( psession->recv_buffer + LEN_COMMHEAD , == , "HBP" , LEN_MSGHEAD_MSGTYPE ) )
	{
		nret = app_HeartBeatResponse( penv , psession ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "app_HeartBeatResponse failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "app_HeartBeatResponse ok" );
		}
	}
	else
	{
		ErrorLog( __FILE__ , __LINE__ , "unknow msgtype[%.3s]" , psession->recv_buffer + LEN_COMMHEAD );
		return -1;
	}
	
	if( psession->total_send_len )
	{
		InfoLog( __FILE__ , __LINE__ , "OUTPUT [%d]bytes[%.*s]" , psession->total_send_len - LEN_COMMHEAD , psession->total_send_len - LEN_COMMHEAD , psession->send_buffer + LEN_COMMHEAD );
	}
	
	return 0;
}

int proto_HeartBeatRequest( struct ServerEnv *penv , struct SocketSession *psession )
{
	CleanSendBuffer( psession );
	
	FormatSendHead( psession , "HBQ" , 0 );
	
	InfoLog( __FILE__ , __LINE__ , "OUTPUT [%d]bytes[%.*s]" , psession->total_send_len - LEN_COMMHEAD , psession->total_send_len - LEN_COMMHEAD , psession->send_buffer + LEN_COMMHEAD );
	
	return 0;
}


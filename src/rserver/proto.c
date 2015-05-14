#include "rserver.h"

#define PREFIX_DSCLOG_worker_register_request	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_worker_register_request
#include "IDL_worker_register_request.dsc.LOG.c"

#define PREFIX_DSCLOG_worker_register_response	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_worker_register_response
#include "IDL_worker_register_response.dsc.LOG.c"

#define PREFIX_DSCLOG_query_workers_request	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_query_workers_request
#include "IDL_query_workers_request.dsc.LOG.c"

#define PREFIX_DSCLOG_query_workers_response	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_query_workers_response
#include "IDL_query_workers_response.dsc.LOG.c"

#define PREFIX_DSCLOG_worker_notice_request	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_worker_notice_request
#include "IDL_worker_notice_request.dsc.LOG.c"

#define PREFIX_DSCLOG_worker_notice_response	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_worker_notice_response
#include "IDL_worker_notice_response.dsc.LOG.c"

int proto_CommandLine( struct ServerEnv *penv , struct SocketSession *psession )
{
	int		cmd_and_para_count ;
	char		cmd[ 64 + 1 ] ;
	char		param1[ 64 + 1 ] ;
	
	int		nret = 0 ;
	
	CleanSendBuffer( psession );
	
	memset( cmd , 0x00 , sizeof(cmd) );
	memset( param1 , 0x00 , sizeof(param1) );
	cmd_and_para_count = sscanf( psession->recv_buffer , "%s %s" , cmd , param1 ) ;
	
	if( cmd_and_para_count == 1 && cmd[0] && STRNCMP( cmd , == , "quit" , strlen(cmd) ) )
	{
		return RETURN_QUIT;
	}
	else if( cmd_and_para_count == 1 + 1 && STRNCMP( cmd , == , "list" , strlen(cmd) ) && STRNCMP( param1 , == , "os" , strlen(param1) ) )
	{
		nret = app_QueryAllOsTypes( penv , psession ) ;
		if( nret )
		{
			psession->total_send_len = (int)SNPRINTF( psession->send_buffer , psession->send_buffer_size-1 , "unknow internal error[%d]\r\n" , nret );
			ErrorLog( __FILE__ , __LINE__ , "app_QueryAllOsType failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "app_QueryAllOsType ok" );
		}
	}
	else if( cmd_and_para_count == 1 + 1 && STRNCMP( cmd , == , "list" , strlen(cmd) ) && STRNCMP( param1 , == , "hosts" , strlen(param1) ) )
	{
		nret = app_QueryAllHosts( penv , psession ) ;
		if( nret )
		{
			psession->total_send_len = (int)SNPRINTF( psession->send_buffer , psession->send_buffer_size-1 , "unknow internal error[%d]\r\n" , nret );
			ErrorLog( __FILE__ , __LINE__ , "app_QueryAllHosts failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "app_QueryAllHosts ok" );
		}
	}
	else if( cmd_and_para_count == 1 + 1 && STRNCMP( cmd , == , "list" , strlen(cmd) ) && STRNCMP( param1 , == , "worker" , strlen(param1) ) )
	{
		nret = app_QueryAllWorkers( penv , psession ) ;
		if( nret )
		{
			psession->total_send_len = (int)SNPRINTF( psession->send_buffer , psession->send_buffer_size-1 , "unknow internal error[%d]\r\n" , nret );
			ErrorLog( __FILE__ , __LINE__ , "app_QueryAllWorkers failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "app_QueryAllWorkers ok" );
		}
	}
	else
	{
		psession->total_send_len = (int)SNPRINTF( psession->send_buffer , psession->send_buffer_size-1 , "unknow cmd[%s]\r\n" , psession->recv_buffer );
		WarnLog( __FILE__ , __LINE__ , "unknow cmd[%s]" , psession->recv_buffer );
		return 0;
	}
	
	return 0;
}

funcDoProtocol proto ;
int proto( void *_penv , struct SocketSession *psession )
{
	struct ServerEnv	*penv = (struct ServerEnv *) _penv ;
	int			msg_len ;
	
	int			nret = 0 ;
	
	if( psession->comm_protocol_mode == COMMPROTO_LINE )
	{
		return proto_CommandLine( penv , psession );
	}
	
	CleanSendBuffer( psession );
	
	InfoLog( __FILE__ , __LINE__ , "INPUT [%d]bytes[%.*s]" , psession->recv_body_len , psession->recv_body_len , psession->recv_buffer + LEN_COMMHEAD );
	
	if( psession->recv_body_len < LEN_COMMHEAD )
	{
		ErrorLog( __FILE__ , __LINE__ , "body data is too short" );
		return -1;
	}
	
	if( STRNCMP( psession->recv_buffer + LEN_COMMHEAD , == , "WRQ" , LEN_MSGHEAD_MSGTYPE ) )
	{
		worker_register_request		req ;
		worker_register_response	rsp ;
		
		DSCINIT_worker_register_request( & req );
		DSCINIT_worker_register_response( & rsp );
		
		msg_len = psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
		nret = DSCDESERIALIZE_JSON_COMPACT_worker_register_request( NULL , psession->recv_buffer + LEN_COMMHEAD + LEN_MSGHEAD_MSGTYPE , & msg_len , & req ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_worker_register_request failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_worker_register_request ok" );
		}
		
		DSCLOG_worker_register_request( & req );
		
		nret = app_WorkerRegisterRequest( penv , psession , & req , & rsp ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "app_WorkerRegisterRequest failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "app_WorkerRegisterRequest ok" );
		}
		
		DSCLOG_worker_register_response( & rsp );
		
		memset( psession->send_buffer , 0x00 , psession->send_buffer_size );
		msg_len = psession->send_buffer_size-1 - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
		nret = DSCSERIALIZE_JSON_COMPACT_worker_register_response( & rsp , NULL , psession->send_buffer + LEN_COMMHEAD + LEN_MSGHEAD , & msg_len ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_worker_register_response failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_worker_register_response ok , [%d]bytes" , msg_len );
		}
		
		FormatSendHead( psession , "WRP" , msg_len );
	}
	else if( STRNCMP( psession->recv_buffer + LEN_COMMHEAD , == , "QWQ" , LEN_MSGHEAD_MSGTYPE ) )
	{
		query_workers_request		req ;
		query_workers_response		rsp ;
		
		DSCINIT_query_workers_request( & req );
		DSCINIT_query_workers_response( & rsp );
		
		msg_len = psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
		nret = DSCDESERIALIZE_JSON_COMPACT_query_workers_request( NULL , psession->recv_buffer + LEN_COMMHEAD + LEN_MSGHEAD_MSGTYPE , & msg_len , & req ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_query_workers_request failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_query_workers_request ok" );
		}
		
		DSCLOG_query_workers_request( & req );
		
		nret = app_QueryWorkersRequest( penv , psession , & req , & rsp ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "app_WorkerRegisterRequest failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "app_WorkerRegisterRequest ok" );
		}
		
		DSCLOG_query_workers_response( & rsp );
		
		memset( psession->send_buffer , 0x00 , psession->send_buffer_size );
		msg_len = psession->send_buffer_size-1 - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
		nret = DSCSERIALIZE_JSON_COMPACT_query_workers_response( & rsp , NULL , psession->send_buffer + LEN_COMMHEAD + LEN_MSGHEAD , & msg_len ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_query_workers_response failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_query_workers_response ok , [%d]bytes" , msg_len );
		}
		
		FormatSendHead( psession , "QWP" , msg_len );
	}
	else if( STRNCMP( psession->recv_buffer + LEN_COMMHEAD , == , "WNQ" , LEN_MSGHEAD_MSGTYPE ) )
	{
		worker_notice_request		req ;
		
		DSCINIT_worker_notice_request( & req );
		
		msg_len = psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
		nret = DSCDESERIALIZE_JSON_COMPACT_worker_notice_request( NULL , psession->recv_buffer + LEN_COMMHEAD + LEN_MSGHEAD_MSGTYPE , & msg_len , & req ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_worker_notice_request failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_COMPACT_worker_notice_request ok" );
		}
		
		DSCLOG_worker_notice_request( & req );
		
		nret = app_WorkerNoticeRequest( penv , psession , & req ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "app_WorkerNoticeRequest failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "app_WorkerNoticeRequest ok" );
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


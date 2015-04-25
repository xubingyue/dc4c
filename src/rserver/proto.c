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
	char		cmd[ 64 + 1 ] ;
	char		param[ 64 + 1 ] ;
	
	int		nret = 0 ;
	
	memset( cmd , 0x00 , sizeof(cmd) );
	memset( param , 0x00 , sizeof(param) );
	sscanf( psession->recv_buffer , "%s %s" , cmd , param );
	
	if( cmd[0] == '\0' )
	{
		return 0;
	}
	else if( STRNCMP( cmd , == , "quit" , strlen(cmd) ) )
	{
		return 1;
	}
	else if( STRNCMP( cmd , == , "list" , strlen(cmd) ) )
	{
		if( STRNCMP( param , == , "workers" , strlen(param) ) || param[0] == '\0' )
		{
			nret = app_QueryAllWorkers( penv , psession ) ;
			if( nret )
			{
				psession->total_send_len = (int)SNPRINTF( psession->send_buffer , psession->send_buffer_size-1 , "unknow internal error[%d]\r\n" , nret );
				ErrorLog( __FILE__ , __LINE__ , "app_QueryAllWorkers failed[%d]" , nret );
				return -1;
			}
		}
		else if( STRNCMP( param , == , "hosts" , strlen(param) ) )
		{
			nret = app_QueryAllHosts( penv , psession ) ;
			if( nret )
			{
				psession->total_send_len = (int)SNPRINTF( psession->send_buffer , psession->send_buffer_size-1 , "unknow internal error[%d]\r\n" , nret );
				ErrorLog( __FILE__ , __LINE__ , "app_QueryAllHosts failed[%d]" , nret );
				return -1;
			}
		}
		else if( STRNCMP( param , == , "os" , strlen(param) ) )
		{
			nret = app_QueryAllOsTypes( penv , psession ) ;
			if( nret )
			{
				psession->total_send_len = (int)SNPRINTF( psession->send_buffer , psession->send_buffer_size-1 , "unknow internal error[%d]\r\n" , nret );
				ErrorLog( __FILE__ , __LINE__ , "app_QueryAllOsType failed[%d]" , nret );
				return -1;
			}
		}
		else
		{
			psession->total_send_len = (int)SNPRINTF( psession->send_buffer , psession->send_buffer_size-1 , "unknow param[%s]\r\n" , param );
			WarnLog( __FILE__ , __LINE__ , "unknow param[%s]" , param );
			return 0;
		}
	}
	else
	{
		psession->total_send_len = (int)SNPRINTF( psession->send_buffer , psession->send_buffer_size-1 , "unknow cmd[%s]\r\n" , cmd );
		WarnLog( __FILE__ , __LINE__ , "unknow cmd[%s]" , cmd );
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
	
	InfoLog( __FILE__ , __LINE__ , "input buffer [%d]bytes[%.*s]" , psession->recv_body_len , psession->recv_body_len , psession->recv_buffer + LEN_COMMHEAD );
	
	if( psession->recv_body_len < LEN_COMMHEAD )
	{
		ErrorLog( __FILE__ , __LINE__ , "body data is too short" );
		return -1;
	}
	
	if( STRNCMP( psession->recv_buffer + LEN_COMMHEAD , == , "WRQ" , LEN_MSGHEAD_MSGTYPE ) )
	{
		worker_register_request		req ;
		worker_register_response	rsp ;
		
		/* test
		00000103WRQ     {"os":"RedHatEnterpriseLinux","main_version":"5.8","bits":"64","ip":"127.0.0.1","port":11124}
		*/
		
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
		
		/* test
		00000084WWQ     {"os":"RedHatEnterpriseLinux","main_version":"5.8","bits":"64","count":-1}
		*/
		
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
		worker_notice_response		rsp ;
		
		/* test
		00000056WNQ     {"ip":"127.0.0.1","port":11124,"is_working":1}
		*/
		
		DSCINIT_worker_notice_request( & req );
		DSCINIT_worker_notice_response( & rsp );
		
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
		
		nret = app_WorkerNoticeRequest( penv , psession , & req , & rsp ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "app_WorkerRegisterRequest failed[%d]" , nret );
			return -1;
		}
		
		DSCLOG_worker_notice_response( & rsp );
		
		memset( psession->send_buffer , 0x00 , psession->send_buffer_size );
		msg_len = psession->send_buffer_size-1 - LEN_COMMHEAD - LEN_MSGHEAD_MSGTYPE ;
		nret = DSCSERIALIZE_JSON_COMPACT_worker_notice_response( & rsp , NULL , psession->send_buffer + LEN_COMMHEAD + LEN_MSGHEAD , & msg_len ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_worker_notice_response failed[%d]" , nret );
			return -1;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCSERIALIZE_JSON_COMPACT_worker_notice_response ok , [%d]bytes" , msg_len );
		}
		
		FormatSendHead( psession , "WNP" , msg_len );
	}
	else
	{
		ErrorLog( __FILE__ , __LINE__ , "unknow msgtype[%.3s]" , psession->recv_buffer + LEN_COMMHEAD );
		return -1;
	}
	
	if( psession->total_send_len )
	{
		InfoLog( __FILE__ , __LINE__ , "output buffer [%d]bytes[%.*s]" , psession->total_send_len - LEN_COMMHEAD , psession->total_send_len - LEN_COMMHEAD , psession->send_buffer + LEN_COMMHEAD );
	}
	
	return 0;
}

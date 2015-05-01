#include "dc4c_util.h"
#include "dc4c_api.h"

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

int app_QueryWorkersRequest( struct SocketSession *psession , query_workers_request *p_req , int want_count );
int app_ExecuteProgramRequest( struct SocketSession *psession , execute_program_request *p_req , char *program_and_params );

int proto_QueryWorkersRequest( struct SocketSession *psession , int want_count )
{
	int				msg_len ;
	
	int				nret = 0 ;
	
	query_workers_request		req ;
	
	CleanSendBuffer( psession );
	
	DSCINIT_query_workers_request( & req );
	nret = app_QueryWorkersRequest( psession , & req , want_count ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "app_QueryWorkersRequest failed[%d]" , nret );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "app_QueryWorkersRequest ok" );
	}
	
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

int proto_QueryWorkersResponse( struct SocketSession *psession , query_workers_response *p_rsp )
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

int proto_ExecuteProgramRequest( struct SocketSession *psession , char *program_and_params , execute_program_request *p_req )
{
	int				msg_len ;
	
	int				nret = 0 ;
	
	execute_program_request		req ;
	
	CleanSendBuffer( psession );
	
	DSCINIT_execute_program_request( & req );
	nret = app_ExecuteProgramRequest( psession , & req , program_and_params ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "app_ExecuteProgramRequest failed[%d]" , nret );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "app_ExecuteProgramRequest ok" );
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

int proto_DeployProgramRequest( struct SocketSession *psession , deploy_program_request *p_rsp )
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

int proto_DeployProgramResponse( struct SocketSession *psession , char *program )
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
	
	InfoLog( __FILE__ , __LINE__ , "output buffer [%d]bytes[%.*s]" , psession->total_send_len - LEN_COMMHEAD , psession->total_send_len - LEN_COMMHEAD , psession->send_buffer + LEN_COMMHEAD );
	
	return 0;
}

int proto_ExecuteProgramResponse( struct SocketSession *psession , execute_program_response *p_rsp )
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


#include "dc4c_api.h"

int proto_QueryWorkersRequest( struct SocketSession *psession );
int proto_QueryWorkersResponse( struct SocketSession *psession , query_workers_response *p_rsp );
int proto_ExecuteProgramRequest( struct SocketSession *psession , char *program , char *params );
int proto_ExecuteProgramResponse( struct SocketSession *psession , execute_program_response *p_rsp );
int proto_DeployProgramRequest( struct SocketSession *psession , deploy_program_request *p_rsp );
int proto_DeployProgramResponse( struct SocketSession *psession , char *program );

int DC4CDoTaskImmediately( char *rserver_ip , int rserver_port , char *program , char *params , execute_program_response *p_epp )
{
	struct SocketSession		session ;
	query_workers_response		qwp ;
	deploy_program_request		dpq ;
	
	int				nret = 0 ;
	
	SetLogFile( "%s/log/dc4c_api.log" , getenv("HOME") );
	SetLogLevel( LOGLEVEL_DEBUG );
	
_GOTO_QUERY_WORKERS :
	
	InitSocketSession( & session );
	
	nret = SyncConnectSocket( rserver_ip , rserver_port , & session ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] failed[%d]errno[%d]" , rserver_ip , rserver_port , nret , errno );
		close( session.sock );
		return DC4C_ERROR_CONNECT;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] ok" , rserver_ip , rserver_port );
	}
	
	nret = proto_QueryWorkersRequest( & session ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_QueryWorkersRequest failed[%d]errno[%d]" , nret , errno );
		close( session.sock );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "proto_QueryWorkersRequest ok" );
	}
	
	nret = SyncSendSocketData( & session ) ; 
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
		close( session.sock );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
	}
	
	nret = SyncReceiveSocketData( & session ) ; 
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
		close( session.sock );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
	}
	
	if( STRNCMP( session.recv_buffer + LEN_COMMHEAD , != , "QWP" , 3 ) )
	{
		ErrorLog( __FILE__ , __LINE__ , "invalid msghead.msgtype[%.*s]" , 3 , session.recv_buffer + LEN_COMMHEAD );
		close( session.sock );
		return nret;
	}
	
	nret = proto_QueryWorkersResponse( & session , & qwp ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_QueryWorkersResponse failed[%d]errno[%d]" , nret , errno );
		close( session.sock );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "proto_QueryWorkersResponse ok" );
	}
	
	if( qwp.response_code == 5 || qwp.count == 0 || qwp._nodes_count == 0 )
	{
		close( session.sock );
		InfoLog( __FILE__ , __LINE__ , "no worker" );
		sleep(1);
		goto _GOTO_QUERY_WORKERS;
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
	
	close( session.sock );
	
	nret = SyncConnectSocket( qwp.nodes[0].node.ip , qwp.nodes[0].node.port , & session ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] failed[%d]errno[%d]" , qwp.nodes[0].node.ip , qwp.nodes[0].node.port , nret , errno );
		return DC4C_ERROR_CONNECT;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "SyncConnectSocket[%s:%d] ok" , qwp.nodes[0].node.ip , qwp.nodes[0].node.port );
	}
	
	nret = proto_ExecuteProgramRequest( & session , program , params ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_ExecuteProgramRequest failed[%d]errno[%d]" , nret , errno );
		close( session.sock );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "proto_ExecuteProgramRequest ok" );
	}
	
	nret = SyncSendSocketData( & session ) ; 
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
		close( session.sock );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
	}
	
	nret = SyncReceiveSocketData( & session ) ; 
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
		close( session.sock );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
	}
	
	if( STRNCMP( session.recv_buffer + LEN_COMMHEAD , == , "DPQ" , 3 ) )
	{
		nret = proto_DeployProgramRequest( & session , & dpq ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest failed[%d]errno[%d]" , nret , errno );
			close( session.sock );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest ok" );
		}
		
		nret = proto_DeployProgramResponse( & session , dpq.program ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest failed[%d]errno[%d]" , nret , errno );
			close( session.sock );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "proto_DeployProgramRequest ok" );
		}
		
		nret = SyncSendSocketData( & session ) ; 
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
			close( session.sock );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
		}
		
		nret = SyncReceiveSocketData( & session ) ; 
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "SyncReceiveSocketData failed[%d]errno[%d]" , nret , errno );
			close( session.sock );
			return nret;
		}
		else
		{
			InfoLog( __FILE__ , __LINE__ , "SyncReceiveSocketData ok" );
		}
	}
	
	if( STRNCMP( session.recv_buffer + LEN_COMMHEAD , != , "EPP" , 3 ) )
	{
		ErrorLog( __FILE__ , __LINE__ , "invalid response[%.*s]" , session.total_recv_len , session.recv_buffer );
		close( session.sock );
		return -1;
	}
	
	nret = proto_ExecuteProgramResponse( & session , p_epp ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_ProgramExitRequest failed[%d]errno[%d]" , nret , errno );
		close( session.sock );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "proto_ProgramExitRequest ok" );
	}
	
	close( session.sock );
	
	return 0;
}

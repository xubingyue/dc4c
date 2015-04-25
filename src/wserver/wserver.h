#ifndef _H_WSERVER_
#define _H_WSERVER_

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "IDL_worker_register_request.dsc.h"
#include "IDL_worker_register_response.dsc.h"
#include "IDL_worker_notice_request.dsc.h"
#include "IDL_worker_notice_response.dsc.h"
#include "IDL_execute_program_request.dsc.h"
#include "IDL_execute_program_response.dsc.h"
#include "IDL_deploy_program_request.dsc.h"

#include "dc4c_util.h"
#include "dc4c_api.h"

#define EPOLL_FD_COUNT			1024
#define WAIT_EVENTS_COUNT		100

struct CommandParameter
{
	char				rserver_ip[ 40 + 1 ] ;
	int				rserver_port ;
	char				wserver_ip[ 40 + 1 ] ;
	int				wserver_port ;
} ;

struct ServerEnv
{
	struct CommandParameter		param ;
	
	int				epoll_socks ;
	struct SocketSession		listen_session ;
	int				connect_flag ;
	struct SocketSession		connect_session ;
	int				accepted_flag ;
	struct SocketSession		accepted_session ;
	int				exit_pipe[ 2 ] ;
	struct SocketSession		exit_session ;
	
	int				working_flag ;
	execute_program_request		epq ;
	pid_t				pid ;
} ;

int server( struct ServerEnv *penv );

int comm_AsyncConnectToRegisterServer( struct ServerEnv *penv , int skip_connect_flag );
int comm_OnConnectedSocketInput( struct ServerEnv *penv , struct SocketSession *p_event_session );
int comm_OnConnectedSocketOutput( struct ServerEnv *penv , struct SocketSession *p_event_session );
int comm_OnConnectedSocketError( struct ServerEnv *penv , struct SocketSession *p_event_session );
int comm_OnListenSocketInput( struct ServerEnv *penv , struct SocketSession *p_event_session );
int comm_OnAcceptedSocketInput( struct ServerEnv *penv , struct SocketSession *p_event_session );
int comm_OnAcceptedSocketOutput( struct ServerEnv *penv , struct SocketSession *p_event_session );
int comm_OnAcceptedSocketError( struct ServerEnv *penv , struct SocketSession *p_event_session );

int proto_WorkerRegisterRequest( struct ServerEnv *penv , struct SocketSession *psession );
int proto_ProgramExitRequest( struct ServerEnv *penv , struct SocketSession *psession , int status );
int proto( void *_penv , struct SocketSession *psession );

int app_WorkerRegisterRequest( struct ServerEnv *penv , struct SocketSession *psession , worker_register_request *p_req );
int app_WorkerRegisterResponse( struct ServerEnv *penv , struct SocketSession *psession , worker_register_response *p_rsp );
int app_WorkerNoticeRequest( struct ServerEnv *penv , struct SocketSession *psession , worker_notice_request *p_req , worker_notice_response *p_rsp );
int app_ExecuteProgramRequest( struct ServerEnv *penv , struct SocketSession *psession , execute_program_request *p_req );

#endif

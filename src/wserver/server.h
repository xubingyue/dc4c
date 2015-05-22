/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#ifndef _H_SERVER_
#define _H_SERVER_

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "IDL_worker_register_request.dsc.h"
#include "IDL_worker_register_response.dsc.h"
#include "IDL_worker_notice_request.dsc.h"
#include "IDL_execute_program_request.dsc.h"
#include "IDL_execute_program_response.dsc.h"
#include "IDL_deploy_program_request.dsc.h"

#include "dc4c_util.h"
#include "dc4c_api.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char *__DC4C_WSERVER_VERSION ;

#define EPOLL_FD_COUNT			1024
#define WAIT_EVENTS_COUNT		100

#define RSERVER_ARRAYSIZE		8
#define MAXCOUNT_WSERVERS		1000
#define MAXCOUNT_ACCEPTED_SESSION	1000

#define SEND_HEARTBEAT_INTERVAL		60
#define MAXCNT_HEARTBEAT_LOST		3

struct CommandParameter
{
	char				rserver_ip[ RSERVER_ARRAYSIZE ][ 40 + 1 ] ;
	int				rserver_port[ RSERVER_ARRAYSIZE ] ;
	char				wserver_ip[ 40 + 1 ] ;
	int				wserver_port ;
	int				wserver_port_base ;
	int				wserver_count ;
	int				loglevel_debug ;
} ;

#define CONNECT_SESSION_PROGRESS_CLOSED		0
#define CONNECT_SESSION_PROGRESS_CONNECTED	1
#define CONNECT_SESSION_PROGRESS_REGISTED	2

struct ServerEnv
{
	struct CommandParameter		param ;
	
	int				epoll_socks ;
	struct SocketSession		listen_session ;
	struct SocketSession		*accepted_session_array ; /* sizeof(struct SocketSession) * MAXCOUNT_ACCEPTED_SESSION */
	struct SocketSession		*p_slibing_accepted_session ;
	struct SocketSession		connect_session[ RSERVER_ARRAYSIZE ] ;
	int				info_pipe[ 2 ] ;
	struct SocketSession		info_session ;
	
	int				rserver_count ;
	
	int				wserver_index ;
	
	int				is_working ;
	execute_program_request		epq ;
	execute_program_response	epp ;
	pid_t				pid ;
	time_t				begin_timestamp ;
} ;

int server( struct ServerEnv *penv );

int comm_AsyncConnectToRegisterServer( struct ServerEnv *penv , struct SocketSession *psession , int skip_connect_flag );
int comm_CloseConnectedSocket( struct ServerEnv *penv , struct SocketSession *psession );
int comm_CloseAcceptedSocket( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnConnectedSocketInput( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnConnectedSocketOutput( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnConnectedSocketError( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnInfoPipeInput( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnInfoPipeError( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnListenSocketInput( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnAcceptedSocketInput( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnAcceptedSocketOutput( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnAcceptedSocketError( struct ServerEnv *penv , struct SocketSession *psession );

int proto_WorkerRegisterRequest( struct ServerEnv *penv , struct SocketSession *psession );
int proto_WorkerNoticeRequest( struct ServerEnv *penv , struct SocketSession *psession );
int proto_ExecuteProgramResponse( struct ServerEnv *penv , struct SocketSession *psession , int error , int status );
int proto_DeployProgramRequest( struct ServerEnv *penv , struct SocketSession *psession , execute_program_request *p_req );
#define RETURN_QUIT	1
int proto( void *_penv , struct SocketSession *psession );
int proto_HeartBeatRequest( struct ServerEnv *penv , struct SocketSession *psession );

int app_WorkerRegisterResponse( struct ServerEnv *penv , struct SocketSession *psession , worker_register_response *p_rsp );
int app_WaitProgramExiting( struct ServerEnv *penv , struct SocketSession *psession );
int app_ExecuteProgramRequest( struct ServerEnv *penv , struct SocketSession *psession , execute_program_request *p_req );
int app_DeployProgramResponse( struct ServerEnv *penv , struct SocketSession *psession );
int app_HeartBeatRequest( struct ServerEnv *penv , long *p_now , long *p_epoll_timeout );
int app_HeartBeatResponse( struct ServerEnv *penv , struct SocketSession *psession );

int AddInputSockToEpoll( int epoll_socks , struct SocketSession *psession );
int AddOutputSockToEpoll( int epoll_socks , struct SocketSession *psession );
int ModifyInputSockFromEpoll( int epoll_socks , struct SocketSession *psession );
int ModifyOutputSockFromEpoll( int epoll_socks , struct SocketSession *psession );
int DeleteSockFromEpoll( int epoll_socks , struct SocketSession *psession );
int lock_file( int *p_fd );
int unlock_file( int *p_fd );

#ifdef __cplusplus
}
#endif

#endif

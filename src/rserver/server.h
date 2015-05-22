/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#ifndef _H_SERVER_
#define _H_SERVER_

#include "IDL_worker_register_request.dsc.h"
#include "IDL_worker_register_response.dsc.h"
#include "IDL_query_workers_request.dsc.h"
#include "IDL_query_workers_response.dsc.h"
#include "IDL_worker_notice_request.dsc.h"

#include "dc4c_util.h"
#include "dc4c_api.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char *__DC4C_RSERVER_VERSION ;

#define EPOLL_FD_COUNT			1024
#define WAIT_EVENTS_COUNT		100

#define SEND_HEARTBEAT_INTERVAL		60
#define MAXCNT_HEARTBEAT_LOST		3

#define MAXCOUNT_ACCEPTED_SESSION	1000

struct WorkerInfo
{
	int				port ;
	
	int				is_working ;
	
	long				access_timestamp ;
} ;

struct HostInfo
{
	char				ip[ sizeof( ((worker_register_request*)NULL)->ip ) ] ;
	
	int				idler_count ;
	int				working_count ;
	
	SList				*worker_info_list ; /* struct WorkerInfo */
} ;

struct OsType
{
	char				sysname[ sizeof( ((worker_register_request*)NULL)->sysname ) ] ;
	char				release[ sizeof( ((worker_register_request*)NULL)->release ) ] ;
	int				bits ;
	
	SList				*host_info_list ; /* struct HostInfo */
} ;

struct CommandParameter
{
	char				rserver_ip[ 40 + 1 ] ;
	int				rserver_port ;
	int				loglevel_debug ;
} ;

#define SESSIONTYPE_WORKER		1
#define SESSIONTYPE_MASTER		2

struct ServerEnv
{
	struct CommandParameter		param ;
	
	int				epoll_socks ;
	struct SocketSession		listen_session ;
	struct SocketSession		*accepted_session_array ; /* sizeof(struct SocketSession) * MAXCOUNT_ACCEPTED_SESSION */
	struct SocketSession		*p_slibing_accepted_session ;
	/*
	list os ( l o )
	list host ( l h )
	list worker ( l w )
	*/
	SList				*os_type_list ; /* struct OsType */
} ;

int server( struct ServerEnv *penv );

int comm_CloseAcceptedSocket( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnListenSocketInput( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnAcceptedSocketInput( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnAcceptedSocketOutput( struct ServerEnv *penv , struct SocketSession *psession );
int comm_OnAcceptedSocketError( struct ServerEnv *penv , struct SocketSession *psession );

int proto_CommandLine( struct ServerEnv *penv , struct SocketSession *psession );
#define RETURN_QUIT	1
int proto( void *_penv , struct SocketSession *psession );
int proto_HeartBeatRequest( struct ServerEnv *penv , struct SocketSession *psession );

int app_WorkerRegisterRequest( struct ServerEnv *penv , struct SocketSession *psession , worker_register_request *p_req , worker_register_response *p_rsp );
int app_QueryWorkersRequest( struct ServerEnv *penv , struct SocketSession *psession , query_workers_request *p_req , query_workers_response *p_rsp );
int app_WorkerNoticeRequest( struct ServerEnv *penv , struct SocketSession *psession , worker_notice_request *p_req );
int app_WorkerUnregister( struct ServerEnv *penv , struct SocketSession *psession );
int app_QueryAllOsTypes( struct ServerEnv *penv , struct SocketSession *psession );
int app_QueryAllHosts( struct ServerEnv *penv , struct SocketSession *psession );
int app_QueryAllWorkers( struct ServerEnv *penv , struct SocketSession *psession );
int app_HeartBeatRequest( struct ServerEnv *penv , long *p_tt , long *p_epoll_timeout );
int app_HeartBeatResponse( struct ServerEnv *penv , struct SocketSession *psession );

int AddInputSockToEpoll( int epoll_socks , struct SocketSession *psession );
int AddOutputSockToEpoll( int epoll_socks , struct SocketSession *psession );
int ModifyInputSockFromEpoll( int epoll_socks , struct SocketSession *psession );
int ModifyOutputSockFromEpoll( int epoll_socks , struct SocketSession *psession );
int DeleteSockFromEpoll( int epoll_socks , struct SocketSession *psession );

#ifdef __cplusplus
}
#endif

#endif

/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#ifndef _H_DC4C_UTIL_
#define _H_DC4C_UTIL_

#include <stdio.h>

#if ( defined __linux ) || ( defined __unix )
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <limits.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define __USE_GNU
#include <sched.h>
#define _VSNPRINTF		vsnprintf
#define _SNPRINTF		snprintf
#define _CLOSESOCKET		close
#define _ERRNO			errno
#define _EWOULDBLOCK		EWOULDBLOCK
#define _ECONNABORTED		ECONNABORTED
#define _EINPROGRESS		EINPROGRESS
#define _ECONNRESET		ECONNRESET
#define _SOCKLEN_T		socklen_t
#elif ( defined _WIN32 )
#include <time.h>
#include <sys/types.h>
#include <io.h>
#include <windows.h>
#define _VSNPRINTF		_vsnprintf
#define _SNPRINTF		_snprintf
#define _CLOSESOCKET		closesocket
#define _ERRNO			GetLastError()
#define _EWOULDBLOCK		WSAEWOULDBLOCK
#define _ECONNABORTED		WSAECONNABORTED
#define _EINPROGRESS		WSAEINPROGRESS
#define _ECONNRESET		WSAECONNRESET
#define _SOCKLEN_T		int
#endif

#ifndef MIN
#define MIN(_a_,_b_) ((_a_)<(_b_)?(_a_):(_b_))
#endif

#ifndef MAX
#define MAX(_a_,_b_) ((_a_)>(_b_)?(_a_):(_b_))
#endif

#include "openssl/md5.h"
#include "openssl/err.h"

#include "LOGC.h"
#include "ListX.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char *__DC4C_VERSION ;

#define MAXCNT_LISTEN_BAKLOG		10000

#define LEN_COMMHEAD			8
#define LEN_MSGHEAD			8
#define LEN_MSGHEAD_MSGTYPE		3

#define COMMPROTO_PRELEN		0
#define COMMPROTO_LINE			1

#define DEFAULT_RECV_BUFFERSIZE		100*1024
#define DEFAULT_SEND_BUFFERSIZE		100*1024

struct SocketSession
{
	char			ip[ 40 + 1 ] ;
	long			port ;
	int			sock ;
	struct sockaddr_in	addr ;
	int			established_flag ;
	
	int			recv_buffer_size ;
	char			*recv_buffer ;
	int			total_recv_len ;
	int			recv_body_len ;
	
	int			send_buffer_size ;
	char			*send_buffer ;
	int			total_send_len ;
	int			send_len ;
	
	int			type ;
	int			comm_protocol_mode ;
	int			progress ;
	
	time_t			alive_timestamp ;
	int			alive_timeout ;
	time_t			active_timestamp ;
	int			heartbeat_lost_count ;
	
	char			ch ;
	void			*p1 ;
	void			*p2 ;
	void			*p3 ;
} ;

/********* comm *********/

int InitSocketSession( struct SocketSession *psession );
void CleanSocketSession( struct SocketSession *psession );
void FreeSocketSession( struct SocketSession *psession );
struct SocketSession *AllocSocketSession();
void CleanSendBuffer( struct SocketSession *psession );
void CleanRecvBuffer( struct SocketSession *psession );
int ReallocSendBuffer( struct SocketSession *psession , int new_buffer_size );
int ReallocRecvBuffer( struct SocketSession *psession , int new_buffer_size );

struct SocketSession *GetUnusedSocketSession( struct SocketSession *p_session_array , int count , struct SocketSession **pp_slibing_session );

int SetAddrReused( int sock );
int SetNonBlocking( int sock );
void SetSocketAddr( struct sockaddr_in *p_sockaddr , char *ip , long port );
void GetSocketAddr( struct sockaddr_in *addr , char *ip , long *port );

int BindListenSocket( char *ip , long port , struct SocketSession *psession );
#define RETURN_CONNECTING_IN_PROGRESS		1
int AcceptSocket( int listen_sock , struct SocketSession *psession );
int AsyncConnectSocket( char *ip , long port , struct SocketSession *psession );
int AsyncCompleteConnectedSocket( struct SocketSession *psession );
int DiscardAcceptSocket( int listen_sock );
void CloseSocketSilently( struct SocketSession *psession );
void CloseSocket( struct SocketSession *psession );
void SetSocketClosed( struct SocketSession *psession );
void SetSocketOpened( struct SocketSession *psession );
void SetSocketEstablished( struct SocketSession *psession );
int IsSocketClosed( struct SocketSession *psession );
int IsSocketOpened( struct SocketSession *psession );
int IsSocketEstablished( struct SocketSession *psession );
typedef int funcDoProtocol( void *_penv , struct SocketSession *psession );
#define OPTION_ASYNC_CHANGE_MODE_FLAG		1
#define RETURN_RECEIVING_IN_PROGRESS		1
#define RETURN_CHANGE_COMM_PROTOCOL_MODE	2
#define RETURN_NO_SEND_RESPONSE			3
#define RETURN_PEER_CLOSED			4
int AsyncReceiveSocketData( struct SocketSession *psession , int change_mode_flag );
int AfterDoProtocol( struct SocketSession *psession );
#define OPTION_ASYNC_SKIP_RECV_FLAG		1
int AsyncReceiveCommand( struct SocketSession *psession , int skip_recv_flag );
int AfterDoCommandProtocol( struct SocketSession *psession );
#define RETURN_SENDING_IN_PROGRESS		1
int AsyncSendSocketData( struct SocketSession *psession );
int SyncConnectSocket( char *ip , long port , struct SocketSession *psession );
#define RETURN_TIMEOUT				1
int SyncReceiveSocketData( struct SocketSession *psession , int *p_timeout );
int SyncSendSocketData( struct SocketSession *psession , int *p_timeout );

/********* proto *********/

void FormatSendHead( struct SocketSession *psession , char *msg_type , int msg_len );

/********* util *********/

int ConvertToDaemonServer();
int ns2i( char *str , int len );
int FileMd5( char *pathfilename , char *program_md5_exp ); /* char program_md5_exp[ MD5_DIGEST_LENGTH * 2 + 1 ] ; */

int BindCpuProcessor( int processor_no );
int UnbindCpuProcessor();

#ifdef __cplusplus
}
#endif

#endif

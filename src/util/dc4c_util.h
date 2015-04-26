#ifndef _H_DC4C_UTIL_
#define _H_DC4C_UTIL_

#include <stdio.h>

#if ( defined __linux ) || ( defined __unix )
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/epoll.h>
#include <signal.h>
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

#include "openssl/md5.h"
#include "openssl/err.h"

#include "LOGC.h"
#include "ListX.h"

#define MAXCNT_LISTEN_BAKLOG		10000

#define LEN_COMMHEAD			8
#define LEN_MSGHEAD			8
#define LEN_MSGHEAD_MSGTYPE		3

#define COMMPROTO_PRELEN		0
#define COMMPROTO_LINE			1

struct SocketSession
{
	char			ip[ 40 + 1 ] ;
	long			port ;
	int			sock ;
	struct sockaddr_in	addr ;
	
	int			comm_protocol_mode ;
	
	int			recv_buffer_size ;
	char			*recv_buffer ;
	int			total_recv_len ;
	int			recv_body_len ;
	
	int			send_buffer_size ;
	char			*send_buffer ;
	int			total_send_len ;
	int			send_len ;
	
	char			bak ;
	char			*newline ;
	void			*p1 ;
	void			*p2 ;
	void			*p3 ;
} ;

/********* comm *********/

int InitSocketSession( struct SocketSession *psession );
void CleanSocketSession( struct SocketSession *psession );
void FreeSocketSession( struct SocketSession *psession );
struct SocketSession *AllocSocketSession();
void ResetSocketSession( struct SocketSession *psession );
void CleanSendBuffer( struct SocketSession *psession );
void CleanRecvBuffer( struct SocketSession *psession );
int ReallocSendBuffer( struct SocketSession *psession , int new_buffer_size );
int ReallocRecvBuffer( struct SocketSession *psession , int new_buffer_size );

struct SocketSession *GetUnusedSocketSession( struct SocketSession *p_session_array , int count , struct SocketSession **pp_slibing_session );

int AddInputSockToEpoll( int epoll_socks , struct SocketSession *psession );
int AddOutputSockToEpoll( int epoll_socks , struct SocketSession *psession );
int ModifyInputSockFromEpoll( int epoll_socks , struct SocketSession *psession );
int ModifyOutputSockFromEpoll( int epoll_socks , struct SocketSession *psession );
int DeleteSockFromEpoll( int epoll_socks , struct SocketSession *psession );

int SetAddrReused( int sock );
int SetNonBlocking( int sock );
void SetSocketAddr( struct sockaddr_in *p_sockaddr , char *ip , long port );
void GetSocketAddr( struct sockaddr_in *addr , char *ip , long *port );

int BindListenSocket( char *ip , long port , struct SocketSession *psession );
#define RETURN_CONNECTING_IN_PROGRESS		1
int AsyncConnectSocket( char *ip , long port , struct SocketSession *psession );
int AcceptSocket( int epoll_socks , int listen_sock , struct SocketSession *psession );
int DiscardAcceptSocket( int epoll_socks , int listen_sock );
#define RETURN_CLOSE_PEER			1
typedef int funcDoProtocol( void *_penv , struct SocketSession *psession );
#define OPTION_ASYNC_CHANGE_MODE_FLAG		1
#define RETURN_RECEIVING_IN_PROGRESS		1
#define RETURN_CHANGE_COMM_PROTOCOL_MODE	2
#define RETURN_NO_SEND_RESPONSE			3
#define RETURN_PEER_CLOSED			4
int AsyncReceiveSocketData( int epoll_socks , struct SocketSession *psession , int change_mode_flag );
int AfterDoProtocol( struct SocketSession *psession );
#define OPTION_ASYNC_SKIP_RECV_FLAG		1
int AsyncReceiveCommand( int epoll_socks , struct SocketSession *psession , int skip_recv_flag );
int AfterDoCommandProtocol( struct SocketSession *psession );
#define RETURN_SENDING_IN_PROGRESS		1
int AsyncSendSocketData( int epoll_socks , struct SocketSession *psession );
int SyncConnectSocket( char *ip , long port , struct SocketSession *psession );
int SyncReceiveSocketData( struct SocketSession *psession );
int SyncSendSocketData( struct SocketSession *psession );

/********* proto *********/

void FormatSendHead( struct SocketSession *psession , char *msg_type , int msg_len );

/********* util *********/

int ConvertToDaemonServer();
int ns2i( char *str , int len );
/* char	md5_exp[ MD5_DIGEST_LENGTH * 2 + 1 ] ; */
int FileMd5( char *pathfilename , char *md5_exp );

#endif

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

#ifndef STRCMP
#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef STRICMP
#if ( defined __linux ) || ( defined __unix )
#define STRICMP(_a_,_C_,_b_) ( strcasecmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( strncasecmp(_a_,_b_,_n_) _C_ 0 )
#elif ( defined _WIN32 )
#define STRICMP(_a_,_C_,_b_) ( stricmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( strnicmp(_a_,_b_,_n_) _C_ 0 )
#endif
#endif

#ifndef MEMCMP
#define MEMCMP(_a_,_C_,_b_,_n_) ( memcmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef SNPRINTF
#if ( defined _WIN32 )
#define snprintf	_snprintf
#define vsnprintf	_vsnprintf
#endif
#endif

#include "openssl/md5.h"
#include "openssl/err.h"

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
#define COMMPROTO_HTTP			2

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

/*
 * iLOG3Lite - log function library written in c
 * author	: calvin
 * email	: calvinwilliams.c@gmail.com
 * LastVersion	: v1.0.9
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#if ( defined _WIN32 )
#include <windows.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <syslog.h>
#include <pthread.h>
#endif

/* 公共宏 */
#ifndef MAXLEN_FILENAME
#define MAXLEN_FILENAME			256
#endif

#ifndef STRCMP
#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef MEMCMP
#define MEMCMP(_a_,_C_,_b_,_n_) ( memcmp(_a_,_b_,_n_) _C_ 0 )
#endif

/* 跨平台宏 */
#if ( defined __linux__ ) || ( defined __unix ) || ( defined _AIX )
#define TLS		__thread
#define VSNPRINTF	vsnprintf
#define SNPRINTF	snprintf
#define OPEN		open
#define READ		read
#define WRITE		write
#define CLOSE		close
#define PROCESSID	(unsigned long)getpid()
#define THREADID	(unsigned long)pthread_self()
#define NEWLINE		"\n"
#elif ( defined _WIN32 )
#define TLS		__declspec( thread )
#define VSNPRINTF	_vsnprintf
#define SNPRINTF	_snprintf
#define OPEN		_open
#define READ		_read
#define WRITE		_write
#define CLOSE		_close
#define PROCESSID	(unsigned long)GetCurrentProcessId()
#define THREADID	(unsigned long)GetCurrentThreadId()
#define NEWLINE		"\r\n"
#endif

/* 简单日志函数 */
#ifndef LOGLEVEL_DEBUG
#define LOGLEVEL_DEBUG		0
#define LOGLEVEL_INFO		1
#define LOGLEVEL_WARN		2
#define LOGLEVEL_ERROR		3
#define LOGLEVEL_FATAL		4
#endif

void SetLogFile( char *format , ... );
void SetLogLevel( int log_level );

int WriteLog( int log_level , char *c_filename , long c_fileline , char *format , ... );
int FatalLog( char *c_filename , long c_fileline , char *format , ... );
int ErrorLog( char *c_filename , long c_fileline , char *format , ... );
int WarnLog( char *c_filename , long c_fileline , char *format , ... );
int InfoLog( char *c_filename , long c_fileline , char *format , ... );
int DebugLog( char *c_filename , long c_fileline , char *format , ... );

int WriteHexLog( int log_level , char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... );
int FatalHexLog( char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... );
int ErrorHexLog( char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... );
int WarnHexLog( char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... );
int InfoHexLog( char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... );
int DebugHexLog( char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... );

#if ( defined __STDC_VERSION__ ) && ( __STDC_VERSION__ >= 199901 )

#define WRITELOG(_log_level_,...)	WriteLog( _log_level_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define FATALLOG(...)			FatalLog( __FILE__ , __LINE__ , __VA_ARGS__ );
#define ERRORLOG(...)			ErrorLog( __FILE__ , __LINE__ , __VA_ARGS__ );
#define WARNLOG(...)			WarnLog( __FILE__ , __LINE__ , __VA_ARGS__ );
#define INFOLOG(...)			InfoLog( __FILE__ , __LINE__ , __VA_ARGS__ );
#define DEBUGLOG(...)			DebugLog( __FILE__ , __LINE__ , __VA_ARGS__ );

#define WRITEHEXLOG(_log_level_,_buf_,_buflen_,...)	WriteHexLog( _log_level_ , __FILE__ , __LINE__ , buf , buflen , __VA_ARGS__ );
#define FATALHEXLOG(_buf_,_buflen_,...)	FatalHexLog( __FILE__ , __LINE__ , buf , buflen , __VA_ARGS__ );
#define ERRORHEXLOG(_buf_,_buflen_,...)	ErrorHexLog( __FILE__ , __LINE__ , buf , buflen , __VA_ARGS__ );
#define WARNHEXLOG(_buf_,_buflen_,...)	WarnHexLog( __FILE__ , __LINE__ , buf , buflen , __VA_ARGS__ );
#define INFOHEXLOG(_buf_,_buflen_,...)	InfoHexLog( __FILE__ , __LINE__ , buf , buflen , __VA_ARGS__ );
#define DEBUGHEXLOG(_buf_,_buflen_,...)	DebugHexLog( __FILE__ , __LINE__ , buf , buflen , __VA_ARGS__ );

#endif

#if ( defined __STDC_VERSION__ ) && ( __STDC_VERSION__ >= 199901 )

#define set_log_file		SetLogFile
#define set_log_level		SetLogLevel

#define write_log		WriteLog
#define fatal_log		FatalLog
#define error_log		ErrorLog
#define warn_log		WarnLog
#define info_log		InfoLog
#define debug_log		DebugLog

#define write_hex_log		WriteHexLog
#define fatal_hex_log		FatalHexLog
#define error_hex_log		ErrorHexLog
#define warn_hex_log		WarnHexLog
#define info_hex_log		InfoHexLog
#define debug_hex_log		DebugHexLog

#endif

/*
** 库名		:	ListX
** 库描述	:	用于链表list操作的函数库
** 作者		:	calvin
** E-mail	:	
** QQ		:	
** 创建日期时间	:	2003/10/18
** 更新日期时间	:	2005/5/2
*/

#ifdef _DEFINE_FASTCGI_
#include "fcgi_stdio.h"
#else
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>

/*
** 宏名                 :       动态链接库 导入、导出修饰符
** 更新日志             :       2006/5/9        创建
*/

#ifdef _TYPE_COMPILER_MSC_
#ifndef _WINDLL_EXPORT
#define _WINDLL_EXPORT		_declspec(dllexport)
#endif
#else
#ifndef _WINDLL_EXPORT
#define _WINDLL_EXPORT		extern
#endif
#endif

#ifdef _TYPE_COMPILER_MSC_
#ifndef _WINDLL_IMPORT
#define _WINDLL_IMPORT		_declspec(dllimport)
#endif
#else
#ifndef _WINDLL_IMPORT
#define _WINDLL_IMPORT		extern
#endif
#endif


/*
** 类型名		:	BOOL
** 类型描述		:	用于真假值的类型
** 更新日志		:	2003/10/18	创建
*/

#ifndef _TYPEDEF_BOOL_
#define _TYPEDEF_BOOL_
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOLNULL
#define BOOLNULL -1
#endif

/*
** 类型描述	:	定义用于ListX函数库的结构
**				SList		:	链表结构
**				SListNode	:	链表结点结构
*/

typedef struct tagListNode
{
	long msize;
	void *member;
	BOOL (* FreeNodeMember)( void *pv );

	struct tagListNode *prev;
	struct tagListNode *next;
	
	/* 附加结构成员集 为iDB库 */
	struct tagListNode **_iDB_pplist ;
	short _iDB_action ;
	BOOL _iDB_enableflag ;
}
SList,SListNode;

/*
** 函数集描述	:	用于操作一个泛型的链表list的函数集
*/

_WINDLL_EXPORT SList *CreateList();
_WINDLL_EXPORT BOOL DestroyList( SList **list , BOOL (* FreeNodeMember)( void *pv ) );

_WINDLL_EXPORT SList *AddListNode( SList **list , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );
#define InsertListNode	InsertListNodeBefore
_WINDLL_EXPORT SList *InsertListNodeBefore( SList **list , SListNode **nodeList , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT SList *InsertListNodeAfter( SList **list , SListNode **nodeList , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT SList *InsertListIndexNode( SList **list , int index , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );

_WINDLL_EXPORT BOOL DeleteListNode( SList **list , SListNode **nodeList , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT BOOL DeleteListIndexNode( SList **list , int index , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT BOOL DeleteAllListNode( SList **list , BOOL (* FreeNodeMember)( void *pv ) );

_WINDLL_EXPORT SListNode *FindFirstListNode( SList *list );
_WINDLL_EXPORT SListNode *FindLastListNode( SList *list );
_WINDLL_EXPORT SListNode *FindPrevListNode( SListNode *nodeList );
_WINDLL_EXPORT SListNode *FindNextListNode( SListNode *nodeList );

_WINDLL_EXPORT BOOL IsFirstListNode( SListNode *nodeList );
_WINDLL_EXPORT BOOL IsLastListNode( SListNode *nodeList );
_WINDLL_EXPORT BOOL IsListEmpty( SList *list );

_WINDLL_EXPORT BOOL IsListNodeValid( SListNode *node );

_WINDLL_EXPORT SListNode *GetListIndexNode( SList *list , long position );
_WINDLL_EXPORT void *GetListIndexMember( SList *list , long position );
_WINDLL_EXPORT void *GetNodeMember( SListNode *nodeList );

_WINDLL_EXPORT long CountListNodes( SList *list );

_WINDLL_EXPORT long AccessList( SList *listHead , BOOL (* AccessListNodeProc)( void *member ) );

_WINDLL_EXPORT int SwapNeighborListNodes( SListNode **list , SListNode **ppnode );
_WINDLL_EXPORT int SwapTwoListNodes( SListNode *pnode1 , SListNode *pnode2 );
_WINDLL_EXPORT int SortList( SList *plist , int (* SortListNodeProc)( void *pmember1 , void *pmember2 ) );

_WINDLL_EXPORT BOOL CopyList( SList **pplistSource , SList **pplistDest , BOOL (* CopyListNodeProc)( void *pmemberCopyFrom , long *plmsize , void **ppmemberNew ) );

_WINDLL_EXPORT BOOL JoinList( SList **pplistSource , SList **pplistAddition );
_WINDLL_EXPORT BOOL RuptureList( SList **pplistSource , SListNode *nodeRupture , SList **pplistNew );

_WINDLL_EXPORT BOOL DetachListNode( SList **pplistSource , SListNode *nodeDetach );
_WINDLL_EXPORT BOOL AttachListNodeAfter( SList **pplistSource , SListNode *node , SListNode *nodeAttach );
_WINDLL_EXPORT BOOL AttachListNodeBefore( SList **pplistSource , SListNode *node , SListNode *nodeAttach );

/*
** 函数集描述	:	栈式链表的函数集
*/

_WINDLL_EXPORT SListNode *PushStackList( SList **listHead , long max_len , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT BOOL PopupStackList(SList **listHead , void **member );

/*
** 函数集描述	:	队列式链表的函数集
*/

_WINDLL_EXPORT SListNode *EnterQueueList( SList **listHead , long max_len , void *member , long msize , BOOL (* FreeNodeMember)( void *pv ) );
_WINDLL_EXPORT BOOL LeaveQueueList( SList **listHead , void **member );

#ifdef __cplusplus
}
#endif

#endif

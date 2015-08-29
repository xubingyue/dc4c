/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "server.h"

static struct WorkerInfo *AllocWorkerInfo()
{
	struct WorkerInfo	*p = NULL ;
	
	p = (struct WorkerInfo *)malloc( sizeof(struct WorkerInfo) ) ;
	if( p == NULL )
		return NULL;
	memset( p , 0x00 , sizeof(struct WorkerInfo) );
	
	return p;
}

static BOOL FreeWorkerInfo( void *pv )
{
	struct WorkerInfo	*p = (struct WorkerInfo *) pv ;
	
	free( p );
	
	return TRUE;
}

static struct HostInfo *AllocHostInfo()
{
	struct HostInfo	*p = NULL ;
	
	p = (struct HostInfo *)malloc( sizeof(struct HostInfo) ) ;
	if( p == NULL )
		return NULL;
	memset( p , 0x00 , sizeof(struct HostInfo) );
	
	return p;
}

static BOOL FreeHostInfo( void *pv )
{
	struct HostInfo	*p = (struct HostInfo *) pv ;
	
	DestroyList( & (p->worker_info_list) , NULL );
	free( p );
	
	return TRUE;
}

static struct OsType *AllocOsType()
{
	struct OsType	*p = NULL ;
	
	p = (struct OsType *)malloc( sizeof(struct OsType) ) ;
	if( p == NULL )
		return NULL;
	memset( p , 0x00 , sizeof(struct OsType) );
	
	return p;
}

static BOOL FreeOsType( void *pv )
{
	struct OsType	*p = (struct OsType *) pv ;
	
	DestroyList( & (p->host_info_list) , NULL );
	free( p );
	
	return TRUE;
}

int app_WorkerRegisterRequest( struct ServerEnv *penv , struct SocketSession *psession , worker_register_request *p_req , worker_register_response *p_rsp )
{
	long			timestamp ;
	
	SListNode		*os_type_node = NULL ;
	struct OsType		*os_type = NULL ;
	SListNode		*host_info_node = NULL ;
	struct HostInfo		*host_info = NULL ;
	SListNode		*worker_info_node = NULL ;
	struct WorkerInfo	*worker_info = NULL ;
	
	if( p_req->sysname[0] == '\0' || p_req->release[0] == '\0' || p_req->bits == 0 )
	{
		ErrorLog( __FILE__ , __LINE__ , "sysname[%s] release[%s] bits[%d] invalid" , p_req->sysname , p_req->release , p_req->bits );
		return -1;
	}
	
	timestamp = 0 ;
	time( & timestamp );
	
	for( os_type_node = FindFirstListNode(penv->os_type_list) ; os_type_node ; os_type_node = FindNextListNode(os_type_node) )
	{
		os_type = GetNodeMember(os_type_node) ;
		DebugLog( __FILE__ , __LINE__ , "os_type[%s][%s][%d]" , os_type->sysname , os_type->release , os_type->bits );
		if( STRCMP( os_type->sysname , == , p_req->sysname ) && STRCMP( os_type->release , == , p_req->release ) && os_type->bits == p_req->bits )
		{
			for( host_info_node = FindFirstListNode(os_type->host_info_list) ; host_info_node ; host_info_node = FindNextListNode(host_info_node) )
			{
				host_info = GetNodeMember(host_info_node) ;
				DebugLog( __FILE__ , __LINE__ , "host_info[%s][%d][%d]" , host_info->ip , host_info->idler_count , host_info->working_count );
				if( STRCMP( host_info->ip , == , p_req->ip ) )
				{
					for( worker_info_node = FindFirstListNode(host_info->worker_info_list) ; worker_info_node ; worker_info_node = FindNextListNode(worker_info_node) )
					{
						worker_info = GetNodeMember(worker_info_node) ;
						DebugLog( __FILE__ , __LINE__ , "worker_info[%d][%d][%ld]" , worker_info->port , worker_info->is_working , worker_info->access_timestamp );
						if( worker_info->port == p_req->port )
						{
							ErrorLog( __FILE__ , __LINE__ , "sysname[%s] release[%s] bits[%d] ip[%s] port[%d] duplicated" , p_req->sysname , p_req->release , p_req->bits , p_req->ip , p_req->port );
							p_rsp->error = 1 ;
							return 0;
						}
					}
					if( worker_info_node == NULL )
					{
						worker_info = AllocWorkerInfo() ;
						if( worker_info == NULL )
						{
							ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
							p_rsp->error = 11 ;
							return 0;
						}
						
						worker_info->port = p_req->port ;
						worker_info->is_working = 0 ;
						worker_info->access_timestamp = timestamp ;
						
						worker_info_node = InsertListNodeBefore( & (host_info->worker_info_list) , & (host_info->worker_info_list) , worker_info , sizeof(struct WorkerInfo) , & FreeWorkerInfo ) ;
						if( worker_info_node == NULL )
						{
							ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
							p_rsp->error = 11 ;
							return 0;
						}
						
						DebugLog( __FILE__ , __LINE__ , "Add worker ok , port[%d] in sysname[%s] release[%s] bits[%d] ip[%s]" , p_req->port , p_req->sysname , p_req->release , p_req->bits , p_req->ip );
						
						host_info->idler_count++;
						
						psession->p1 = os_type_node ;
						psession->p2 = host_info_node ;
						psession->p3 = worker_info_node ;
						
						break;
					}
				}
			}
			if( host_info_node == NULL )
			{
				worker_info = AllocWorkerInfo() ;
				if( worker_info == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
					p_rsp->error = 11 ;
					return 0;
				}
				
				worker_info->port = p_req->port ;
				worker_info->is_working = 0 ;
				worker_info->access_timestamp = timestamp ;
				
				host_info = AllocHostInfo() ;
				if( host_info == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
					p_rsp->error = 11 ;
					return 0;
				}
				
				strcpy( host_info->ip , p_req->ip );
				host_info->idler_count = 0 ;
				host_info->working_count = 0 ;
				worker_info_node = AddListNode( & (host_info->worker_info_list) , worker_info , sizeof(struct WorkerInfo) , & FreeWorkerInfo ) ;
				if( worker_info_node == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
					p_rsp->error = 11 ;
					return 0;
				}
				
				host_info_node = AddListNode( & (os_type->host_info_list) , host_info , sizeof(struct HostInfo) , & FreeHostInfo ) ;
				if( host_info_node == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
					p_rsp->error = 11 ;
					return 0;
				}
				
				DebugLog( __FILE__ , __LINE__ , "Add worker ok , ip[%s] port[%d] in sysname[%s] release[%s] bits[%d]" , p_req->ip , p_req->port , p_req->sysname , p_req->release , p_req->bits );
				
				host_info->idler_count++;
				
				psession->p1 = os_type_node ;
				psession->p2 = host_info_node ;
				psession->p3 = worker_info_node ;
			}
			
			break;
		}
	}
	if( os_type_node == NULL )
	{
		worker_info = AllocWorkerInfo() ;
		if( worker_info == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			p_rsp->error = 11 ;
			return 0;
		}
		
		worker_info->port = p_req->port ;
		worker_info->is_working = 0 ;
		worker_info->access_timestamp = timestamp ;
		
		host_info = AllocHostInfo() ;
		if( host_info == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			p_rsp->error = 11 ;
			return 0;
		}
		
		strcpy( host_info->ip , p_req->ip );
		host_info->idler_count = 0 ;
		host_info->working_count = 0 ;
		worker_info_node = AddListNode( & (host_info->worker_info_list) , worker_info , sizeof(struct WorkerInfo) , & FreeWorkerInfo ) ;
		if( worker_info_node == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
			p_rsp->error = 11 ;
			return 0;
		}
		
		os_type = AllocOsType() ;
		if( os_type == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			p_rsp->error = 11 ;
			return 0;
		}
		
		strcpy( os_type->sysname , p_req->sysname );
		strcpy( os_type->release , p_req->release );
		os_type->bits = p_req->bits ;
		host_info_node = AddListNode( & (os_type->host_info_list) , host_info , sizeof(struct HostInfo) , & FreeHostInfo ) ;
		if( host_info_node == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
			p_rsp->error = 11 ;
			return 0;
		}
		
		os_type_node = InsertListNodeBefore( & (penv->os_type_list) , & (penv->os_type_list) , os_type , sizeof(struct OsType) , & FreeOsType ) ;
		if( os_type_node == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
			p_rsp->error = 11 ;
			return 0;
		}
		
		DebugLog( __FILE__ , __LINE__ , "Add worker ok , sysname[%s] release[%s] bits[%d] ip[%s] port[%d]" , p_req->sysname , p_req->release , p_req->bits , p_req->ip , p_req->port );
		
		host_info->idler_count++;
		
		psession->p1 = os_type_node ;
		psession->p2 = host_info_node ;
		psession->p3 = worker_info_node ;
	}
	
	psession->type = SESSIONTYPE_WORKER ;
	
	p_rsp->error = 0 ;
	return 0;
}

int app_QueryWorkersRequest( struct ServerEnv *penv , struct SocketSession *psession , query_workers_request *p_req , query_workers_response *p_rsp )
{
	long			timestamp ;
	
	SListNode		*os_type_node = NULL ;
	struct OsType		*os_type = NULL ;
	SListNode		*host_info_node = NULL ;
	struct HostInfo		*host_info = NULL ;
	SListNode		*worker_info_node = NULL ;
	struct WorkerInfo	*worker_info = NULL ;
	
	int			find_one ;
	SListNode		*last_node = NULL ;
	
	timestamp = 0 ;
	time( & timestamp );
	
	for( os_type_node = FindFirstListNode(penv->os_type_list) ; os_type_node ; os_type_node = FindNextListNode(os_type_node) )
	{
		os_type = GetNodeMember(os_type_node) ;
		DebugLog( __FILE__ , __LINE__ , "os_type[%s][%s][%d]" , os_type->sysname , os_type->release , os_type->bits );
		if( STRCMP( os_type->sysname , == , p_req->sysname ) && STRCMP( os_type->release , == , p_req->release ) && os_type->bits == p_req->bits )
		{
			while
			(
				(
					p_req->count == -1
					||
					( p_req->count != -1 && p_rsp->_nodes_count < p_req->count )
				)
				&&
				p_rsp->_nodes_count < p_rsp->_nodes_size
			)
			{
				find_one = 0 ;
				for( host_info_node = FindFirstListNode(os_type->host_info_list) ; host_info_node ; host_info_node = FindNextListNode(host_info_node) )
				{
					host_info = GetNodeMember(host_info_node) ;
					DebugLog( __FILE__ , __LINE__ , "host_info[%s][%d][%d]" , host_info->ip , host_info->idler_count , host_info->working_count );
					for( worker_info_node = FindFirstListNode(host_info->worker_info_list) ; worker_info_node ; worker_info_node = FindNextListNode(worker_info_node) )
					{
						worker_info = GetNodeMember(worker_info_node) ;
						DebugLog( __FILE__ , __LINE__ , "worker_info[%d][%d][%ld]" , worker_info->port , worker_info->is_working , worker_info->access_timestamp );
						
						if( worker_info->is_working == 0 && worker_info->access_timestamp != timestamp )
						{
							strcpy( p_rsp->nodes[p_rsp->_nodes_count].node.ip , host_info->ip );
							p_rsp->nodes[p_rsp->_nodes_count].node.port = worker_info->port ;
							
							p_rsp->_nodes_count++;
							
							DebugLog( __FILE__ , __LINE__ , "Find a worker_%d ok , sysname[%s] release[%s] bits[%d] ip[%s] port[%d]" , p_rsp->_nodes_count , os_type->sysname , os_type->release , os_type->bits , host_info->ip , worker_info->port );
							
							worker_info->access_timestamp = timestamp ;
							
							last_node = FindLastListNode(host_info->worker_info_list) ;
							if( last_node != worker_info_node )
							{
								DetachListNode( & (host_info->worker_info_list) , worker_info_node );
								AttachListNodeAfter( & (host_info->worker_info_list) , last_node , worker_info_node );
							}
							
							last_node = FindLastListNode(os_type->host_info_list) ;
							if( last_node != host_info_node )
							{
								DetachListNode( & (os_type->host_info_list) , host_info_node );
								AttachListNodeAfter( & (os_type->host_info_list) , last_node , host_info_node );
							}
							
							find_one = 1 ;
							break;
						}
					}
					if( find_one == 1 )
						break;
				}
				if( find_one == 0 )
					break;
			}
			if( p_rsp->_nodes_count > p_rsp->_nodes_size )
				p_rsp->_nodes_count = p_rsp->_nodes_size ;
			
			break;
		}
	}
	if( os_type_node == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "os[%s] main_version[%s] bits[%d] not found" , p_req->sysname , p_req->release , p_req->bits );
		p_rsp->error = 1 ;
		return 0;
	}
	
	return 0;
}

int app_WorkerNoticeRequest( struct ServerEnv *penv , struct SocketSession *psession , worker_notice_request *p_req )
{
	long			timestamp ;
	
	SListNode		*os_type_node = NULL ;
	struct OsType		*os_type = NULL ;
	SListNode		*host_info_node = NULL ;
	struct HostInfo		*host_info = NULL ;
	SListNode		*worker_info_node = NULL ;
	struct WorkerInfo	*worker_info = NULL ;
	
	SListNode		*last_node = NULL ;
	
	SListNode		*next_host_info_node = NULL ;
	struct HostInfo		*next_host_info = NULL ;
	
	timestamp = 0 ;
	time( & timestamp );
	
	for( os_type_node = FindFirstListNode(penv->os_type_list) ; os_type_node ; os_type_node = FindNextListNode(os_type_node) )
	{
		os_type = GetNodeMember(os_type_node) ;
		DebugLog( __FILE__ , __LINE__ , "os_type[%s][%s][%d]" , os_type->sysname , os_type->release , os_type->bits );
		if( STRCMP( os_type->sysname , == , p_req->sysname ) && STRCMP( os_type->release , == , p_req->release ) && os_type->bits == p_req->bits )
		{
			for( host_info_node = FindFirstListNode(os_type->host_info_list) ; host_info_node ; host_info_node = FindNextListNode(host_info_node) )
			{
				host_info = GetNodeMember(host_info_node) ;
				DebugLog( __FILE__ , __LINE__ , "host_info[%s][%d][%d]" , host_info->ip , host_info->idler_count , host_info->working_count );
				if( STRCMP( host_info->ip , == , p_req->ip ) )
				{
					int		working_delta ;
					working_delta = 0 ;
					host_info = GetNodeMember(host_info_node) ;
					for( worker_info_node = FindFirstListNode(host_info->worker_info_list) ; worker_info_node ; worker_info_node = FindNextListNode(worker_info_node) )
					{
						worker_info = GetNodeMember(worker_info_node) ;
						DebugLog( __FILE__ , __LINE__ , "worker_info[%d][%d][%ld]" , worker_info->port , worker_info->is_working , worker_info->access_timestamp );
						if( worker_info->port == p_req->port )
						{
							if( worker_info->is_working == 0 && p_req->is_working == 1 )
							{
								working_delta++;
							}
							else if( worker_info->is_working == 1 && p_req->is_working == 0 )
							{
								working_delta--;
							}
							
							worker_info->is_working = p_req->is_working ;
							strcpy( worker_info->program_and_params , p_req->program_and_params );
							
							DebugLog( __FILE__ , __LINE__ , "worker notice ok , sysname[%s] release[%s] bits[%d] ip[%s] port[%d] is_working[%d] program_and_params[%s]" , os_type->sysname , os_type->release , os_type->bits , host_info->ip , worker_info->port , worker_info->is_working , worker_info->program_and_params );
							
							last_node = FindLastListNode(host_info->worker_info_list) ;
							if( last_node != worker_info_node )
							{
								DetachListNode( & (host_info->worker_info_list) , worker_info_node );
								AttachListNodeAfter( & (host_info->worker_info_list) , last_node , worker_info_node );
							}
							
							break;
						}
					}
					if( worker_info_node == NULL )
					{
						ErrorLog( __FILE__ , __LINE__ , "ip[%s] port[%d] not found" , p_req->ip , p_req->port );
						return 0;
					}
					
					host_info->idler_count -= working_delta ;
					host_info->working_count += working_delta ;
					
					while(1)
					{
						next_host_info_node = FindNextListNode(host_info_node) ;
						if( next_host_info_node == NULL )
							break;
						
						next_host_info = GetNodeMember(next_host_info_node) ;
						
						if( host_info->idler_count >= next_host_info->idler_count )
							break;
						
						SwapNeighborListNodes( & (os_type->host_info_list) , & host_info_node ) ;
					}
					
					return 0;
				}
			}
			if( host_info_node == NULL )
			{
				ErrorLog( __FILE__ , __LINE__ , "ip[%s] not found" , p_req->ip );
				return 0;
			}
		}
	}
	
	ErrorLog( __FILE__ , __LINE__ , "os[%s] main_version[%s] bits[%d] not found" , p_req->sysname , p_req->release , p_req->bits );
	return 0;
}

int app_WorkerUnregister( struct ServerEnv *penv , struct SocketSession *psession )
{
	SListNode		*os_type_node = NULL ;
	struct OsType		*os_type = NULL ;
	SListNode		*host_info_node = NULL ;
	struct HostInfo		*host_info = NULL ;
	SListNode		*worker_info_node = NULL ;
	struct WorkerInfo	*worker_info = NULL ;
	
	if( psession->p1 == NULL || psession->p2 == NULL || psession->p3 == NULL )
		return 0;
	
	os_type_node = (SListNode*)(psession->p1) ;
	os_type = GetNodeMember(os_type_node) ;
	host_info_node = (SListNode*)(psession->p2) ;
	host_info = GetNodeMember(host_info_node) ;
	worker_info_node = (SListNode*)(psession->p3) ;
	worker_info = GetNodeMember(worker_info_node) ;
	
	host_info->idler_count -= (worker_info->is_working?0:1) ;
	host_info->working_count -= (worker_info->is_working?1:0) ;
	
	InfoLog( __FILE__ , __LINE__ , "DeleteListNode sysname[%s] release[%s] bits[%d] ip[%s] - port[%d]" , os_type->sysname , os_type->release , os_type->bits , host_info->ip , worker_info->port );
	DeleteListNode( & (host_info->worker_info_list) , & worker_info_node , NULL );
	if( host_info->worker_info_list == NULL )
	{
		InfoLog( __FILE__ , __LINE__ , "DeleteListNode sysname[%s] release[%s] bits[%d] - ip[%s]" , os_type->sysname , os_type->release , os_type->bits , host_info->ip );
		DeleteListNode( & (os_type->host_info_list) , & host_info_node , NULL );
		if( os_type->host_info_list == NULL )
		{
			InfoLog( __FILE__ , __LINE__ , "DeleteListNode sysname[%s] release[%s] bits[%d]" , os_type->sysname , os_type->release , os_type->bits );
			DeleteListNode( & (penv->os_type_list) , & os_type_node , NULL );
		}
	}
	
	return 0;
}

#define SNPRINTF_ARGS		psession->send_buffer + psession->total_send_len		\
				, psession->send_buffer_size-1 - psession->total_send_len	\

#define SENDBUFFER_APPEND_LEN	if( len > 0 && psession->total_send_len + len < psession->send_buffer_size-1 )	\
					psession->total_send_len += len ;					\

int app_QueryAllOsTypes( struct ServerEnv *penv , struct SocketSession *psession )
{
	SListNode		*os_type_node = NULL ;
	struct OsType		*os_type = NULL ;
	
	int			len ;
	
	for( os_type_node = FindFirstListNode(penv->os_type_list) ; os_type_node ; os_type_node = FindNextListNode(os_type_node) )
	{
		os_type = GetNodeMember(os_type_node) ;
		len = (int)SNPRINTF( SNPRINTF_ARGS , "%s\t%s\t%d\r\n" , os_type->sysname , os_type->release , os_type->bits );
		SENDBUFFER_APPEND_LEN
	}
	
	return 0;
}

int app_QueryAllHosts( struct ServerEnv *penv , struct SocketSession *psession )
{
	SListNode		*os_type_node = NULL ;
	struct OsType		*os_type = NULL ;
	SListNode		*host_info_node = NULL ;
	struct HostInfo		*host_info = NULL ;
	
	int			len ;
	
	for( os_type_node = FindFirstListNode(penv->os_type_list) ; os_type_node ; os_type_node = FindNextListNode(os_type_node) )
	{
		os_type = GetNodeMember(os_type_node) ;
		for( host_info_node = FindFirstListNode(os_type->host_info_list) ; host_info_node ; host_info_node = FindNextListNode(host_info_node) )
		{
			host_info = GetNodeMember(host_info_node) ;
			
			len = (int)SNPRINTF( SNPRINTF_ARGS , "%s\t%s\t%d\t%s\t%d\t%d\r\n" , os_type->sysname , os_type->release , os_type->bits , host_info->ip , host_info->idler_count , host_info->working_count );
			SENDBUFFER_APPEND_LEN
		}
	}
	
	return 0;
}

int app_QueryAllWorkers( struct ServerEnv *penv , struct SocketSession *psession )
{
	SListNode		*os_type_node = NULL ;
	struct OsType		*os_type = NULL ;
	SListNode		*host_info_node = NULL ;
	struct HostInfo		*host_info = NULL ;
	SListNode		*worker_info_node = NULL ;
	struct WorkerInfo	*worker_info = NULL ;
	
	int			len ;
	
	for( os_type_node = FindFirstListNode(penv->os_type_list) ; os_type_node ; os_type_node = FindNextListNode(os_type_node) )
	{
		os_type = GetNodeMember(os_type_node) ;
		for( host_info_node = FindFirstListNode(os_type->host_info_list) ; host_info_node ; host_info_node = FindNextListNode(host_info_node) )
		{
			host_info = GetNodeMember(host_info_node) ;
			for( worker_info_node = FindFirstListNode(host_info->worker_info_list) ; worker_info_node ; worker_info_node = FindNextListNode(worker_info_node) )
			{
				worker_info = GetNodeMember(worker_info_node) ;
				len = (int)SNPRINTF( SNPRINTF_ARGS , "%s\t%s\t%d\t%s\t%d\t%d\t%s\r\n" , os_type->sysname , os_type->release , os_type->bits , host_info->ip , worker_info->port , worker_info->is_working , worker_info->program_and_params );
				SENDBUFFER_APPEND_LEN
			}
		}
	}
	
	return 0;
}

#define HTML_BGCOLOR		"#EEF3F7"
#define HTML_BORDERCOLOR	"#A5B6C8"
#define HTML_FONTSIZE		"16px"
#define HTML_FONTFAMILY		"Fixedsys, System, Terminal"

int app_MonitorHtmlFrame( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			len ;
	int			httphead_len ;
	char			Content_Length[ 11 + 1 ] ;
	
	httphead_len = len = (int)SNPRINTF( SNPRINTF_ARGS ,
		"HTTP/1.0 200 OK\r\nContent-Length: 1234567890\r\n"
		"Cache-Control: no-cache,must-revalidate\r\n\r\n"
		);
	SENDBUFFER_APPEND_LEN
	
	len = (int)SNPRINTF( SNPRINTF_ARGS , 
		"<html>\n"
		"<head>\n"
		"<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n"
		"<title>DC4C Servers Monitor</title>\n"
		"<style type='text/css'>\n"
		"body {\n"
		"	font-size: "HTML_FONTSIZE";\n"
		"	font-family: "HTML_FONTFAMILY";\n"
		"}\n"
		"</style>\n"
		"</head>\n"
		"<script language='JavaScript'>\n"
		"var xmlhttpOsTypesList ;\n"
		"var xmlhttpHostsList ;\n"
		"var xmlhttpWorkersList ;\n"
		"function createXmlHttp()\n"
		"{\n"
		"	var xmlhttp ;\n"
		"	try\n"
		"	{\n"
		"		xmlhttp = new XMLHttpRequest();\n"
		"	}\n"
		"	catch ( trymicrosoft )\n"
		"	{\n"
		"		try\n"
		"		{\n"
		"			xmlhttp = new ActiveXObject('Msxml2.XMLHTTP') ;\n"
		"		}\n"
		"		catch ( othermicrosoft )\n"
		"		{\n"
		"			try\n"
		"			{\n"
		"				xmlhttp = new ActiveXObject('Microsoft.XMLHTTP') ;\n"
		"			}\n"
		"			catch ( failed )\n"
		"			{\n"
		"				xmlhttp = null ;\n"
		"			}\n"
		"		}\n"
		"	}\n"
		"	return xmlhttp;\n"
		"}\n"
		"function refreshList(xmlhttp,url,list_id)\n"
		"{\n"
		"	xmlhttp.open( 'GET' , url , true );\n"
		"	xmlhttp.onreadystatechange = function()\n"
		"		{\n"
		"			if( xmlhttp.readyState == 4 )\n"
		"			{\n"
		"				if( xmlhttp.status == 200 )\n"
		"				{\n"
		"					document.getElementById(list_id).innerHTML = xmlhttp.responseText ;\n"
		"				}\n"
		"				else\n"
		"				{\n"
		"					if( xmlhttp == xmlhttpOsTypesList )\n"
		"						document.getElementById(list_id).innerHTML = \"<font color='"HTML_BORDERCOLOR"'>No wservers connected</font>\" ;\n"
		"					else\n"
		"						document.getElementById(list_id).innerHTML = \"\" ;\n"
		"				}\n"
		"			}\n"
		"		}\n"
		"	xmlhttp.setRequestHeader('If-Modified-Since','0');\n"
		"	xmlhttp.send(null);\n"
		"}\n"
		"function initXmlHttpEnv()\n"
		"{\n"
		"	xmlhttpOsTypesList = createXmlHttp() ;\n"
		"	xmlhttpHostsList = createXmlHttp() ;\n"
		"	xmlhttpWorkersList = createXmlHttp() ;\n"
		"	setInterval('refreshList(xmlhttpOsTypesList,\"/ostypesList\",\"ostypesList\");',1000);\n"
		"	setInterval('refreshList(xmlhttpHostsList,\"/hostsList\",\"hostsList\");',1000);\n"
		"	setInterval('refreshList(xmlhttpWorkersList,\"/workersList\",\"workersList\");',1000);\n"
		"}\n"
		"</script>\n"
		"<body onLoad='initXmlHttpEnv();'>\n"
		"<font color='"HTML_BORDERCOLOR"'>"
		"rserver v%s build %s %s<br />\n"
		"Copyright by calvin<calvinwilliams.c@gmail.com> 2014,2015<br />\n"
		"</font>\n"
		"<p />\n"
		"<div id='ostypesList'></div>\n"
		"<p />\n"
		"<div id='hostsList'></div>\n"
		"<p />\n"
		"<div id='workersList'></div>\n"
		"</body>\n"
		"</html>\n"
		, __DC4C_VERSION , __DATE__ , __TIME__
		);
	SENDBUFFER_APPEND_LEN
	
	memset( Content_Length , 0x00 , sizeof(Content_Length) );
	SNPRINTF( Content_Length , sizeof(Content_Length)-1 , "%-10d" , psession->total_send_len - httphead_len );
	memcpy( psession->send_buffer + 33 , Content_Length , 10 );
	
	return 0;
}

int app_OutputOsTypesListHtml( struct ServerEnv *penv , struct SocketSession *psession )
{
	SListNode		*os_type_node = NULL ;
	struct OsType		*os_type = NULL ;
	int			len ;
	int			httphead_len ;
	char			Content_Length[ 11 + 1 ] ;
	
	httphead_len = len = (int)SNPRINTF( SNPRINTF_ARGS ,
		"HTTP/1.0 200 OK\r\nContent-Length: 1234567890\r\n"
		"Cache-Control: no-cache,must-revalidate\r\n\r\n"
		);
	SENDBUFFER_APPEND_LEN
	
	len = (int)SNPRINTF( SNPRINTF_ARGS , 
		"<table border='1' bordercolor='"HTML_BORDERCOLOR"' cellpadding='5' cellspacing='0' style='border-collapse:collapse;'>\n"
		"  <tr style='background-color:"HTML_BGCOLOR"; color:"HTML_BORDERCOLOR";'>\n"
		"    <th colSpan=3>Os Types List</th>\n"
		"  </tr>\n"
		"  <tr style='background-color:"HTML_BGCOLOR"; color:"HTML_BORDERCOLOR";'>\n"
		"    <th>sysname</th>\n"
		"    <th>release</th>\n"
		"    <th>bits</th>\n"
		"  </tr>\n"
		) ;
	SENDBUFFER_APPEND_LEN
	
	for( os_type_node = FindFirstListNode(penv->os_type_list) ; os_type_node ; os_type_node = FindNextListNode(os_type_node) )
	{
		os_type = GetNodeMember(os_type_node) ;
		
		len = (int)SNPRINTF( SNPRINTF_ARGS , 
			"  <tr style='color:"HTML_BORDERCOLOR";'>\n"
			"    <td>%s</td>\n"
			"    <td>%s</td>\n"
			"    <td>%d</td>\n"
			"  </tr>\n"
			, os_type->sysname , os_type->release , os_type->bits
			) ;
		SENDBUFFER_APPEND_LEN
	}
	
	len = (int)SNPRINTF( SNPRINTF_ARGS , 
		"</table>\n"
		);
	SENDBUFFER_APPEND_LEN
	
	memset( Content_Length , 0x00 , sizeof(Content_Length) );
	SNPRINTF( Content_Length , sizeof(Content_Length)-1 , "%-10d" , psession->total_send_len - httphead_len );
	memcpy( psession->send_buffer + 33 , Content_Length , 10 );
	
	return 0;
}

int app_OutputHostsListHtml( struct ServerEnv *penv , struct SocketSession *psession )
{
	SListNode		*os_type_node = NULL ;
	struct OsType		*os_type = NULL ;
	SListNode		*host_info_node = NULL ;
	struct HostInfo		*host_info = NULL ;
	int			len ;
	int			httphead_len ;
	char			Content_Length[ 11 + 1 ] ;
	
	httphead_len = len = (int)SNPRINTF( SNPRINTF_ARGS ,
		"HTTP/1.0 200 OK\r\nContent-Length: 1234567890\r\n"
		"Cache-Control: no-cache,must-revalidate\r\n\r\n"
		);
	SENDBUFFER_APPEND_LEN
	
	len = (int)SNPRINTF( SNPRINTF_ARGS , 
		"<table border='1' bordercolor='"HTML_BORDERCOLOR"' cellpadding='5' cellspacing='0' style='border-collapse:collapse;'>\n"
		"  <tr style='background-color:"HTML_BGCOLOR"; color:"HTML_BORDERCOLOR";'>\n"
		"    <th colSpan=6>Hosts List</th>\n"
		"  </tr>\n"
		"  <tr style='background-color:"HTML_BGCOLOR"; color:"HTML_BORDERCOLOR";'>\n"
		"    <th>sysname</th>\n"
		"    <th>release</th>\n"
		"    <th>bits</th>\n"
		"    <th>ip</th>\n"
		"    <th>idler_count</th>\n"
		"    <th>working_count</th>\n"
		"  </tr>\n"
		) ;
	SENDBUFFER_APPEND_LEN
	
	for( os_type_node = FindFirstListNode(penv->os_type_list) ; os_type_node ; os_type_node = FindNextListNode(os_type_node) )
	{
		os_type = GetNodeMember(os_type_node) ;
		for( host_info_node = FindFirstListNode(os_type->host_info_list) ; host_info_node ; host_info_node = FindNextListNode(host_info_node) )
		{
			host_info = GetNodeMember(host_info_node) ;
			
			len = (int)SNPRINTF( SNPRINTF_ARGS , 
				"  <tr style='color:"HTML_BORDERCOLOR";'>\n"
				"    <td>%s</td>\n"
				"    <td>%s</td>\n"
				"    <td>%d</td>\n"
				"    <td>%s</td>\n"
				"    <td>%d</td>\n"
				"    <td>%d</td>\n"
				"  </tr>\n"
				 , os_type->sysname , os_type->release , os_type->bits , host_info->ip , host_info->idler_count , host_info->working_count
				) ;
			SENDBUFFER_APPEND_LEN
		}
	}
	
	len = (int)SNPRINTF( SNPRINTF_ARGS , 
		"</table>\n"
		);
	SENDBUFFER_APPEND_LEN
	
	memset( Content_Length , 0x00 , sizeof(Content_Length) );
	SNPRINTF( Content_Length , sizeof(Content_Length)-1 , "%-10d" , psession->total_send_len - httphead_len );
	memcpy( psession->send_buffer + 33 , Content_Length , 10 );
	
	return 0;
}

int app_OutputWorkersListHtml( struct ServerEnv *penv , struct SocketSession *psession )
{
	SListNode		*os_type_node = NULL ;
	struct OsType		*os_type = NULL ;
	SListNode		*host_info_node = NULL ;
	struct HostInfo		*host_info = NULL ;
	SListNode		*worker_info_node = NULL ;
	struct WorkerInfo	*worker_info = NULL ;
	int			len ;
	int			httphead_len ;
	char			Content_Length[ 11 + 1 ] ;
	
	httphead_len = len = (int)SNPRINTF( SNPRINTF_ARGS ,
		"HTTP/1.0 200 OK\r\nContent-Length: 1234567890\r\n"
		"Cache-Control: no-cache, must-revalidate\r\n\r\n"
		);
	SENDBUFFER_APPEND_LEN
	
	len = (int)SNPRINTF( SNPRINTF_ARGS , 
		"<table border='1' bordercolor='"HTML_BORDERCOLOR"' cellpadding='5' cellspacing='0' style='border-collapse:collapse;'>\n"
		"  <tr style='background-color:"HTML_BGCOLOR"; color:"HTML_BORDERCOLOR";'>\n"
		"    <th colSpan=7>Workers List</th>\n"
		"  </tr>\n"
		"  <tr style='background-color:"HTML_BGCOLOR"; color:"HTML_BORDERCOLOR";'>\n"
		"    <th>sysname</th>\n"
		"    <th>release</th>\n"
		"    <th>bits</th>\n"
		"    <th>ip</th>\n"
		"    <th>port</th>\n"
		"    <th>is_working</th>\n"
		"    <th>program_and_params</th>\n"
		"  </tr>\n"
		) ;
	SENDBUFFER_APPEND_LEN
	
	for( os_type_node = FindFirstListNode(penv->os_type_list) ; os_type_node ; os_type_node = FindNextListNode(os_type_node) )
	{
		os_type = GetNodeMember(os_type_node) ;
		for( host_info_node = FindFirstListNode(os_type->host_info_list) ; host_info_node ; host_info_node = FindNextListNode(host_info_node) )
		{
			host_info = GetNodeMember(host_info_node) ;
			for( worker_info_node = FindFirstListNode(host_info->worker_info_list) ; worker_info_node ; worker_info_node = FindNextListNode(worker_info_node) )
			{
				worker_info = GetNodeMember(worker_info_node) ;
				
				len = (int)SNPRINTF( SNPRINTF_ARGS , 
					"  <tr style='color:"HTML_BORDERCOLOR";'>\n"
					"    <td>%s</td>\n"
					"    <td>%s</td>\n"
					"    <td>%d</td>\n"
					"    <td>%s</td>\n"
					"    <td>%d</td>\n"
					"    <td>%d</td>\n"
					"    <td>%s</td>\n"
					"  </tr>\n"
					 , os_type->sysname , os_type->release , os_type->bits , host_info->ip , worker_info->port , worker_info->is_working , worker_info->program_and_params
					) ;
				SENDBUFFER_APPEND_LEN
			}
		}
	}
	
	len = (int)SNPRINTF( SNPRINTF_ARGS , 
		"</table>\n"
		);
	SENDBUFFER_APPEND_LEN
	
	memset( Content_Length , 0x00 , sizeof(Content_Length) );
	SNPRINTF( Content_Length , sizeof(Content_Length)-1 , "%-10d" , psession->total_send_len - httphead_len );
	memcpy( psession->send_buffer + 33 , Content_Length , 10 );
	
	return 0;
}

int app_OutputErrorHtml( struct ServerEnv *penv , struct SocketSession *psession )
{
	int			len ;
	int			httphead_len ;
	
	httphead_len = len = (int)SNPRINTF( SNPRINTF_ARGS ,
		"HTTP/1.0 404 Not found\r\n\r\n"
		);
	SENDBUFFER_APPEND_LEN
	
	return 0;
}

int app_HeartBeatRequest( struct ServerEnv *penv , long *p_now , long *p_epoll_timeout )
{
	struct SocketSession	*psession ;
	int			wserver_index ;
	long			try_timeout ;
	
	(*p_epoll_timeout) = SEND_HEARTBEAT_INTERVAL ;
	
	for( wserver_index = 0 , psession = penv->accepted_session_array ; wserver_index < MAXCOUNT_ACCEPTED_SESSION ; wserver_index++ , psession++ )
	{
		if( psession->type == SESSIONTYPE_WORKER )
		{
			if( psession->heartbeat_lost_count > MAXCNT_HEARTBEAT_LOST )
			{
				ErrorLog( __FILE__ , __LINE__ , "heartbeat_lost_count[%d] > [%d]" , psession->heartbeat_lost_count , MAXCNT_HEARTBEAT_LOST );
				comm_CloseAcceptedSocket( penv , psession );
			}
			
			if( (*p_now) - psession->active_timestamp >= SEND_HEARTBEAT_INTERVAL )
			{
				proto_HeartBeatRequest( penv , psession );
				ModifyOutputSockFromEpoll( penv->epoll_socks , psession );
				
				psession->active_timestamp = (*p_now) ;
				DebugLog( __FILE__ , __LINE__ , "heartbeat_lost_count[%d]->[%d]" , psession->heartbeat_lost_count , psession->heartbeat_lost_count + 1 );
				psession->heartbeat_lost_count++;
			}
			else /* tt - psession->active_timestamp < SEND_HEARTBEAT_INTERVAL */
			{
				try_timeout = SEND_HEARTBEAT_INTERVAL - ( (*p_now) - psession->active_timestamp ) ;
				if( try_timeout < (*p_epoll_timeout) )
					(*p_epoll_timeout) = try_timeout ;
			}
		}
	}
	
	return 0;
}

int app_HeartBeatResponse( struct ServerEnv *penv , struct SocketSession *psession )
{
	DebugLog( __FILE__ , __LINE__ , "heartbeat_lost_count[%d]->[%d]" , psession->heartbeat_lost_count , 0 );
	psession->heartbeat_lost_count = 0 ;
	
	return 0;
}


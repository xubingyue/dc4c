#include "rserver.h"

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
							p_rsp->response_code = 1 ;
							return 0;
						}
					}
					if( worker_info_node == NULL )
					{
						worker_info = AllocWorkerInfo() ;
						if( worker_info == NULL )
						{
							ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
							p_rsp->response_code = 11 ;
							return 0;
						}
						
						worker_info->port = p_req->port ;
						worker_info->is_working = 0 ;
						worker_info->access_timestamp = timestamp ;
						
						worker_info_node = InsertListNodeBefore( & (host_info->worker_info_list) , & (host_info->worker_info_list) , worker_info , sizeof(struct WorkerInfo) , & FreeWorkerInfo ) ;
						if( worker_info_node == NULL )
						{
							ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
							p_rsp->response_code = 11 ;
							return 0;
						}
						
						host_info->idler_count++;
						
						DebugLog( __FILE__ , __LINE__ , "Add worker ok , port[%d] in sysname[%s] release[%s] bits[%d] ip[%s]" , p_req->port , p_req->sysname , p_req->release , p_req->bits , p_req->ip );
						
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
					p_rsp->response_code = 11 ;
					return 0;
				}
				
				worker_info->port = p_req->port ;
				worker_info->is_working = 0 ;
				worker_info->access_timestamp = timestamp ;
				
				host_info = AllocHostInfo() ;
				if( host_info == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
					p_rsp->response_code = 11 ;
					return 0;
				}
				
				strcpy( host_info->ip , p_req->ip );
				host_info->idler_count = 0 ;
				host_info->working_count = 0 ;
				worker_info_node = AddListNode( & (host_info->worker_info_list) , worker_info , sizeof(struct WorkerInfo) , & FreeWorkerInfo ) ;
				if( worker_info_node == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
					p_rsp->response_code = 11 ;
					return 0;
				}
				
				host_info_node = AddListNode( & (os_type->host_info_list) , host_info , sizeof(struct HostInfo) , & FreeHostInfo ) ;
				if( host_info_node == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
					p_rsp->response_code = 11 ;
					return 0;
				}
				
				DebugLog( __FILE__ , __LINE__ , "Add worker ok , ip[%s] port[%d] in sysname[%s] release[%s] bits[%d]" , p_req->ip , p_req->port , p_req->sysname , p_req->release , p_req->bits );
				
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
			p_rsp->response_code = 11 ;
			return 0;
		}
		
		worker_info->port = p_req->port ;
		worker_info->is_working = 0 ;
		worker_info->access_timestamp = timestamp ;
		
		host_info = AllocHostInfo() ;
		if( host_info == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			p_rsp->response_code = 11 ;
			return 0;
		}
		
		strcpy( host_info->ip , p_req->ip );
		host_info->idler_count = 0 ;
		host_info->working_count = 0 ;
		worker_info_node = AddListNode( & (host_info->worker_info_list) , worker_info , sizeof(struct WorkerInfo) , & FreeWorkerInfo ) ;
		if( worker_info_node == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
			p_rsp->response_code = 11 ;
			return 0;
		}
		
		os_type = AllocOsType() ;
		if( os_type == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			p_rsp->response_code = 11 ;
			return 0;
		}
		
		strcpy( os_type->sysname , p_req->sysname );
		strcpy( os_type->release , p_req->release );
		os_type->bits = p_req->bits ;
		host_info_node = AddListNode( & (os_type->host_info_list) , host_info , sizeof(struct HostInfo) , & FreeHostInfo ) ;
		if( host_info_node == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
			p_rsp->response_code = 11 ;
			return 0;
		}
		
		os_type_node = InsertListNodeBefore( & (penv->os_type_list) , & (penv->os_type_list) , os_type , sizeof(struct OsType) , & FreeOsType ) ;
		if( os_type_node == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "AddListNode failed , errno[%d]" , errno );
			p_rsp->response_code = 11 ;
			return 0;
		}
		
		DebugLog( __FILE__ , __LINE__ , "Add worker ok , sysname[%s] release[%s] bits[%d] ip[%s] port[%d]" , p_req->sysname , p_req->release , p_req->bits , p_req->ip , p_req->port );
		
		psession->p1 = os_type_node ;
		psession->p2 = host_info_node ;
		psession->p3 = worker_info_node ;
	}
	
	p_rsp->response_code = 0 ;
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
				p_rsp->_nodes_count <= p_rsp->_nodes_size
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
							
							find_one = 1 ;
							break;
						}
					}
					
					if( find_one == 1 )
						break;
				}
			}
			if( p_rsp->_nodes_count > p_rsp->_nodes_size )
				p_rsp->_nodes_count = p_rsp->_nodes_size ;
			
			break;
		}
	}
	if( os_type_node == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "os[%s] main_version[%s] bits[%d] not found" , p_req->sysname , p_req->release , p_req->bits );
		p_rsp->response_code = 4 ;
		return 0;
	}
	
	p_rsp->count = p_rsp->_nodes_count ;
	
	return 0;
}

int app_WorkerNoticeRequest( struct ServerEnv *penv , struct SocketSession *psession , worker_notice_request *p_req , worker_notice_response *p_rsp )
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
					host_info = GetNodeMember(host_info_node) ;
					for( worker_info_node = FindFirstListNode(host_info->worker_info_list) ; worker_info_node ; worker_info_node = FindNextListNode(worker_info_node) )
					{
						worker_info = GetNodeMember(worker_info_node) ;
						DebugLog( __FILE__ , __LINE__ , "worker_info[%d][%d][%ld]" , worker_info->port , worker_info->is_working , worker_info->access_timestamp );
						
						if( worker_info->port == p_req->port )
						{
							worker_info->is_working = p_req->is_working ;
							
							DebugLog( __FILE__ , __LINE__ , "worker notice ok , sysname[%s] release[%s] bits[%d] ip[%s] port[%d] is_working[%d]" , os_type->sysname , os_type->release , os_type->bits , host_info->ip , worker_info->port , worker_info->is_working );
							
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
						p_rsp->response_code = 1 ;
						return 0;
					}
					
					host_info->idler_count += (p_req->is_working?-1:1) ;
					host_info->working_count += (p_req->is_working?1:-1) ;
					
					while(1)
					{
						next_host_info_node = FindNextListNode(host_info_node) ;
						if( next_host_info_node == NULL )
							break;
						
						next_host_info = GetNodeMember(next_host_info_node) ;
						
						if( host_info->idler_count >= next_host_info->idler_count )
							break;
						
						SwapTwoListNodes( host_info_node , next_host_info_node );
						host_info_node = FindNextListNode(host_info_node) ;
					}
					
					p_rsp->response_code = 0 ;
					return 0;
				}
			}
			if( host_info_node == NULL )
			{
				ErrorLog( __FILE__ , __LINE__ , "ip[%s] not found" , p_req->ip );
				p_rsp->response_code = 1 ;
				return 0;
			}
		}
	}
	
	ErrorLog( __FILE__ , __LINE__ , "os[%s] main_version[%s] bits[%d] not found" , p_req->sysname , p_req->release , p_req->bits );
	p_rsp->response_code = 1 ;
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
				
				len = (int)SNPRINTF( psession->send_buffer + psession->total_send_len
					, psession->send_buffer_size-1 - psession->total_send_len
					, "%s\t%s\t%d\t%s\t%d\t%d\r\n"
					, os_type->sysname , os_type->release , os_type->bits , host_info->ip , worker_info->port , worker_info->is_working );
				psession->total_send_len += len ;
			}
		}
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
			
			len = (int)SNPRINTF( psession->send_buffer + psession->total_send_len
				, psession->send_buffer_size-1 - psession->total_send_len
				, "%s\t%s\t%d\t%s\t%d\t%d\r\n"
				, os_type->sysname , os_type->release , os_type->bits , host_info->ip , host_info->idler_count , host_info->working_count );
			psession->total_send_len += len ;
		}
	}
	
	return 0;
}

int app_QueryAllOsTypes( struct ServerEnv *penv , struct SocketSession *psession )
{
	SListNode		*os_type_node = NULL ;
	struct OsType		*os_type = NULL ;
	
	int			len ;
	
	for( os_type_node = FindFirstListNode(penv->os_type_list) ; os_type_node ; os_type_node = FindNextListNode(os_type_node) )
	{
		os_type = GetNodeMember(os_type_node) ;
		
		len = (int)SNPRINTF( psession->send_buffer + psession->total_send_len
			, psession->send_buffer_size-1 - psession->total_send_len
			, "%s\t%s\t%d\r\n"
			, os_type->sysname , os_type->release , os_type->bits );
		psession->total_send_len += len ;
	}
	
	return 0;
}


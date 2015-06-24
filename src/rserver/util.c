/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "server.h"

int AddInputSockToEpoll( int epoll_socks , struct SocketSession *psession )
{
	struct epoll_event	event ;
	
	int			nret = 0 ;
	
	memset( & event , 0x00 , sizeof(event) );
	event.data.ptr = psession ;
	event.events = ( EPOLLIN | EPOLLERR ) ;
	nret = epoll_ctl( epoll_socks , EPOLL_CTL_ADD , psession->sock , & event ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "add sock[%d] to wait EPOLLIN event failed[%d] , errno[%d]" , psession->sock , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "add sock[%d] to wait EPOLLIN event" , psession->sock );
		return 0;
	}
}

int AddOutputSockToEpoll( int epoll_socks , struct SocketSession *psession )
{
	struct epoll_event	event ;
	
	int			nret = 0 ;
	
	memset( & event , 0x00 , sizeof(event) );
	event.data.ptr = psession ;
	event.events = ( EPOLLOUT | EPOLLERR ) ;
	nret = epoll_ctl( epoll_socks , EPOLL_CTL_ADD , psession->sock , & event ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "add sock[%d] to wait EPOLLOUT event failed[%d] , errno[%d]" , psession->sock , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "add sock[%d] to wait EPOLLOUT event" , psession->sock );
		return 0;
	}
}

int ModifyInputSockFromEpoll( int epoll_socks , struct SocketSession *psession )
{
	struct epoll_event	event ;
	
	int			nret = 0 ;
	
	memset( & event , 0x00 , sizeof(event) );
	event.data.ptr = psession ;
	event.events = ( EPOLLIN | EPOLLERR ) ;
	nret = epoll_ctl( epoll_socks , EPOLL_CTL_MOD , psession->sock , & event ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "modify sock[%d] to wait EPOLLIN event failed[%d] , errno[%d]" , psession->sock , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "modify sock[%d] to wait EPOLLIN event" , psession->sock );
		return 0;
	}
}

int ModifyOutputSockFromEpoll( int epoll_socks , struct SocketSession *psession )
{
	struct epoll_event	event ;
	
	int			nret = 0 ;
	
	memset( & event , 0x00 , sizeof(event) );
	event.data.ptr = psession ;
	event.events = ( EPOLLOUT | EPOLLERR ) ;
	nret = epoll_ctl( epoll_socks , EPOLL_CTL_MOD , psession->sock , & event ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "modify sock[%d] to wait EPOLLOUT event failed[%d] , errno[%d]" , psession->sock , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "modify sock[%d] to wait EPOLLOUT event" , psession->sock );
		return 0;
	}
}

int DeleteSockFromEpoll( int epoll_socks , struct SocketSession *psession )
{
	int			nret = 0 ;
	
	nret = epoll_ctl( epoll_socks , EPOLL_CTL_DEL , psession->sock , NULL ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "delete sock[%d] from epoll failed[%d] , errno[%d]" , psession->sock , nret , errno );
		return -1;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "delete sock[%d] from epoll" , psession->sock );
		return 0;
	}
}


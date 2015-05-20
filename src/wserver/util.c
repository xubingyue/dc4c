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
	event.events = EPOLLIN | EPOLLERR ;
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
	event.events = EPOLLOUT | EPOLLERR ;
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
	event.events = EPOLLIN | EPOLLERR ;
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
	event.events = EPOLLOUT | EPOLLERR ;
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
		CloseSocket( psession );
		return 0;
	}
}

int lock_file( int *p_fd )
{
	mode_t		m ;
	struct flock	lock ;
	
	int		nret = 0 ;
	
	m=umask(0);
	(*p_fd) = open( "/tmp/dc4c.lock" , O_CREAT|O_WRONLY , S_IRWXU|S_IRWXG|S_IRWXO ) ;
	umask(m);
	if( (*p_fd) == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "open lock file failed , errno[%d]" , errno );
		return -1;
	}
	
	memset( & (lock) , 0x00 , sizeof(lock) );
	lock.l_type = F_WRLCK ;
	lock.l_whence = SEEK_SET ;
	lock.l_start = 0 ;
	lock.l_len = 0 ;
	nret = fcntl( (*p_fd) , F_SETLKW , & (lock) ) ;
	if( nret == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "lock file failed , errno[%d]" , errno );
		return -2;
	}
	
	return 0;
}

int unlock_file( int *p_fd )
{
	struct flock	lock ;
	
	int		nret = 0 ;
	
	memset( & (lock) , 0x00 , sizeof(lock) );
	lock.l_type = F_UNLCK ;
	lock.l_whence = SEEK_SET ;
	lock.l_start = 0 ;
	lock.l_len = 0 ;
	nret = fcntl( (*p_fd) , F_SETLKW , & (lock) ) ;
	if( nret == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "unlock file failed , errno[%d]" , errno );
		return -2;
	}
	
	close( (*p_fd) );
	
	return 0;
}

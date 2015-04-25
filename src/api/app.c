#include "dc4c_api.h"

int app_QueryWorkersRequest( struct SocketSession *psession , query_workers_request *p_req )
{
	struct utsname	uts ;
	
	int		nret = 0 ;
	
	nret = uname( & uts ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "uname failed[%d]" , nret );
		return -1;
	}
	
	strcpy( p_req->sysname , uts.sysname );
	strcpy( p_req->release , uts.release );
	p_req->bits = sizeof(long) * 8 ;
	
	p_req->count = 1 ;
	
	return 0;
}

int app_ExecuteProgramRequest( struct SocketSession *psession , execute_program_request *p_req , char *program , char *params )
{
	char		pathfilename[ MAXLEN_FILENAME + 1 ] ;
	
	int		nret = 0 ;
	
	strcpy( p_req->program , program );
	
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	SNPRINTF( pathfilename , sizeof(pathfilename)-1 , "%s/bin/%s" , getenv("HOME") , program );
	memset( p_req->md5_exp , 0x00 , sizeof(p_req->md5_exp) );
	nret = FileMd5( pathfilename , p_req->md5_exp ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "FileMd5 failed[%d]" , nret );
		return DC4C_ERROR_FILE_NOT_EXIST;
	}
	
	strcpy( p_req->params , params );
	
	return 0;
}


#include "wserver.h"

int proto_ExecuteProgramResponse( struct ServerEnv *penv , struct SocketSession *psession , int status );
int proto_DeployProgramRequest( struct ServerEnv *penv , struct SocketSession *psession , execute_program_request *p_req );
int proto_WorkerNoticeRequest( struct ServerEnv *penv , struct SocketSession *psession );

int app_WorkerRegisterRequest( struct ServerEnv *penv , struct SocketSession *psession , worker_register_request *p_req )
{
	struct utsname		uts ;
	
	int			nret = 0 ;
	
	nret = uname( & uts ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "uname failed[%d]" , nret );
		return -1;
	}
	
	strcpy( p_req->sysname , uts.sysname );
	strcpy( p_req->release , uts.release );
	p_req->bits = sizeof(long) * 8 ;
	strcpy( p_req->ip , penv->param.wserver_ip );
	p_req->port = penv->param.wserver_port ;
	
	return 0;
}

int app_WorkerRegisterResponse( struct ServerEnv *penv , struct SocketSession *psession , worker_register_response *p_rsp )
{
	if( p_rsp->response_code )
	{
		ErrorLog( __FILE__ , __LINE__ , "response_code[%d]" , p_rsp->response_code );
		return -1;
	}
	
	penv->connect_progress = 2 ;
	
	return 0;
}

int app_WorkerNoticeRequest( struct ServerEnv *penv , struct SocketSession *psession , worker_notice_request *p_req )
{
	struct utsname		uts ;
	
	int			nret = 0 ;
	
	nret = uname( & uts ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "uname failed[%d]" , nret );
		return -1;
	}
	
	strcpy( p_req->sysname , uts.sysname );
	strcpy( p_req->release , uts.release );
	p_req->bits = sizeof(long) * 8 ;
	strcpy( p_req->ip , penv->param.wserver_ip );
	p_req->port = penv->param.wserver_port ;
	
	p_req->is_working = (penv->accepted_progress?1:0) ;
	
	return 0;
}

static int ExecuteProgram( struct ServerEnv *penv , execute_program_request *p_req )
{
	pid_t		pid ;
	
	int		nret = 0 ;
	
	nret = pipe( penv->exit_pipe ) ;
	if( nret )
	{
		FatalLog( __FILE__ , __LINE__ , "pipe failed , errno[%d]" , errno );
		exit(1);
	}
	
	pid = fork() ;
	if( pid < 0 )
	{
		FatalLog( __FILE__ , __LINE__ , "fork failed , errno[%d]" , errno );
		close( penv->exit_pipe[0] );
		close( penv->exit_pipe[1] );
		return -1;
	}
	else if( pid == 0 )
	{
		char		*args[ 64 + 1 ] = { NULL } ;
		char		*pc = NULL ;
		int		i ;
		
		close( penv->exit_pipe[0] );
		
		InfoLog( __FILE__ , __LINE__ , "[%d]fork[%d] ok" , (int)getppid() , (int)getpid() );
		
		i = 0 ;
		pc = strtok( p_req->program_and_params , " \t" ) ;
		while( pc && i < sizeof(args)/sizeof(args[0])-1 )
		{
			args[i++] = pc ;
			pc = strtok( NULL , " \t" ) ;
		}
		if( i >= sizeof(args)/sizeof(args[0])-1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "param[%s] is to long" , p_req->program_and_params );
			exit(9);
		}
		args[i++] = NULL ;
		
		InfoLog( __FILE__ , __LINE__ , "execvp [%s] [%s] [%s] [%s] ..." , args[0]?args[0]:"" , args[1]?args[1]:"" , args[2]?args[2]:"" , args[3]?args[3]:"" );
		return execvp( args[0] , args );
	}
	else
	{
		close( penv->exit_pipe[1] );
		
		ResetSocketSession( & (penv->program_session) );
		penv->program_session.sock = penv->exit_pipe[0] ;
		penv->program_session.established_flag = 1 ;
		AddInputSockToEpoll( penv->epoll_socks , & (penv->program_session) );
		
		InfoLog( __FILE__ , __LINE__ , "[%d]fork[%d] ok" , (int)getpid() , (int)pid );
		
		penv->pid = pid ;
		
		penv->accepted_progress = 2 ;
	}
	
	return 0;
}

int app_WaitProgramExiting( struct ServerEnv *penv , struct SocketSession *psession )
{
	pid_t		pid ;
	int		status ;
	
	int		nret = 0 ;
	
	pid = waitpid( penv->pid , & status , WNOHANG ) ;
	if( pid == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "waitpid[%d] failed , errno[%d]" , (int)(penv->pid) , errno );
		return -1;
	}
	
	if( WTERMSIG(status) )
	{
		ErrorLog( __FILE__ , __LINE__ , "child[%d] terminated" , (int)pid );
	}
	else if( WIFSIGNALED(status) )
	{
		ErrorLog( __FILE__ , __LINE__ , "child[%d] signaled" , (int)pid );
	}
	else if( WIFSTOPPED(status) )
	{
		ErrorLog( __FILE__ , __LINE__ , "child[%d] stoped" , (int)pid );
	}
	else if( WIFEXITED(status) )
	{
		InfoLog( __FILE__ , __LINE__ , "child[%d] exit[%d]" , (int)pid , WEXITSTATUS(status) );
	}
	
	if( penv->accepted_progress == 2 )
	{
		penv->accepted_progress = 1 ;
		
		nret = proto_ExecuteProgramResponse( penv , & (penv->accepted_session) , status ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse failed[%d]" , nret );
			return nret;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse ok" );
		}
		
		ModifyOutputSockFromEpoll( penv->epoll_socks , & (penv->accepted_session) );
	}
	else
	{
		penv->accepted_progress = 0 ;
		DebugLog( __FILE__ , __LINE__ , "accepted sock not existed" );
	}
	
	DeleteSockFromEpoll( penv->epoll_socks , psession );
	CloseSocket( psession );
	
	penv->pid = 0 ;
	
	return 0;
}

int app_ExecuteProgramRequest( struct ServerEnv *penv , struct SocketSession *psession , execute_program_request *p_req )
{
	int		lock_fd ;
	
	char		program[ MAXLEN_FILENAME + 1 ] ;
	char		pathfilename[ MAXLEN_FILENAME + 1 ] ;
	char		program_md5_exp[ sizeof(((execute_program_request*)NULL)->program_md5_exp) ] ;
	
	int		nret = 0 ;
	
	lock_file( & lock_fd );
	
	memcpy( & (penv->epq) , p_req , sizeof(execute_program_request) );
	
	memset( program , 0x00 , sizeof(program) );
	sscanf( p_req->program_and_params , "%s" , program );
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	SNPRINTF( pathfilename , sizeof(pathfilename)-1 , "%s/bin/%s" , getenv("HOME") , program );
	memset( program_md5_exp , 0x00 , sizeof(program_md5_exp) );
	nret = FileMd5( pathfilename , program_md5_exp ) ;
	if( nret || STRCMP( program_md5_exp , != , p_req->program_md5_exp ) )
	{
		InfoLog( __FILE__ , __LINE__ , "FileMd5[%s][%d] or MD5[%s] and req[%s] not matched" , pathfilename , nret , program_md5_exp , p_req->program_md5_exp );
		proto_DeployProgramRequest( penv , psession , p_req );
		unlock_file( & lock_fd );
		return 0;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "program[%s] md5[%s] matched" , pathfilename , program_md5_exp );
	}
	
	unlock_file( & lock_fd );
	
	return ExecuteProgram( penv , p_req );
}

int app_DeployProgramResponse( struct ServerEnv *penv , struct SocketSession *psession )
{
	int		lock_fd ;
	
	char		program[ MAXLEN_FILENAME + 1 ] ;
	char		pathfilename[ MAXLEN_FILENAME + 1 ] ;
	FILE		*fp = NULL ;
	int		len ;
	
	lock_file( & lock_fd );
	
	memset( program , 0x00 , sizeof(program) );
	sscanf( penv->epq.program_and_params , "%s" , program );
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	SNPRINTF( pathfilename , sizeof(pathfilename)-1 , "%s/bin/%s" , getenv("HOME") , program );
	unlink( pathfilename );
	fp = fopen( pathfilename , "wb" ) ;
	if( fp == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "fopen[%s] failed , errno[%d]" , pathfilename , errno );
		proto_ExecuteProgramResponse( penv , psession , 124<<8 );
		unlock_file( & lock_fd );
		return 0;
	}
	
	len = (int)fwrite( psession->recv_buffer + LEN_COMMHEAD + LEN_MSGHEAD , sizeof(char) , psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD , fp );
	fchmod( fileno(fp) , 0700 );
	
	fclose( fp );
	
	if( len != psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD )
	{
		ErrorLog( __FILE__ , __LINE__ , "filesize[%d] != writed[%d]bytes , errno[%d]" , psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD , len , errno );
		proto_ExecuteProgramResponse( penv , psession , 125<<8 );
		unlock_file( & lock_fd );
		return 0;
	}
	
	unlock_file( & lock_fd );
	
	return ExecuteProgram( penv , & (penv->epq) );
}

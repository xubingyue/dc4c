#include "wserver.h"

int proto_DeployProgramRequest( struct ServerEnv *penv , struct SocketSession *psession , execute_program_request *p_req );
int proto_ExecuteProgramResponse( struct ServerEnv *penv , struct SocketSession *psession , int response_code , int status );

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
	
	penv->connect_flag = 2 ;
	
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
		args[i++] = p_req->program ;
		pc = strtok( p_req->params , " \t" ) ;
		while( pc && i < sizeof(args)/sizeof(args[0])-1 )
		{
			args[i++] = pc ;
			pc = strtok( NULL , " \t" ) ;
		}
		if( i >= sizeof(args)/sizeof(args[0])-1 )
		{
			ErrorLog( __FILE__ , __LINE__ , "param[%s] is to long" , p_req->params );
			exit(1);
		}
		args[i++] = NULL ;
		
		InfoLog( __FILE__ , __LINE__ , "execvp [%s] [%s] [%s] [%s] ..." , args[0]?args[0]:"" , args[1]?args[1]:"" , args[2]?args[2]:"" , args[3]?args[3]:"" );
		return execvp( args[0] , args );
	}
	else
	{
		close( penv->exit_pipe[1] );
		
		ResetSocketSession( & (penv->exit_session) );
		penv->exit_session.sock = penv->exit_pipe[0] ;
		AddInputSockToEpoll( penv->epoll_socks , & (penv->exit_session) );
		
		InfoLog( __FILE__ , __LINE__ , "[%d]fork[%d] ok" , (int)getpid() , (int)pid );
		penv->pid = pid ;
		penv->working_flag = 2 ;
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
	
	penv->working_flag = 1 ;
	
	nret = proto_ExecuteProgramResponse( penv , & (penv->accepted_session) , 0 , WEXITSTATUS(status) ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse failed[%d]" , nret );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse ok" );
	}
	
	ModifyOutputSockFromEpoll( penv->epoll_socks , & (penv->accepted_session) );
	
	penv->accepted_flag = 3 ;
	
	DeleteSockFromEpoll( penv->epoll_socks , psession );
	close( psession->sock );
	
	return 0;
}

int app_ExecuteProgramRequest( struct ServerEnv *penv , struct SocketSession *psession , execute_program_request *p_req )
{
	char		pathfilename[ MAXLEN_FILENAME + 1 ] ;
	char		md5_exp[ sizeof(((execute_program_request*)NULL)->md5_exp) ] ;
	
	int		nret = 0 ;
	
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	SNPRINTF( pathfilename , sizeof(pathfilename)-1 , "%s/bin/%s" , getenv("HOME") , p_req->program );
	memset( md5_exp , 0x00 , sizeof(md5_exp) );
	nret = FileMd5( pathfilename , md5_exp ) ;
	if( nret || STRCMP( md5_exp , != , p_req->md5_exp ) )
	{
		InfoLog( __FILE__ , __LINE__ , "FileMd5[%s][%d] or MD5[%s] and req[%s] not matched" , pathfilename , nret , md5_exp , p_req->md5_exp );
		memcpy( & (penv->epq) , p_req , sizeof(execute_program_request) );
		proto_DeployProgramRequest( penv , psession , p_req );
		return 0;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "program[%s]md5[%s] matched" , pathfilename , md5_exp );
	}
	
	return ExecuteProgram( penv , p_req );
}

int app_DeployProgramRequest( struct ServerEnv *penv , struct SocketSession *psession )
{
	char		pathfilename[ MAXLEN_FILENAME + 1 ] ;
	FILE		*fp = NULL ;
	int		len ;
	
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	SNPRINTF( pathfilename , sizeof(pathfilename)-1 , "%s/bin/%s" , getenv("HOME") , penv->epq.program );
	fp = fopen( pathfilename , "wb" ) ;
	if( fp == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "fopen[%s] failed , errno[%d]" , pathfilename , errno );
		proto_ExecuteProgramResponse( penv , psession , 0 , 7 );
		return 0;
	}
	
	len = (int)fwrite( psession->recv_buffer + LEN_COMMHEAD + LEN_MSGHEAD , sizeof(char) , psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD , fp );
	fchmod( fileno(fp) , 0700 );
	
	fclose( fp );
	
	if( len != psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD )
	{
		ErrorLog( __FILE__ , __LINE__ , "filesize[%d] != writed[%d]bytes , errno[%d]" , psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD , len , errno );
		proto_ExecuteProgramResponse( penv , psession , 0 , 8 );
		return 0;
	}
	
	return ExecuteProgram( penv , & (penv->epq) );
}

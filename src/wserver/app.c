/*
 * dc4c - Distributed computing framework
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "server.h"

int app_WorkerRegisterResponse( struct ServerEnv *penv , struct SocketSession *psession , worker_register_response *p_rsp )
{
	if( p_rsp->error )
	{
		ErrorLog( __FILE__ , __LINE__ , "error[%d]" , p_rsp->error );
		return -1;
	}
	
	psession->progress = CONNECT_SESSION_PROGRESS_REGISTED ;
	
	return 0;
}

int app_SendWorkerNotice( struct ServerEnv *penv )
{
	int		rserver_index ;
	
	int		nret = 0 ;
	
	for( rserver_index = 0 ; rserver_index < penv->rserver_count ; rserver_index++ )
	{
		if( penv->connect_session[rserver_index].progress == CONNECT_SESSION_PROGRESS_REGISTED )
		{
			nret = proto_WorkerNoticeRequest( penv , & (penv->connect_session[rserver_index]) ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "proto_WorkerNoticeRequest failed[%d]" , nret );
				return -1;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "proto_WorkerNoticeRequest ok" );
				ModifyOutputSockFromEpoll( penv->epoll_socks , & (penv->connect_session[rserver_index]) );
			}
		}
	}
	
	return 0;
}

static int app_ExecuteProgram( struct ServerEnv *penv , struct SocketSession *psession , execute_program_request *p_req )
{
	pid_t		pid ;
	
	int		nret = 0 ;
	
	nret = pipe( penv->executing_pipe ) ;
	if( nret )
	{
		FatalLog( __FILE__ , __LINE__ , "pipe failed , errno[%d]" , errno );
		nret = proto_ExecuteProgramResponse( penv , psession , DC4C_ERROR_CREATEPIPE , 0 ) ;
		return -1;
	}
	
	pid = fork() ;
	if( pid < 0 )
	{
		FatalLog( __FILE__ , __LINE__ , "fork failed , errno[%d]" , errno );
		close( penv->executing_pipe[0] );
		close( penv->executing_pipe[1] );
		nret = proto_ExecuteProgramResponse( penv , psession , DC4C_ERROR_FORK , 0 ) ;
		return -1;
	}
	else if( pid == 0 )
	{
		char		*args[ 64 + 1 ] = { NULL } ;
		char		*pc = NULL ;
		int		i ;
		char		envbuf[ 100 + 1 ] ;
		
		int		rserver_index ;
		
		close( penv->executing_pipe[0] );
		SetNonBlocking( penv->executing_pipe[1] );
		
		InfoLog( __FILE__ , __LINE__ , "[%d]fork[%d] ok" , (int)getppid() , (int)getpid() );
		
		setenv( "_" , p_req->program_and_params , 1 );
		
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
		
		memset( envbuf , 0x00 , sizeof(envbuf) );
		SNPRINTF( envbuf , sizeof(envbuf)-1 , "%d" , penv->wserver_index+1 );
		setenv( "WSERVER_INDEX" , envbuf , 1 );
		
		memset( envbuf , 0x00 , sizeof(envbuf) );
		SNPRINTF( envbuf , sizeof(envbuf)-1 , "%s:%d" , penv->param.wserver_ip , penv->param.wserver_port );
		setenv( "WSERVER_IP_PORT" , envbuf , 1 );
		
		memset( envbuf , 0x00 , sizeof(envbuf) );
		SNPRINTF( envbuf , sizeof(envbuf)-1 , "%d" , penv->executing_pipe[1] );
		setenv( "WSERVER_INFO_PIPE" , envbuf , 1 );
		
		if( IsSocketEstablished( & (penv->listen_session) ) )
		{
			CloseSocketSilently( & (penv->listen_session) );
			CleanSocketSession( & (penv->listen_session) );
		}
		
		for( rserver_index = 0 ; rserver_index < penv->rserver_count ; rserver_index++ )
		{
			CloseSocketSilently( & (penv->connect_session[rserver_index]) );
			CleanSocketSession( & (penv->connect_session[rserver_index]) );
		}
		
		close( penv->epoll_socks );
		
		if( p_req->bind_cpu_flag )
		{
			BindCpuProcessor( penv->wserver_index );
		}
		
		InfoLog( __FILE__ , __LINE__ , "execvp [%s] [%s] [%s] [%s] ..." , args[0]?args[0]:"" , args[1]?args[1]:"" , args[2]?args[2]:"" , args[3]?args[3]:"" );
		nret = execvp( args[0] , args ) ;
		FatalLog( __FILE__ , __LINE__ , "execvp failed[%d] , errno[%d]" , nret , errno );
		exit(DC4C_ERROR_EXEC);
	}
	else
	{
		psession->progress = ACCEPTED_SESSION_PROGRESS_DO_EXECUTE ;
		
		close( penv->executing_pipe[1] );
		SetNonBlocking( penv->executing_pipe[0] );
		
		penv->executing_session.sock = penv->executing_pipe[0] ;
		DebugLog( __FILE__ , __LINE__ , "executing_pipe[%d]" , penv->executing_pipe[0] );
		SetSocketEstablished( & (penv->executing_session) );
		psession->p1 = & (penv->executing_session) ;
		penv->executing_session.p1 = psession ;
		AddInputSockToEpoll( penv->epoll_socks , & (penv->executing_session) );
		
		InfoLog( __FILE__ , __LINE__ , "[%d]fork[%d] ok" , (int)getpid() , (int)pid );
		
		penv->pid = pid ;
		
		time( & (penv->begin_datetime_stamp) );
	}
	
	app_SendWorkerNotice( penv );
	
	return 0;
}

int app_WaitProgramExiting( struct ServerEnv *penv , struct SocketSession *p_execute_session )
{
	pid_t			pid ;
	int			error ;
	int			status ;
	
	struct SocketSession	*p_accepted_session = NULL ;
	
	int			nret = 0 ;
	
	status = 0 ;
	pid = waitpid( penv->pid , & status , 0 ); // WNOHANG ) ;
	if( pid == -1 )
	{
		ErrorLog( __FILE__ , __LINE__ , "waitpid[%d] failed , errno[%d]" , (int)pid , errno );
		return -1;
	}
	
	if( WTERMSIG(status) )
	{
		ErrorLog( __FILE__ , __LINE__ , "child[%d] terminated" , (int)pid );
		error = DC4C_ERROR_TERMSIG ;
	}
	else if( WIFSIGNALED(status) )
	{
		ErrorLog( __FILE__ , __LINE__ , "child[%d] signaled" , (int)pid );
		error = DC4C_ERROR_SIGNALED ;
	}
	else if( WIFSTOPPED(status) )
	{
		ErrorLog( __FILE__ , __LINE__ , "child[%d] stoped" , (int)pid );
		error = DC4C_ERROR_STOPPED ;
	}
	else if( WIFEXITED(status) )
	{
		InfoLog( __FILE__ , __LINE__ , "child[%d] exit[%d]" , (int)pid , WEXITSTATUS(status) );
		error = 0 ;
	}
	else
	{
		error = DC4C_ERROR_UNKNOW_QUIT ;
	}
	
	p_accepted_session = p_execute_session->p1 ;
	if( p_accepted_session && IsSocketEstablished( p_accepted_session ) )
	{
		p_accepted_session->progress = ACCEPTED_SESSION_PROGRESS_WAITFOR_REQUEST ;
		
		nret = proto_ExecuteProgramResponse( penv , p_accepted_session , error , status ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse failed[%d]" , nret );
			return nret;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "proto_ExecuteProgramResponse ok" );
		}
		
		ModifyOutputSockFromEpoll( penv->epoll_socks , p_accepted_session );
		
		p_execute_session->p1 = NULL ;
		p_accepted_session->p1 = NULL ;
	}
	else
	{
		ErrorLog( __FILE__ , __LINE__ , "accepted sock not existed" );
	}
	
	DeleteSockFromEpoll( penv->epoll_socks , p_execute_session );
	CloseSocketSilently( p_execute_session );
	
	app_SendWorkerNotice( penv );
	
	return 0;
}

int app_ExecuteProgramRequest( struct ServerEnv *penv , struct SocketSession *psession , execute_program_request *p_req )
{
	int		lock_fd ;
	
	char		program[ MAXLEN_FILENAME + 1 ] ;
	char		pathfilename[ MAXLEN_FILENAME + 1 ] ;
	char		program_md5_exp[ sizeof(((execute_program_request*)NULL)->program_md5_exp) ] ;
	
	int		nret = 0 ;
	
	if( psession->progress != ACCEPTED_SESSION_PROGRESS_WAITFOR_REQUEST )
	{
		ErrorLog( __FILE__ , __LINE__ , "progress[%d] invalid" , psession->progress );
		return RETURN_CLOSE;
	}
	
	lock_file( & lock_fd );
	
	memcpy( & (penv->epq) , p_req , sizeof(execute_program_request) );
	memset( & (penv->epp) , 0x00 , sizeof(execute_program_response) );
	
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
		psession->progress = ACCEPTED_SESSION_PROGRESS_DO_DEPLOY_BEFORE_EXECUTING ;
		unlock_file( & lock_fd );
		return 0;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "program[%s] md5[%s] matched" , pathfilename , program_md5_exp );
	}
	
	unlock_file( & lock_fd );
	
	return app_ExecuteProgram( penv , psession , p_req );
}

int app_DeployProgramResponse( struct ServerEnv *penv , struct SocketSession *psession )
{
	int		lock_fd ;
	
	char		program[ MAXLEN_FILENAME + 1 ] ;
	char		pathfilename[ MAXLEN_FILENAME + 1 ] ;
	FILE		*fp = NULL ;
	int		len ;
	char		program_md5_exp[ sizeof(((execute_program_request*)NULL)->program_md5_exp) ] ;
	
	int		nret = 0 ;
	
	if( psession->progress != ACCEPTED_SESSION_PROGRESS_DO_DEPLOY_BEFORE_EXECUTING )
	{
		ErrorLog( __FILE__ , __LINE__ , "progress[%d] invalid" , psession->progress );
		return RETURN_CLOSE;
	}
	
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
		proto_ExecuteProgramResponse( penv , psession , DC4C_ERROR_OPENFILE , 0 );
		unlock_file( & lock_fd );
		return 0;
	}
	
	len = (int)fwrite( psession->recv_buffer + LEN_COMMHEAD + LEN_MSGHEAD , sizeof(char) , psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD , fp );
	fchmod( fileno(fp) , 0700 );
	
	fclose( fp );
	
	if( len != psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD )
	{
		ErrorLog( __FILE__ , __LINE__ , "filesize[%d] != writed[%d]bytes , errno[%d]" , psession->total_recv_len - LEN_COMMHEAD - LEN_MSGHEAD , len , errno );
		proto_ExecuteProgramResponse( penv , psession , DC4C_ERROR_WRITEFILE , 0 );
		unlock_file( & lock_fd );
		return 0;
	}
	
	nret = FileMd5( pathfilename , program_md5_exp ) ;
	if( nret || STRCMP( program_md5_exp , != , penv->epq.program_md5_exp ) )
	{
		InfoLog( __FILE__ , __LINE__ , "FileMd5[%s][%d] or MD5[%s] and req[%s] not matched too" , pathfilename , nret , program_md5_exp , penv->epq.program_md5_exp );
		proto_ExecuteProgramResponse( penv , psession , DC4C_ERROR_MD5_NOT_MATCHED_TOO , 0 );
		unlock_file( & lock_fd );
		return 0;
	}
	
	unlock_file( & lock_fd );
	
	return app_ExecuteProgram( penv , psession , & (penv->epq) );
}

int app_HeartBeatRequest( struct ServerEnv *penv , long *p_now , long *p_epoll_timeout )
{
	struct SocketSession	*psession ;
	int			rserver_index ;
	long			try_timeout ;
	
	(*p_epoll_timeout) = SEND_HEARTBEAT_INTERVAL ;
	
	for( rserver_index = 0 , psession = & (penv->connect_session[0]) ; rserver_index < penv->rserver_count ; rserver_index++ , psession++ )
	{
		if( psession->heartbeat_lost_count > MAXCNT_HEARTBEAT_LOST )
		{
			ErrorLog( __FILE__ , __LINE__ , "heartbeat_lost_count[%d] > [%d]" , psession->heartbeat_lost_count , MAXCNT_HEARTBEAT_LOST );
			comm_CloseConnectedSocket( penv , psession );
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
	
	return 0;
}

int app_HeartBeatResponse( struct ServerEnv *penv , struct SocketSession *psession )
{
	DebugLog( __FILE__ , __LINE__ , "heartbeat_lost_count[%d]->[%d]" , psession->heartbeat_lost_count , 0 );
	psession->heartbeat_lost_count = 0 ;
	
	return 0;
}


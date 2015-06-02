#include "dc4c_api.h"

/* for testing
time ./dc4c_test_master 192.168.6.54:12001,192.168.6.54:12002 "dc4c_test_worker_sleep 3"
*/

int main( int argc , char *argv[] )
{
	struct Dc4cApiEnv	*penv = NULL ;
	
	char			begin_timebuf[ 256 + 1 ] ;
	char			end_timebuf[ 256 + 1 ] ;
	
	int			nret = 0 ;
	
	DC4CSetAppLogFile( "dc4c_test_master" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	if( argc == 1 + 2 )
	{
		nret = DC4CInitEnv( & penv , argv[1] ) ;
		if( nret )
		{
			printf( "DC4CInitEnv failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CInitEnv ok\n" );
		}
		
		DC4CSetTimeout( penv , 60 );
		DC4CSetOptions( penv , DC4C_OPTIONS_INTERRUPT_BY_APP );
		
		nret = DC4CDoTask( penv , argv[2] , DC4CGetTimeout(penv) ) ;
		if( nret )
		{
			printf( "DC4CDoTask failed[%d]\n" , nret );
		}
		else
		{
			printf( "DC4CDoTask ok\n" );
		}
		
		printf( "[%s][%ld]-[%s][%s][%d][%s][%s][%d]-[%d][%d][%s]\n"
			, DC4CGetTaskIp(penv) , DC4CGetTaskPort(penv)
			, DC4CGetTaskTid(penv) , DC4CGetTaskProgramAndParams(penv) , DC4CGetTaskTimeout(penv) , ConvertTimeString(DC4CGetTaskBeginTimestamp(penv),begin_timebuf,sizeof(begin_timebuf))+11 , ConvertTimeString(DC4CGetTaskEndTimestamp(penv),end_timebuf,sizeof(end_timebuf))+11 , DC4CGetTaskElapse(penv)
			, DC4CGetTaskError(penv) , WEXITSTATUS(DC4CGetTaskStatus(penv)) , DC4CGetTaskInfo(penv) );
		
		DC4CCleanEnv( & penv );
		printf( "DC4CCleanEnv ok\n" );
	}
	else
	{
		printf( "USAGE : dc4c_test_master rserver_ip:rserver_port program_and_params\n" );
		exit(7);
	}
	
	return 0;
}


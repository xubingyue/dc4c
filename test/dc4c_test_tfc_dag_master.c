#include "dc4c_api.h"
#include "dc4c_tfc_dag.h"

/* for testing
time ./dc4c_test_tfc_dag_master *.schedule"
*/

int main( int argc , char *argv[] )
{
	struct Dc4cDagSchedule	*p_sched = NULL ;
	
	int			nret = 0 ;
	
	DC4CSetAppLogFile( "dc4c_test_tfc_dag_master" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	if( argc == 1 + 1 )
	{
		nret = DC4CLoadDagScheduleFromFile( & p_sched , argv[1] ) ;
		if( nret )
		{
			printf( "DC4CLoadDagScheduleFromFile failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CLoadDagScheduleFromFile ok\n" );
		}
		
		nret = DC4CExecuteDagSchedule( p_sched ) ;
		if( nret )
		{
			printf( "DC4CExecuteDagSchedule failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CExecuteDagSchedule ok\n" );
		}
		
		DC4CLogDagSchedule( p_sched );
		
		DC4CUnloadDagSchedule( & p_sched );
		printf( "DC4CCleanEnv ok\n" );
	}
	else
	{
		printf( "USAGE : dc4c_test_tfc_dag_master *.schedule\n" );
		exit(7);
	}
	
	return 0;
}


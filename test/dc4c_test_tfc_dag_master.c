#include "dc4c_api.h"
#include "dc4c_tfc_dag.h"

/* for testing
time ./dc4c_test_tfc_dag_master rservers_ip_port *.dag_schedule"
*/

static int TestDagSchedule( char *dag_schedule_pathfilename , char *rservers_ip_port )
{
	struct Dc4cDagSchedule	*p_sched = NULL ;
	
	int			nret = 0 ;
	
	nret = DC4CLoadDagScheduleFromFile( & p_sched , dag_schedule_pathfilename , rservers_ip_port , 0 ) ;
	if( nret )
	{
		printf( "DC4CLoadDagScheduleFromFile failed[%d]\n" , nret );
		return -1;
	}
	else
	{
		printf( "DC4CLoadDagScheduleFromFile ok\n" );
	}
	
	DC4CLogDagSchedule( p_sched );
	
	nret = DC4CExecuteDagSchedule( p_sched ) ;
	if( nret )
	{
		printf( "DC4CExecuteDagSchedule failed[%d]\n" , nret );
		return -1;
	}
	else
	{
		printf( "DC4CExecuteDagSchedule ok\n" );
	}
	
	DC4CUnloadDagSchedule( & p_sched );
	printf( "DC4CUnloadDagSchedule ok\n" );
	
	return 0;
}

int main( int argc , char *argv[] )
{
	DC4CSetAppLogFile( "dc4c_test_tfc_dag_master" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	if( argc == 1 + 2 )
	{
		return TestDagSchedule( argv[2] , argv[1] );
	}
	else
	{
		printf( "USAGE : dc4c_test_tfc_dag_master rservers_ip_port .dag_schedule\n" );
		exit(7);
	}
}


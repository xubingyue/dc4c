#include "dc4c_api.h"
#include "dc4c_tfc_dag.h"
#include "IDL_dag_schedule.dsc.ESQL.eh"

/* for testing
time ./dc4c_test_tfc_dag_master rservers_ip_port *.dag_schedule"
*/

int main( int argc , char *argv[] )
{
	struct Dc4cDagSchedule	*p_sched = NULL ;
	
	int			nret = 0 ;
	
	DSCDBCONN( "0.0.0.0" , 18432 , "calvin" , "calvin" , "calvin" );
	if( SQLCODE )
	{
		printf( "DSCDBCONN failed , SQLCODE[%d][%s][%s]\n" , SQLCODE , SQLSTATE , SQLDESC );
		return 1;
	}
	else
	{
		printf( "DSCDBCONN ok\n" );
	}
	
	DC4CSetAppLogFile( "dc4c_test_tfc_dag_master" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	if( argc == 1 + 2 )
	{
		nret = DC4CLoadDagScheduleFromDatabase( & p_sched , argv[2] , DC4C_OPTIONS_INTERRUPT_BY_APP ) ;
		if( nret )
		{
			printf( "DC4CLoadDagScheduleFromDatabase failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CLoadDagScheduleFromDatabase ok\n" );
		}
		
		DC4CLogDagSchedule( p_sched );
		
		nret = DC4CExecuteDagSchedule( p_sched , argv[1] ) ;
		if( nret )
		{
			printf( "DC4CExecuteDagSchedule failed[%d]\n" , nret );
			return 1;
		}
		else
		{
			printf( "DC4CExecuteDagSchedule ok\n" );
		}
		
		DC4CUnloadDagSchedule( & p_sched );
		printf( "DC4CCleanEnv ok\n" );
	}
	else
	{
		printf( "USAGE : dc4c_test_tfc_dag_master rservers_ip_port .dag_schedule\n" );
		exit(7);
	}
	
	DSCDBDISCONN();
	printf( "DSCDBDISCONN ok\n" );
	
	return 0;
}


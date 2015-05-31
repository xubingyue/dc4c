#include "dc4c_api.h"

#include "gmp.h"

int pi_master( char *rservers_ip_port , unsigned long max_x , int tasks_count )
{
	struct Dc4cApiEnv	*penv = NULL ;
	struct Dc4cBatchTask	*tasks_array = NULL ;
	int			workers_count ;
	unsigned long		dd_x ;
	unsigned long		start_x , end_x ;
	
	int			i ;
	struct Dc4cBatchTask	*p_task = NULL ;
	char			*ip = NULL ;
	long			port ;
	char			*tid = NULL ;
	char			*program_and_params = NULL ;
	int			timeout ;
	int			elapse ;
	int			error ;
	int			status ;
	char			*info = NULL ;
	
	mpf_t			pi_incr ;
	mpf_t			pi ;
	
	char			output[ 1024 + 1 ] ;
	
	int			nret = 0 ;
	
	DC4CSetAppLogFile( "pi_master" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	InfoLog( __FILE__ , __LINE__ , "pi_master" );
	
	nret = DC4CInitEnv( & penv , rservers_ip_port ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CInitEnv failed[%d]" , nret );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CInitEnv ok" );
	}
	
	DC4CSetTimeout( penv , 600 );
	DC4CSetOptions( penv , DC4C_OPTIONS_BIND_CPU );
	
	if( tasks_count == -2 )
	{
		nret = DC4CQueryWorkers( penv ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CQueryWorkers failed[%d]" , nret );
			return -1;
		}
		
		tasks_count = DC4CGetUnusedWorkersCount( penv ) ;
		if( tasks_count <= 0 )
		{
			ErrorLog( __FILE__ , __LINE__ , "tasks_count[%d] invalid" , tasks_count );
			return -1;
		}
	}
	
	tasks_array = (struct Dc4cBatchTask *)malloc( sizeof(struct Dc4cBatchTask) * tasks_count ) ;
	if( tasks_array == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return -1;
	}
	memset( tasks_array , 0x00 , sizeof(struct Dc4cBatchTask) * tasks_count );
	
	dd_x = ( max_x - tasks_count ) / tasks_count ;
	start_x = 1 ;
	for( i = 0 , p_task = tasks_array ; i < tasks_count ; i++ , p_task++ )
	{
		end_x = start_x + dd_x ;
		snprintf( p_task->program_and_params , sizeof(p_task->program_and_params) , "dc4c_test_worker_pi %lu %lu" , start_x , end_x );
		p_task->timeout = DC4CGetTimeout(penv) ;
		start_x = end_x + 2 ;
	}
	
	workers_count = tasks_count ;
	nret = DC4CDoBatchTasks( penv , workers_count , tasks_array , tasks_count ) ;
	free( tasks_array );
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CDoBatchTasks failed[%d]" , nret );
		return -1;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CDoBatchTasks ok" );
	}
	
	mpf_init( pi_incr );
	mpf_init_set_d( pi , 0.00 );
	
	for( i = 0 ; i < tasks_count ; i++ )
	{
		DC4CGetBatchTasksIp( penv , i , & ip );
		DC4CGetBatchTasksPort( penv , i , & port );
		DC4CGetBatchTasksTid( penv , i , & tid );
		DC4CGetBatchTasksProgramAndParams( penv , i , & program_and_params );
		DC4CGetBatchTasksTimeout( penv , i , & timeout );
		DC4CGetBatchTasksElapse( penv , i , & elapse );
		DC4CGetBatchTasksError( penv , i , & error );
		DC4CGetBatchTasksStatus( penv , i , & status );
		DC4CGetBatchTasksInfo( penv , i , & info );
		InfoLog( __FILE__ , __LINE__ ,  "Task[%d]-[%s][%ld]-[%s][%s][%d][%d]-[%d][%d][%s]" , i , ip , port , tid , program_and_params , timeout , elapse , error , WEXITSTATUS(status) , info );
		
		mpf_set_str( pi_incr , info , 10 );
		mpf_add( pi , pi , pi_incr );
	}
	
	memset( output , 0x00 , sizeof(output) );
	gmp_snprintf( output , sizeof(output)-1 , "%.Ff" , pi );
	InfoLog( __FILE__ , __LINE__ , "pi_master() - max_x[%u] tasks_count[%d] - PI[%s]" , max_x , tasks_count , output );
	
	mpf_clear( pi_incr );
	mpf_clear( pi );
	
	DC4CCleanEnv( & penv );
	InfoLog( __FILE__ , __LINE__ , "DC4CCleanEnv ok" );
	
	return 0;
}

int pi_worker( unsigned long start_x , unsigned long end_x )
{
	mpf_t		_4 ;
	unsigned long	x ;
	int		flag ;
	mpf_t		pi_incr ;
	mpf_t		pi ;
	
	char		output[ 1024 + 1 ] ;
	
	DC4CSetAppLogFile( "pi_worker" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	InfoLog( __FILE__ , __LINE__ , "pi_worker" );
	
	if( start_x % 2 == 0 )
		start_x++;
	if( end_x % 2 == 0 )
		end_x++;
	
	if( start_x < 1 )
		start_x = 1 ;
	if( end_x < start_x )
		end_x = start_x ;
	
	mpf_init_set_d( _4 , 4.00 );
	mpf_init( pi_incr );
	mpf_init( pi );
	
	/*
	                                               0        1         2         3         4
	                                               1234567890123456789012345678901234567890
		           1   1   1   1   1
		PI = 4 * ( _ - _ + _ - _ + _ ... ) = 3.1415926535897932384626433832795
		           1   3   5   7   9
		                        4 1000000000 3.14159265158640477943 14.962s
		                          1000000000 3.14159265557624002834 56.315s
		                         4 100000000 3.14159263358945602263  1.460s
		                           100000000 3.14159267358843756024  5.888s
		                            10000000 3.14159285358961767296  0.621s
		                             1000000 3.14159465358577968703  0.091s
		                              100000 3.14161265318979787768  0.011s
		                               10000 3.14179261359579270235  0.003s
	*/
	mpf_set_d( pi , 0.00 );
	flag = ((start_x/2)%2)?'-':' ' ;
	for( x = start_x ; x <= end_x ; x += 2 )
	{
		mpf_div_ui( pi_incr , _4 , x );
		if( flag == '-' )
			mpf_neg( pi_incr , pi_incr );
		mpf_add( pi , pi , pi_incr );
		
		flag = '-' + ' ' - flag ;
	}
	
	memset( output , 0x00 , sizeof(output) );
	gmp_snprintf( output , sizeof(output)-1 , "%.Ff" , pi );
	InfoLog( __FILE__ , __LINE__ , "pi_worker() - start_x[%lu] end_x[%lu] - PI[%s]" , start_x , end_x , output );
	
	DC4CSetReplyInfo( output );
	
	mpf_clear( _4 );
	mpf_clear( pi_incr );
	mpf_clear( pi );
	
	return 0;
}

int main( int argc , char *argv[] )
{
	if( argc == 1 + 3 )
	{
		return pi_master( argv[1] , atol(argv[2]) , atoi(argv[3]) );
	}
	else if( argc == 1 + 2 )
	{
		return pi_worker( (unsigned long)atol(argv[1]) , (unsigned long)atol(argv[2]) );
	}
	else
	{
		printf( "USAGE : dc4c_test_worker_pi rservers_ip_port max_x worker_count\n" );
		printf( "                            start_x end_x\n" );
		exit(7);
	}
}


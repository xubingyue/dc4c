#include "dc4c_api.h"

int main( int argc , char *argv[] )
{
	DC4CSetAppLogFile( "dc4c_test_worker_hello" );
	SetLogLevel( LOGLEVEL_DEBUG );
	
	if( argc == 1 + 1 )
	{
		DC4CFormatReplyInfo( "hello %s" , argv[1] );
		return 0;
	}
	else
	{
		return 1;
	}
}


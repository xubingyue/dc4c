/* It had generated by DirectStruct v1.4.5 */
#include "IDL_worker_register_request.dsc.h"

#ifndef FUNCNAME_DSCLOG_worker_register_request
#define FUNCNAME_DSCLOG_worker_register_request	DSCLOG_worker_register_request
#endif

#ifndef PREFIX_DSCLOG_worker_register_request
#define PREFIX_DSCLOG_worker_register_request	printf( 
#endif

#ifndef NEWLINE_DSCLOG_worker_register_request
#define NEWLINE_DSCLOG_worker_register_request	"\n"
#endif

int FUNCNAME_DSCLOG_worker_register_request( worker_register_request *pst )
{
	int	index[10] = { 0 } ; index[0] = 0 ;
	PREFIX_DSCLOG_worker_register_request "worker_register_request.sysname[%s]" NEWLINE_DSCLOG_worker_register_request , pst->sysname );
	PREFIX_DSCLOG_worker_register_request "worker_register_request.release[%s]" NEWLINE_DSCLOG_worker_register_request , pst->release );
	PREFIX_DSCLOG_worker_register_request "worker_register_request.bits[%d]" NEWLINE_DSCLOG_worker_register_request , pst->bits );
	PREFIX_DSCLOG_worker_register_request "worker_register_request.ip[%s]" NEWLINE_DSCLOG_worker_register_request , pst->ip );
	PREFIX_DSCLOG_worker_register_request "worker_register_request.port[%d]" NEWLINE_DSCLOG_worker_register_request , pst->port );
	return 0;
}

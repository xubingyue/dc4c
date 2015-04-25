#ifndef _H_DC4C_API_
#define _H_DC4C_API_

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "IDL_query_workers_request.dsc.h"
#include "IDL_query_workers_response.dsc.h"
#include "IDL_execute_program_request.dsc.h"
#include "IDL_execute_program_response.dsc.h"
#include "IDL_deploy_program_request.dsc.h"

#include "dc4c_util.h"

#define DC4C_ERROR_FILE_NOT_EXIST	11

#define DC4C_ERROR_SOCKET		-11
#define DC4C_ERROR_CONNECT		-12

int DC4CDoTaskImmediately( char *rserver_ip , int rserver_port , char *program , char *params , execute_program_response *p_epp );

#endif

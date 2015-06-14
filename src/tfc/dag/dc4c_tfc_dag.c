/*
 * tfc_dag - DAG tasks engine for dc4c
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "dc4c_tfc_dag.h"

#define PREFIX_DSCLOG_dag_schedule_configfile	DebugLog( __FILE__ , __LINE__ , 
#define NEWLINE_DSCLOG_dag_schedule_configfile
#include "IDL_dag_schedule_configfile.dsc.LOG.c"

struct Dc4cDagSchedule
{
	char			schedule_name[ 64 + 1 ] ;
	char			schedule_desc[ 256 + 1 ] ;
	
	char			rservers_ip_port[ 256 + 1 ] ;
	int			options ;
	
	char			begin_datetime[ 19 + 1 ] ;
	char			end_datetime[ 19 + 1 ] ;
	int			progress ;
	
	SList			*all_nodes_list ; /* struct Dc4cDagBatch */
	SList			*config_batches_tree ; /* struct Dc4cDagBatch */
	SList			*executing_batches_tree ; /* struct Dc4cDagBatch */
	
	struct Dc4cApiEnv	**a_penv ;
	int			envs_size ;
} ;

struct Dc4cDagBatch
{
	SList			*predepend_batches_list ; /* struct Dc4cDagBatch */
	
	struct Dc4cDagSchedule	*p_sched ;
	char			batch_name[ 64 + 1 ] ;
	char			batch_desc[ 256 + 1 ] ;
	
	int			view_pos_x ;
	int			view_pos_y ;
	
	struct Dc4cBatchTask	*tasks_array ;
	int			tasks_count ;
	struct Dc4cApiEnv	*penv ;
	int			interrupt_by_app ;
	
	char			begin_datetime[ 19 + 1 ] ;
	char			end_datetime[ 19 + 1 ] ;
	int			progress ;
	
	SList			*postdepend_batches_list ; /* struct Dc4cDagBatch */
} ;

static struct Dc4cDagBatch *FindDagBatch( struct Dc4cDagSchedule *p_sched , char *batch_name )
{
	SListNode		*p_node = NULL ;
	struct Dc4cDagBatch	*p_batch = NULL ;
	
	if( STRCMP( batch_name , == , "" ) )
		return NULL;
	
	for( p_node = FindFirstListNode(p_sched->all_nodes_list) ; p_node ; p_node = FindNextListNode(p_node) )
	{
		p_batch = GetNodeMember(p_node) ;
		if( STRCMP( p_batch->batch_name , != , "" ) && STRCMP( p_batch->batch_name , == , batch_name ) )
			return p_batch;
	}
	
	return NULL;
}

int _LoadDagScheduleFromStruct( struct Dc4cDagSchedule *p_sched , dag_schedule_configfile *p_config , struct Dc4cDagBatch *p_parent_batch , struct Dc4cDagBatch *p_end_batch , char *from_batch_name )
{
	int			i , j , k ;
	struct Dc4cDagBatch	*p_batch = NULL ;
	
	int			nret = 0 ;
	
	for( i = 0 ; i < p_config->batches._batches_direction_count ; i++ )
	{
		if( STRCMP( p_config->batches.batches_direction[i].from_batch , == , from_batch_name ) )
		{
			if( STRCMP( p_config->batches.batches_direction[i].to_batch , == , "" ) )
			{
				p_batch = p_end_batch ;
			}
			else
			{
				p_batch = FindDagBatch( p_sched , p_config->batches.batches_direction[i].to_batch ) ;
				if( p_batch == NULL )
				{
					for( j = 0 ; j < p_config->batches._batches_info_count ; j++ )
					{
						if( STRCMP( p_config->batches.batches_info[j].batch_name , == , p_config->batches.batches_direction[i].to_batch ) )
							break;
					}
					if( j >= p_config->batches._batches_info_count )
					{
						ErrorLog( __FILE__ , __LINE__ , "batch_name[%s] not found in config" , p_config->batches.batches_direction[i].to_batch );
						return DC4C_ERROR_PARAMETER;
					}
					
					p_batch = DC4CAllocDagBatch( p_sched , p_config->batches.batches_info[j].batch_name , p_config->batches.batches_info[j].batch_desc , p_config->batches.batches_info[j].view_pos_x , p_config->batches.batches_info[j].view_pos_y ) ;
					if( p_batch == NULL )
					{
						ErrorLog( __FILE__ , __LINE__ , "DC4CAllocDagBatch failed , errno[%d]" , errno );
						return DC4C_ERROR_ALLOC;
					}
					else
					{
						DebugLog( __FILE__ , __LINE__ , "DC4CAllocDagBatch ok , batch_name[%s]" , p_batch->batch_name );
					}
					
					if( p_config->batches.batches_info[j].progress == DC4C_DAGBATCH_PROGRESS_FINISHED )
					{
						p_batch->tasks_count = 0 ;
						p_batch->tasks_array = NULL ;
					}
					else
					{
						p_batch->tasks_count = p_config->batches.batches_info[j]._tasks_count ;
						p_batch->tasks_array = (struct Dc4cBatchTask *)malloc( sizeof(struct Dc4cBatchTask) * p_batch->tasks_count ) ;
						if( p_batch->tasks_array == NULL )
						{
							ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
							return DC4C_ERROR_ALLOC;
						}
						memset( p_batch->tasks_array , 0x00 , sizeof(struct Dc4cBatchTask) * p_batch->tasks_count );
						
						p_batch->tasks_count = 0 ;
						for( k = 0 ; k < p_config->batches.batches_info[j]._tasks_count ; k++ )
						{
							if( p_config->batches.batches_info[j].tasks[k].progress != DC4C_DAGTASK_PROGRESS_FINISHED )
							{
								p_batch->tasks_array[k].order_index = p_config->batches.batches_info[j].tasks[k].order_index ;
								strncpy( p_batch->tasks_array[k].program_and_params , p_config->batches.batches_info[j].tasks[k].program_and_params , sizeof(p_batch->tasks_array[k].program_and_params)-1 );
								p_batch->tasks_array[k].timeout = p_config->batches.batches_info[j].tasks[k].timeout ;
								p_config->batches.batches_info[j].tasks[k].progress = DC4C_DAGTASK_PROGRESS_INIT ;
								
								p_batch->tasks_count++;
							}
						}
					}
					p_batch->interrupt_by_app = p_config->batches.batches_info[j].interrupt_by_app ;
					
					strncpy( p_batch->begin_datetime , p_config->batches.batches_info[j].begin_datetime , sizeof(p_batch->begin_datetime) );
					strncpy( p_batch->end_datetime , p_config->batches.batches_info[j].end_datetime , sizeof(p_batch->end_datetime) );
					p_batch->progress = DC4C_DAGBATCH_PROGRESS_INIT ;
				}
			}
			
			nret = DC4CLinkDagBatch( p_sched , p_parent_batch , p_batch ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "DC4CLinkDagBatch failed[%d]" , nret );
				return nret;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "DC4CLinkDagBatch ok" );
			}
			
			if( STRCMP( p_config->batches.batches_direction[i].to_batch , != , "" ) )
			{
				nret = _LoadDagScheduleFromStruct( p_sched , p_config , p_batch , p_end_batch , p_config->batches.batches_direction[i].to_batch ) ;
				if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "_LoadDagScheduleFromStruct failed[%d]" , nret );
					return nret;
				}
				else
				{
					DebugLog( __FILE__ , __LINE__ , "_LoadDagScheduleFromStruct ok" );
				}
			}
		}
	}
	
	return 0;
}

int DC4CLoadDagScheduleFromStruct( struct Dc4cDagSchedule **pp_sched , dag_schedule_configfile *p_config , char *rservers_ip_port , int options )
{
	struct Dc4cDagBatch	*p_begin_batch = NULL ;
	struct Dc4cDagBatch	*p_end_batch = NULL ;
	SList			*p_begin_node = NULL ;
	
	int			nret = 0 ;
	
	(*pp_sched) = (struct Dc4cDagSchedule *)malloc( sizeof(struct Dc4cDagSchedule) ) ;
	if( (*pp_sched) == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return DC4C_ERROR_ALLOC;
	}
	memset( (*pp_sched) , 0x00 , sizeof(struct Dc4cDagSchedule) );
	
	DSCLOG_dag_schedule_configfile( p_config );
	
	strncpy( (*pp_sched)->schedule_name , p_config->schedule.schedule_name , sizeof((*pp_sched)->schedule_name)-1 );
	strncpy( (*pp_sched)->schedule_desc , p_config->schedule.schedule_desc , sizeof((*pp_sched)->schedule_desc)-1 );
	
	strncpy( (*pp_sched)->rservers_ip_port , rservers_ip_port , sizeof((*pp_sched)->rservers_ip_port)-1 );
	(*pp_sched)->options = options ;
	
	strncpy( (*pp_sched)->begin_datetime , p_config->schedule.begin_datetime , sizeof((*pp_sched)->begin_datetime)-1 );
	strncpy( (*pp_sched)->end_datetime , p_config->schedule.end_datetime , sizeof((*pp_sched)->end_datetime)-1 );
	(*pp_sched)->progress = p_config->schedule.progress ;
	
	p_begin_batch = DC4CAllocDagBatch( (*pp_sched) , "" , NULL , 0 , 0 ) ;
	if( p_begin_batch == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CAllocDagBatch failed , errno[%d]" , errno );
		DC4CUnloadDagSchedule( pp_sched );
		return DC4C_ERROR_ALLOC;
	}
	
	p_begin_node = AddListNode( & ((*pp_sched)->config_batches_tree) , p_begin_batch , sizeof(struct Dc4cDagBatch) , NULL ) ;
	if( p_begin_node == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CAllocDagBatch failed , errno[%d]" , errno );
		DC4CUnloadDagSchedule( pp_sched );
		return DC4C_ERROR_ALLOC;
	}
	
	p_end_batch = DC4CAllocDagBatch( (*pp_sched) , "" , NULL , 0 , 0 ) ;
	if( p_end_batch == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CAllocDagBatch failed , errno[%d]" , errno );
		DC4CUnloadDagSchedule( pp_sched );
		return DC4C_ERROR_ALLOC;
	}
	
	nret = _LoadDagScheduleFromStruct( (*pp_sched) , p_config , p_begin_batch , p_end_batch , "" ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CAllocDagBatch failed , errno[%d]" , errno );
		DC4CUnloadDagSchedule( pp_sched );
		return nret;
	}
	
	return 0;
}

int DC4CLoadDagScheduleFromFile( struct Dc4cDagSchedule **pp_sched , char *pathfilename , char *rservers_ip_port , int options )
{
	dag_schedule_configfile *p_config = NULL ;
	
	FILE			*fp = NULL ;
	int			filesize ;
	int			readsize ;
	char			*filebuffer = NULL ;
	
	int			nret = 0 ;
	
	p_config = (dag_schedule_configfile *)malloc( sizeof(dag_schedule_configfile) ) ;
	if( p_config == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return DC4C_ERROR_ALLOC;
	}
	
	fp = fopen( pathfilename , "r" ) ;
	if( fp == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "fopen[%s] failed , errno[%d]" , pathfilename , errno );
		free( p_config );
		return DC4C_ERROR_FILE_NOT_FOUND;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "fopen[%s] ok" , pathfilename );
	}
	
	fseek( fp , 0 , SEEK_END );
	filesize = ftell( fp ) ;
	fseek( fp , 0 , SEEK_SET );
	
	filebuffer = (char*)malloc( filesize + 1 ) ;
	if( filebuffer == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		fclose( fp );
		free( p_config );
		return DC4C_ERROR_ALLOC;
	}
	memset( filebuffer , 0x00 , filesize + 1 );
	
	readsize = fread( filebuffer , 1 , filesize , fp ) ;
	fclose( fp );
	if( readsize < filesize )
	{
		ErrorLog( __FILE__ , __LINE__ , "fread failed , errno[%d]" , errno );
		free( filebuffer );
		free( p_config );
		return DC4C_ERROR_INTERNAL;
	}
	
	g_fasterjson_encoding = FASTERJSON_ENCODING_GB18030 ;
	
	DSCINIT_dag_schedule_configfile( p_config );
	
	nret = DSCDESERIALIZE_JSON_dag_schedule_configfile( NULL , filebuffer , & readsize , p_config ) ;
	free( filebuffer );
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_dag_schedule_configfile failed[%d]" , nret );
		free( p_config );
		return DC4C_ERROR_FILE_NOT_FOUND;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_dag_schedule_configfile ok" );
	}
	
	nret = DC4CLoadDagScheduleFromStruct( pp_sched , p_config , rservers_ip_port , options ) ;
	free( p_config );
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CLoadDagScheduleFromStruct failed[%d]" , nret );
		return nret;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CLoadDagScheduleFromStruct ok" );
	}
	
	return 0;
}

void DC4CUnloadDagSchedule( struct Dc4cDagSchedule **pp_sched )
{
	if( (*pp_sched) )
	{
		if( (*pp_sched)->config_batches_tree )
		{
			DestroyList( & ((*pp_sched)->config_batches_tree) , NULL );
			(*pp_sched)->config_batches_tree = NULL ;
		}
		
		if( (*pp_sched)->executing_batches_tree )
		{
			DestroyList( & ((*pp_sched)->executing_batches_tree) , NULL );
			(*pp_sched)->executing_batches_tree = NULL ;
		}
		
		if( (*pp_sched)->all_nodes_list )
		{
			DestroyList( & ((*pp_sched)->all_nodes_list) , NULL );
			(*pp_sched)->all_nodes_list = NULL ;
		}
		
		if( (*pp_sched)->a_penv )
		{
			free( (*pp_sched)->a_penv );
			(*pp_sched)->a_penv = NULL ;
			(*pp_sched)->envs_size = 0 ;
		}
		
		free( (*pp_sched) );
		(*pp_sched) = NULL ;
	}
	
	return;
}

static void _LogDagBatch( int depth , struct Dc4cDagBatch *p_batch )
{
	SListNode		*p_predepend_node = NULL ;
	struct Dc4cDagBatch	*p_predepend_batch = NULL ;
	SListNode		*p_postdepend_node = NULL ;
	struct Dc4cDagBatch	*p_postdepend_batch = NULL ;
	
	InfoLog( __FILE__ , __LINE__ , "[%d] - p_batch[%p][%s]" , depth , p_batch , p_batch->batch_name );
	
	for( p_predepend_node = FindFirstListNode(p_batch->predepend_batches_list) ; p_predepend_node ; p_predepend_node = FindNextListNode(p_predepend_node) )
	{
		p_predepend_batch = (struct Dc4cDagBatch *)GetNodeMember(p_predepend_node) ;
		InfoLog( __FILE__ , __LINE__ , "p_predepend_node[%p] FreeNodeMember[%p] p_batch[%p][%s] tasks_count[%d]" , p_predepend_node , p_predepend_node->FreeNodeMember , p_predepend_batch , p_predepend_batch->batch_name , p_batch->tasks_count );
	}
	for( p_postdepend_node = FindFirstListNode(p_batch->postdepend_batches_list) ; p_postdepend_node ; p_postdepend_node = FindNextListNode(p_postdepend_node) )
	{
		p_postdepend_batch = (struct Dc4cDagBatch *)GetNodeMember(p_postdepend_node) ;
		InfoLog( __FILE__ , __LINE__ , "p_postdepend_node[%p] FreeNodeMember[%p] p_batch[%p][%s] tasks_count[%d]" , p_postdepend_node , p_postdepend_node->FreeNodeMember , p_postdepend_batch , p_postdepend_batch->batch_name , p_batch->tasks_count );
	}
	for( p_postdepend_node = FindFirstListNode(p_batch->postdepend_batches_list) ; p_postdepend_node ; p_postdepend_node = FindNextListNode(p_postdepend_node) )
	{
		p_postdepend_batch = (struct Dc4cDagBatch *)GetNodeMember(p_postdepend_node) ;
		_LogDagBatch( depth + 1 , p_postdepend_batch );
	}
	
	return;
}

void DC4CLogDagSchedule( struct Dc4cDagSchedule *p_sched )
{
	SListNode		*p_node = NULL ;
	struct Dc4cDagBatch	*p_batch = NULL ;
	
	if( p_sched == NULL )
		return;
	
	InfoLog( __FILE__ , __LINE__ , "--- DC4CLogDagSchedule - schedule------------------" );
	InfoLog( __FILE__ , __LINE__ , "p_sched[%p] schedule_name[%s] progress[%d]" , p_sched , p_sched->schedule_name , p_sched->progress );
	
	InfoLog( __FILE__ , __LINE__ , "--- DC4CLogDagSchedule - config_batches_tree ------------------" );
	for( p_node = FindFirstListNode(p_sched->config_batches_tree) ; p_node ; p_node = FindNextListNode(p_node) )
	{
		p_batch = (struct Dc4cDagBatch *)GetNodeMember(p_node) ;
		_LogDagBatch( 1 , p_batch );
	}
	
	InfoLog( __FILE__ , __LINE__ , "--- DC4CLogDagSchedule - all_nodes_list ------------------" );
	for( p_node = FindFirstListNode(p_sched->all_nodes_list) ; p_node ; p_node = FindNextListNode(p_node) )
	{
		p_batch = (struct Dc4cDagBatch *)GetNodeMember(p_node) ;
		InfoLog( __FILE__ , __LINE__ , "p_node[%p] FreeNodeMember[%p] p_batch[%p][%s] progress[%d] tasks_count[%d]" , p_node , p_node->FreeNodeMember , p_batch , p_batch->batch_name , p_batch->progress , p_batch->tasks_count );
	}
	
	return;
}

int DC4CExecuteDagSchedule( struct Dc4cDagSchedule *p_sched )
{
	struct Dc4cDagBatch	*p_batch = NULL ;
	
	int			nret = 0 ;
	
	nret = DC4CBeginDagSchedule( p_sched ) ;
	if( nret )
		return nret;
	
	while(1)
	{
		nret = DC4CPerformDagSchedule( p_sched , & p_batch , NULL , NULL ) ;
		if( nret == DC4C_INFO_TASK_FINISHED )
		{
			DebugLog( __FILE__ , __LINE__ , "DC4CPerformDagSchedule return DC4C_INFO_TASK_FINISHED" );
		}
		else if( nret == DC4C_INFO_BATCH_TASKS_FINISHED )
		{
			DebugLog( __FILE__ , __LINE__ , "DC4CPerformDagSchedule return DC4C_INFO_BATCH_TASKS_FINISHED" );
		}
		else if( nret == DC4C_INFO_ALL_ENVS_FINISHED )
		{
			InfoLog( __FILE__ , __LINE__ , "DC4CPerformDagSchedule return DC4C_INFO_ALL_ENVS_FINISHED" );
			return 0;
		}
		else if( nret == DC4C_ERROR_TIMEOUT )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CPerformDagSchedule return DC4C_ERROR_TIMEOUT" );
			return nret;
		}
		else if( nret == DC4C_ERROR_APP )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CPerformDagSchedule return DC4C_ERROR_APP" );
			return nret;
		}
		else if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CPerformDagSchedule failed[%d]" , nret );
			return nret;
		}
	}
	
	return 0;
}

extern void DC4CSetPrepareTasksCount( struct Dc4cApiEnv *penv , int prepare_count );
extern void DC4CSetRunningTasksCount( struct Dc4cApiEnv *penv , int running_count );
extern void DC4CSetFinishedTasksCount( struct Dc4cApiEnv *penv , int finished_count );

static int MovedownExecutingTree( struct Dc4cDagSchedule *p_sched , SListNode *p_executing_batches_node , struct Dc4cDagBatch *p_branch_batch )
{
	SListNode		*p_postdepend_branch_node = NULL ;
	struct Dc4cDagBatch	*p_postdepend_branch_batch = NULL ;
	SListNode		*p_postdepend_predepend_branch_node = NULL ;
	struct Dc4cDagBatch	*p_postdepend_predepend_branch_batch = NULL ;
	SListNode		*p_insert_execute_batches_node = NULL ;
	
	int			nret = 0 ;
	
	if( p_branch_batch->progress != DC4C_DAGBATCH_PROGRESS_FINISHED )
		return 0;
	
	for( p_postdepend_branch_node = FindFirstListNode(p_branch_batch->postdepend_batches_list) ; p_postdepend_branch_node ; p_postdepend_branch_node = FindNextListNode(p_postdepend_branch_node) )
	{
		p_postdepend_branch_batch = GetNodeMember(p_postdepend_branch_node) ;
		if( STRCMP( p_postdepend_branch_batch->batch_name , == , "" ) )
			break;
		
		for( p_postdepend_predepend_branch_node = FindFirstListNode(p_postdepend_branch_batch->predepend_batches_list) ; p_postdepend_predepend_branch_node ; p_postdepend_predepend_branch_node = FindNextListNode(p_postdepend_predepend_branch_node) )
		{
			p_postdepend_predepend_branch_batch = GetNodeMember(p_postdepend_predepend_branch_node) ;
			if( p_postdepend_predepend_branch_batch->progress != DC4C_DAGBATCH_PROGRESS_FINISHED )
				break;
		}
		if( p_postdepend_predepend_branch_node == NULL )
		{
			p_insert_execute_batches_node = InsertListNodeAfter( & (p_sched->executing_batches_tree) , & p_executing_batches_node , p_postdepend_branch_node , sizeof(struct Dc4cDagBatch) , NULL ) ;
			if( p_insert_execute_batches_node == NULL )
			{
				ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
				return DC4C_ERROR_ALLOC;
			}
			
			nret = DC4CInitEnv( & (p_postdepend_branch_batch->penv) , p_sched->rservers_ip_port ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "DC4CInitEnv failed[%d]" , nret );
				return nret;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "DC4CInitEnv ok" );
			}
			
InfoLog( __FILE__ , __LINE__ , "calvintest - p_postdepend_branch_batch->batch_name[%s] p_postdepend_branch_batch->interrupt_by_app[%d]" , p_postdepend_branch_batch->batch_name , p_postdepend_branch_batch->interrupt_by_app );
			if( p_postdepend_branch_batch->interrupt_by_app )
				DC4CSetOptions( p_postdepend_branch_batch->penv , DC4C_OPTIONS_INTERRUPT_BY_APP );
			else
				DC4CSetOptions( p_postdepend_branch_batch->penv , p_sched->options );
			
			InfoLog( __FILE__ , __LINE__ , "DAG ACT batch - batch_name[%s] tasks_count[%d]" , p_postdepend_branch_batch->batch_name , p_postdepend_branch_batch->tasks_count );
			nret = DC4CBeginBatchTasks( p_postdepend_branch_batch->penv , p_postdepend_branch_batch->tasks_count , p_postdepend_branch_batch->tasks_array , p_postdepend_branch_batch->tasks_count ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "DC4CBeginBatchTasks failed[%d] , errno[%d]" , nret , errno );
				return nret;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "DC4CBeginBatchTasks ok" );
			}
			
			p_postdepend_branch_batch->progress = DC4C_DAGBATCH_PROGRESS_EXECUTING ;
		}
	}
	
	if(	p_branch_batch->progress == DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR
		&&
		(
			DC4CGetDagBatchApiEnvPtr(p_branch_batch) == NULL
			||
			(
				DC4CGetDagBatchApiEnvPtr(p_branch_batch)
				&&
				TestAttribute( DC4CGetOptions(DC4CGetDagBatchApiEnvPtr(p_branch_batch)) , DC4C_OPTIONS_INTERRUPT_BY_APP ) )
			)
		)
	{
		;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "DeleteListNode batch_name[%s]" , p_branch_batch->batch_name );
		DeleteListNode( & (p_sched->executing_batches_tree) , & p_executing_batches_node , NULL );
	}
	
	return 0;
}

int DC4CBeginDagSchedule( struct Dc4cDagSchedule *p_sched )
{
	SListNode		*p_executing_batches_node = NULL ;
	SListNode		*p_branch_node = NULL ;
	struct Dc4cDagBatch	*p_branch_batch = NULL ;
	
	int			nret = 0 ;
	
	if( p_sched == NULL )
		return DC4C_ERROR_PARAMETER;
	
	InfoLog( __FILE__ , __LINE__ , "p_sched[%p] schedule_name[%s] config_batches_tree[%p]" , p_sched , p_sched->schedule_name , p_sched->config_batches_tree );
	
	if( p_sched->config_batches_tree == NULL )
		return 0;
	
	p_sched->progress = DC4C_DAGSCHEDULE_PROGRESS_EXECUTING ;
	
	if( p_sched->a_penv == NULL )
	{
		p_sched->envs_size = 10 ;
		p_sched->a_penv = (struct Dc4cApiEnv**)malloc( sizeof(struct Dc4cApiEnv*) * p_sched->envs_size ) ;
		if( p_sched->a_penv == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			p_sched->progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
			return DC4C_ERROR_ALLOC;
		}
		memset( p_sched->a_penv , 0x00 , sizeof(sizeof(struct Dc4cApiEnv*) * p_sched->envs_size) );
	}
	
	p_branch_batch = GetNodeMember(p_sched->config_batches_tree) ;
	for( p_branch_node = FindFirstListNode(p_branch_batch->postdepend_batches_list) ; p_branch_node ; p_branch_node = FindNextListNode(p_branch_node) )
	{
		p_branch_batch = GetNodeMember(p_branch_node) ;
		if( STRCMP( p_branch_batch->batch_name , == , "" ) )
			continue;
		
		nret = DC4CInitEnv( & (p_branch_batch->penv) , p_sched->rservers_ip_port ) ;
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CInitEnv failed[%d]" , nret );
			p_sched->progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
			return nret;
		}
		
		if( p_branch_batch->interrupt_by_app )
			DC4CSetOptions( p_branch_batch->penv , DC4C_OPTIONS_INTERRUPT_BY_APP );
		else
			DC4CSetOptions( p_branch_batch->penv , p_sched->options );
		
		nret = DC4CBeginBatchTasks( p_branch_batch->penv , p_branch_batch->tasks_count , p_branch_batch->tasks_array , p_branch_batch->tasks_count ) ;
		InfoLog( __FILE__ , __LINE__ , "DAG ACT batch - batch_name[%s] tasks_count[%d]" , p_branch_batch->batch_name , p_branch_batch->tasks_count );
		if( nret )
		{
			ErrorLog( __FILE__ , __LINE__ , "DC4CBeginBatchTasks failed[%d] , errno[%d]" , nret , errno );
			p_sched->progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
			return nret;
		}
		
		p_executing_batches_node = AddListNode( & (p_sched->executing_batches_tree) , p_branch_node , sizeof(struct Dc4cDagBatch) , NULL ) ;
		if( p_executing_batches_node == NULL )
		{
			ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
			p_sched->progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
			return DC4C_ERROR_ALLOC;
		}
	}
	
	return 0;
}

int DC4CPerformDagSchedule( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch **pp_batch , struct Dc4cApiEnv **ppenv , int *p_task_index )
{
	int			envs_count ;
	struct Dc4cApiEnv	*penv = NULL ;
	int			task_index ;
	int			perform_return ;
	
	SListNode		*p_executing_batches_node = NULL ;
	SListNode		*p_branch_node = NULL ;
	struct Dc4cDagBatch	*p_branch_batch = NULL ;
	
	int			nret = 0 ;
	
	if( p_sched == NULL )
		return DC4C_ERROR_PARAMETER;
	
	if( p_sched->executing_batches_tree == NULL )
	{
		p_sched->progress = DC4C_DAGSCHEDULE_PROGRESS_FINISHED ;
		return DC4C_INFO_ALL_ENVS_FINISHED;
	}
	
	envs_count = 0 ;
	for( p_executing_batches_node = FindFirstListNode(p_sched->executing_batches_tree) ; p_executing_batches_node ; )
	{
		p_branch_node = GetNodeMember(p_executing_batches_node) ;
		p_branch_batch = GetNodeMember(p_branch_node) ;
		
		InfoLog( __FILE__ , __LINE__ , "DAG SET batch - batch_name[%s] tasks_count[%d]" , p_branch_batch->batch_name , p_branch_batch->tasks_count );
		
		p_branch_batch->progress = DC4C_DAGBATCH_PROGRESS_EXECUTING ;
		if( p_branch_batch->begin_datetime[0] == '\0' )
		{
			GetTimeStringNow( p_branch_batch->begin_datetime , sizeof(p_branch_batch->begin_datetime) );
		}
		if( envs_count >= p_sched->envs_size )
		{
			struct Dc4cApiEnv	**tmp = NULL ;
			tmp = (struct Dc4cApiEnv**)realloc( p_sched->a_penv , sizeof(struct Dc4cApiEnv*) * p_sched->envs_size * 2 ) ;
			if( tmp == NULL )
			{
				ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
				p_sched->progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
				return DC4C_ERROR_ALLOC;
			}
			else
			{
				InfoLog( __FILE__ , __LINE__ , "alloc ok , envs_size[%d]->[%d]" , p_sched->envs_size , p_sched->envs_size * 2 );
			}
			p_sched->a_penv = tmp ;
			p_sched->envs_size *= 2 ;
		}
		p_sched->a_penv[envs_count++] = p_branch_batch->penv ;
		
		p_executing_batches_node = FindNextListNode(p_executing_batches_node) ;
	}
	
	penv = NULL ;
	perform_return = DC4CPerformMultiBatchTasks( p_sched->a_penv , envs_count , & penv , & task_index ) ;
	if( perform_return == DC4C_INFO_TASK_FINISHED )
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks return DC4C_INFO_TASK_FINISHED" );
	}
	else if( perform_return == DC4C_INFO_BATCH_TASKS_FINISHED )
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks return DC4C_INFO_BATCH_TASKS_FINISHED" );
		p_branch_batch->progress = DC4C_DAGBATCH_PROGRESS_FINISHED ;
	}
	else if( perform_return == DC4C_INFO_ALL_ENVS_FINISHED )
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks return DC4C_INFO_ALL_ENVS_FINISHED" );
		p_sched->progress = DC4C_DAGBATCH_PROGRESS_FINISHED ;
		return DC4C_INFO_ALL_ENVS_FINISHED;
	}
	else if( perform_return == DC4C_ERROR_TIMEOUT )
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks return DC4C_ERROR_TIMEOUT" );
		p_branch_batch->progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
	}
	else if( perform_return == DC4C_ERROR_APP )
	{
		InfoLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks return DC4C_ERROR_APP" );
		p_branch_batch->progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
	}
	else
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CPerformMultiBatchTasks failed[%d]" , perform_return );
		p_branch_batch->progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
		p_sched->progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
	}
	
	for( p_executing_batches_node = FindFirstListNode(p_sched->executing_batches_tree) ; p_executing_batches_node ; p_executing_batches_node = FindNextListNode(p_executing_batches_node) )
	{
		p_branch_node = GetNodeMember(p_executing_batches_node) ;
		p_branch_batch = GetNodeMember(p_branch_node) ;
		if( p_branch_batch->penv == penv )
			break;
	}
	if( p_executing_batches_node == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "Internal Errno" );
		p_sched->progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
		return DC4C_ERROR_INTERNAL;
	}
	
	if( perform_return == DC4C_INFO_TASK_FINISHED )
	{
		InfoLog( __FILE__ , __LINE__ , "DAG FIN task - batch_name[%s] tasks_index[%d] error[%d] status[%d]" , p_branch_batch->batch_name , task_index , DC4CGetBatchTasksError(p_branch_batch->penv,task_index) , DC4CGetBatchTasksStatus(p_branch_batch->penv,task_index) );
	}
	else if( perform_return == DC4C_INFO_BATCH_TASKS_FINISHED )
	{
		InfoLog( __FILE__ , __LINE__ , "DAG FIN batch - batch_name[%s] tasks_count[%d]" , p_branch_batch->batch_name , p_branch_batch->tasks_count );
		p_branch_batch->progress = DC4C_DAGBATCH_PROGRESS_FINISHED ;
	}
	else
	{
		InfoLog( __FILE__ , __LINE__ , "DAG ERR batch - batch_name[%s]" , p_branch_batch->batch_name );
	}
	
	GetTimeStringNow( p_branch_batch->end_datetime , sizeof(p_branch_batch->end_datetime) );
	
	if( pp_batch )
		(*pp_batch) = p_branch_batch ;
	if( ppenv )
		(*ppenv) = penv ;
	if( p_task_index )
		(*p_task_index) = task_index ;
	
	nret = MovedownExecutingTree( p_sched , p_executing_batches_node , p_branch_batch ) ;
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "MovedownExecutingTree failed[%d]" , nret );
		p_sched->progress = DC4C_DAGBATCH_PROGRESS_FINISHED_WITH_ERROR ;
		return nret;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "MovedownExecutingTree ok" );
	}
	
	return perform_return;
}

char *DC4CGetDagScheduleName( struct Dc4cDagSchedule *p_sched )
{
	return p_sched->schedule_name;
}

int DC4CGetDagScheduleProgress( struct Dc4cDagSchedule *p_sched )
{
	return p_sched->progress;
}

char *DC4CGetDagBatchName( struct Dc4cDagBatch *p_batch )
{
	return p_batch->batch_name;
}

void DC4CSetDagBatchTasks( struct Dc4cDagBatch *p_batch , struct Dc4cBatchTask *tasks_array , int tasks_count )
{
	if( p_batch->tasks_array )
	{
		free( p_batch->tasks_array );
	}
	
	p_batch->tasks_array = tasks_array ;
	p_batch->tasks_count = tasks_count ;
	
	return;
}

struct Dc4cApiEnv *DC4CGetDagBatchApiEnvPtr( struct Dc4cDagBatch *p_batch )
{
	return p_batch->penv;
}

struct Dc4cApiEnv **DC4CGetDagBatchApiEnvPPtr( struct Dc4cDagBatch *p_batch )
{
	return & (p_batch->penv);
}

char *DC4CGetDagBatchBeginDatetime( struct Dc4cDagBatch *p_batch )
{
	return p_batch->begin_datetime;
}

char *DC4CGetDagBatchEndDatetime( struct Dc4cDagBatch *p_batch )
{
	return p_batch->end_datetime;
}

int DC4CGetDagBatchProgress( struct Dc4cDagBatch *p_batch )
{
	return p_batch->progress;
}

struct Dc4cDagBatch *DC4CAllocDagBatch( struct Dc4cDagSchedule *p_sched , char *batch_name , char *batch_desc , int view_pos_x , int view_pos_y )
{
	struct Dc4cDagBatch	*p_batch = NULL ;
	SListNode		*p_node = NULL ;
	
	p_batch = (struct Dc4cDagBatch *)malloc( sizeof(struct Dc4cDagBatch) ) ;
	if( p_batch == NULL )
		return NULL;
	memset( p_batch , 0x00 , sizeof(struct Dc4cDagBatch) );
	
	p_batch->p_sched = p_sched ;
	if( batch_name )
		strncpy( p_batch->batch_name , batch_name , sizeof(p_batch->batch_name)-1 );
	if( batch_desc )
		strncpy( p_batch->batch_desc , batch_desc , sizeof(p_batch->batch_desc)-1 );
	
	p_batch->view_pos_x = view_pos_x ;
	p_batch->view_pos_y = view_pos_y ;
	
	p_batch->progress = DC4C_DAGBATCH_PROGRESS_INIT ;
	
	p_node = AddListNode( & (p_sched->all_nodes_list) , p_batch , sizeof(struct Dc4cDagBatch) , & DC4CFreeDagBatch ) ;
	if( p_node == NULL )
	{
		DC4CFreeDagBatch( p_batch );
		return NULL;
	}
	
	return p_batch;
}

BOOL DC4CFreeDagBatch( void *pv )
{
	struct Dc4cDagBatch	*p_batch = (struct Dc4cDagBatch *) pv ;
	
	if( p_batch )
	{
		DestroyList( & (p_batch->postdepend_batches_list) , NULL );
		
		DestroyList( & (p_batch->predepend_batches_list) , NULL );
		
		if( p_batch->tasks_array )
		{
			free( p_batch->tasks_array );
			p_batch->tasks_array = NULL ;
		}
		
		if( p_batch->penv )
		{
			DC4CCleanEnv( & (p_batch->penv) );
			p_batch->penv = NULL ;
		}
		
		free( p_batch );
		p_batch = NULL ;
	}
	
	return TRUE;
}

int DC4CLinkDagBatch( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_parent_batch , struct Dc4cDagBatch *p_batch )
{
	SListNode	*p_node = NULL ;
	
	p_node = AddListNode( & (p_parent_batch->postdepend_batches_list) , p_batch , sizeof(struct Dc4cDagBatch) , NULL ) ;
	if( p_node == NULL )
		return -1;
	
	p_node = AddListNode( & (p_batch->predepend_batches_list) , p_parent_batch , sizeof(struct Dc4cDagBatch) , NULL ) ;
	if( p_node == NULL )
		return -1;
	
	return 0;
}


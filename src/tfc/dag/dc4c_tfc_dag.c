/*
 * tfc_dag for dc4c - Tasks flow controler for dc4c
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "dc4c_tfc_dag.h"

#include "IDL_dag_schedule_config.dsc.h"

#include "IDL_dag_schedule_config.dsc.LOG.c"

struct Dc4cDagSchedule
{
	dag_schedule_config	schedule_config ;
	
	char			schedule_name[ 64 + 1 ] ;
	char			schedule_desc[ 256 + 1 ] ;
	
	SList			*root_batches_list ; /* struct Dc4cDagBatch */
	SList			*executing_batches_list ; /* struct Dc4cDagBatch */
	
	int			progress ;
	int			result ;
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
	
	char			begin_datetime[ 19 + 1 ] ;
	int			begin_datetime_stamp ;
	char			end_datetime[ 19 + 1 ] ;
	int			end_datetime_stamp ;
	int			progress ;
	int			result ;
	
	SList			*postdepend_batches_list ; /* struct Dc4cDagBatch */
} ;

static int _ParseBatchesDirection( struct Dc4cDagSchedule *p_sched , struct Dc4cDagBatch *p_parent_batch , struct Dc4cDagBatch *p_end_batch , char *from_batch_name )
{
	int			i , j ;
	struct Dc4cDagBatch	*p_batch = NULL ;
	
	int			nret = 0 ;
	
	for( i = 0 ; i < p_sched->schedule_config.batches._batches_direction_count ; i++ )
	{
		if( STRCMP( p_sched->schedule_config.batches.batches_direction[i].from , == , from_batch_name ) )
		{
			if( STRCMP( p_sched->schedule_config.batches.batches_direction[i].to , == , "" ) )
			{
				p_batch = p_end_batch ;
			}
			else
			{
				for( j = 0 ; j < p_sched->schedule_config.batches._batches_info_count ; j++ )
				{
					if( STRCMP( p_sched->schedule_config.batches.batches_info[j].name , == , p_sched->schedule_config.batches.batches_direction[i].to ) )
						break;
				}
				if( j >= p_sched->schedule_config.batches._batches_info_count )
				{
					ErrorLog( __FILE__ , __LINE__ , "batch_name[%s] not found in config" , p_sched->schedule_config.batches.batches_direction[i].to );
					return DC4C_ERROR_PARAMETER;
				}
				
				p_batch = DC4CAllocDagBatch( p_sched , p_sched->schedule_config.batches.batches_info[j].name , p_sched->schedule_config.batches.batches_info[j].desc , p_sched->schedule_config.batches.batches_info[j].view_pos_x , p_sched->schedule_config.batches.batches_info[j].view_pos_y ) ;
				if( p_batch == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "DC4CAllocDagBatch failed , errno[%d]" , errno );
					return DC4C_ERROR_ALLOC;
				}
				
				p_batch->tasks_count = p_sched->schedule_config.batches.batches_info[j]._tasks_count ;
				p_batch->tasks_array = (struct Dc4cBatchTask *)malloc( sizeof(struct Dc4cBatchTask) * p_batch->tasks_count ) ;
				if( p_batch->tasks_array == NULL )
				{
					ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
					return DC4C_ERROR_ALLOC;
				}
				memset( p_batch->tasks_array , 0x00 , sizeof(struct Dc4cBatchTask) * p_batch->tasks_count );
			}
			
			nret = DC4CLinkDagBatch( p_sched , p_parent_batch , p_batch ) ;
			if( nret )
			{
				ErrorLog( __FILE__ , __LINE__ , "DC4CLinkDagBatch failed[%d]" , nret );
				return nret;
			}
			
			if( STRCMP( p_sched->schedule_config.batches.batches_direction[i].to , != , "" ) )
			{
				nret = _ParseBatchesDirection( p_sched , p_batch , p_end_batch , p_sched->schedule_config.batches.batches_direction[i].to ) ;
				if( nret )
				{
					ErrorLog( __FILE__ , __LINE__ , "_ParseBatchesDirection failed[%d]" , nret );
					return nret;
				}
			}
		}
	}
	
	return 0;
}

int DC4CLoadDagScheduleFromFile( struct Dc4cDagSchedule **pp_sched , char *pathfilename )
{
	struct Dc4cDagSchedule	*p_sched = NULL ;
	
	FILE			*fp = NULL ;
	int			filesize ;
	int			readsize ;
	char			*filebuffer = NULL ;
	
	struct Dc4cDagBatch	*p_begin_batch = NULL ;
	struct Dc4cDagBatch	*p_end_batch = NULL ;
	SList			*p_begin_node = NULL ;
	
	int			nret = 0 ;
	
	p_sched = (struct Dc4cDagSchedule *)malloc( sizeof(struct Dc4cDagSchedule) ) ;
	if( p_sched == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return DC4C_ERROR_ALLOC;
	}
	memset( p_sched , 0x00 , sizeof(struct Dc4cDagSchedule) );
	
	fp = fopen( pathfilename , "r" ) ;
	if( fp == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "fopen[%s] failed , errno[%d]" , pathfilename , errno );
		DC4CUnloadDagSchedule( & p_sched );
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
		fclose(fp);
		DC4CUnloadDagSchedule( & p_sched );
		return DC4C_ERROR_ALLOC;
	}
	memset( filebuffer , 0x00 , filesize + 1 );
	
	readsize = fread( filebuffer , 1 , filesize , fp ) ;
	if( readsize < filesize )
	{
		ErrorLog( __FILE__ , __LINE__ , "fread failed , errno[%d]" , errno );
		fclose(fp);
		DC4CUnloadDagSchedule( & p_sched );
		return DC4C_ERROR_INTERNAL;
	}
	
	fclose(fp);
	
	g_fasterjson_encoding = FASTERJSON_ENCODING_GB18030 ;
	
	DSCINIT_dag_schedule_config( & (p_sched->schedule_config) );
	nret = DSCDESERIALIZE_JSON_dag_schedule_config( NULL , filebuffer , & readsize , & (p_sched->schedule_config) ) ;
	free( filebuffer );
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_dag_schedule_config failed[%d]" , nret );
		DC4CUnloadDagSchedule( & p_sched );
		return DC4C_ERROR_FILE_NOT_FOUND;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCDESERIALIZE_JSON_dag_schedule_config ok" );
	}
	
	/*
	DSCLOG_dag_schedule_config( & (p_sched->schedule_config) );
	*/
	strncpy( p_sched->schedule_name , p_sched->schedule_config.schedule.schedule_name , sizeof(p_sched->schedule_name)-1 );
	strncpy( p_sched->schedule_desc , p_sched->schedule_config.schedule.schedule_desc , sizeof(p_sched->schedule_desc)-1 );
	
	p_begin_batch = DC4CAllocDagBatch( p_sched , "" , NULL , 0 , 0 ) ;
	if( p_begin_batch == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CAllocDagBatch failed , errno[%d]" , errno );
		DC4CUnloadDagSchedule( & p_sched );
		return DC4C_ERROR_ALLOC;
	}
	
	p_begin_node = AddListNode( & (p_sched->root_batches_list) , p_begin_batch , sizeof(struct Dc4cDagBatch) , & DC4CFreeDagBatch ) ;
	if( p_begin_node == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CAllocDagBatch failed , errno[%d]" , errno );
		DC4CUnloadDagSchedule( & p_sched );
		return DC4C_ERROR_ALLOC;
	}
	
	p_end_batch = DC4CAllocDagBatch( p_sched , "" , NULL , 0 , 0 ) ;
	if( p_end_batch == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "DC4CAllocDagBatch failed , errno[%d]" , errno );
		DC4CUnloadDagSchedule( & p_sched );
		return DC4C_ERROR_ALLOC;
	}
	
	nret = _ParseBatchesDirection( p_sched , p_begin_batch , p_end_batch , "" ) ;	
	if( nret )
	{
		ErrorLog( __FILE__ , __LINE__ , "_ParseBatchesDirection failed[%d]" , nret );
		DC4CUnloadDagSchedule( & p_sched );
		return nret;
	}
	
	(*pp_sched) = p_sched ;
	
	return 0;
}

int DC4CExecuteDagSchedule( struct Dc4cDagSchedule *p_sched )
{
	return 0;
}

static void _LogDagBatch( int depth , struct Dc4cDagBatch *p_batch )
{
	SListNode		*p_postdepend_node = NULL ;
	struct Dc4cDagBatch	*p_postdepend_batch = NULL ;
	
	InfoLog( __FILE__ , __LINE__ , "- [%d] - p_batch[%p] batch_name[%s]" , depth , p_batch , p_batch->batch_name );
	
	for( p_postdepend_node = FindFirstListNode(p_batch->postdepend_batches_list) ; p_postdepend_node ; p_postdepend_node = FindNextListNode(p_postdepend_node) )
	{
		InfoLog( __FILE__ , __LINE__ , "- [%d] - p_batch[%p] batch_name[%s] - p_postdepend_node[%p] p_batch[%p]" , depth , p_batch , p_batch->batch_name , p_postdepend_node , GetNodeMember(p_postdepend_node) );
	}
	for( p_postdepend_node = FindFirstListNode(p_batch->postdepend_batches_list) ; p_postdepend_node ; p_postdepend_node = FindNextListNode(p_postdepend_node) )
	{
		p_postdepend_batch = (struct Dc4cDagBatch *)GetNodeMember( p_postdepend_node ) ;
		_LogDagBatch( depth + 1 , p_postdepend_batch );
	}
	
	return;
}

void DC4CLogDagSchedule( struct Dc4cDagSchedule *p_sched )
{
	SListNode		*p_node = NULL ;
	struct Dc4cDagBatch	*p_batch = NULL ;
	
	InfoLog( __FILE__ , __LINE__ , "--- DC4CLogDagSchedule ------------------" );
	InfoLog( __FILE__ , __LINE__ , "- p_sched[%p] schedule_name[%s]" , p_sched , p_sched->schedule_name );
	
	for( p_node = FindFirstListNode(p_sched->root_batches_list) ; p_node ; p_node = FindNextListNode(p_node) )
	{
		InfoLog( __FILE__ , __LINE__ , "- p_sched[%p] schedule_name[%s] - p_node[%p] p_batch[%p]" , p_sched , p_sched->schedule_name , p_node , GetNodeMember(p_node) );
	}
	for( p_node = FindFirstListNode(p_sched->root_batches_list) ; p_node ; p_node = FindNextListNode(p_node) )
	{
		p_batch = (struct Dc4cDagBatch *)GetNodeMember( p_node ) ;
		_LogDagBatch( 1 , p_batch );
	}
	
	return;
}

int DC4CUnloadDagSchedule( struct Dc4cDagSchedule **pp_sched )
{
	if( (*pp_sched) )
	{
		if( (*pp_sched)->root_batches_list )
		{
			DestroyList( & ((*pp_sched)->root_batches_list) , NULL );
			(*pp_sched)->root_batches_list = NULL ;
		}
		
		if( (*pp_sched)->executing_batches_list )
		{
			DestroyList( & ((*pp_sched)->executing_batches_list) , NULL );
			(*pp_sched)->executing_batches_list = NULL ;
		}
		
		free( (*pp_sched) );
		(*pp_sched) = NULL ;
	}
	
	return 0;
}

int DC4CGetDagScheduleProgress( struct Dc4cDagSchedule *p_sched )
{
	return p_sched->progress;
}

int DC4CGetDagScheduleResult( struct Dc4cDagSchedule *p_sched )
{
	return p_sched->result;
}

int DC4CInitDagSchedule( struct Dc4cDagSchedule *p_sched , char *schedule_name , char *schedule_desc )
{
	memset( p_sched , 0x00 , sizeof(struct Dc4cDagSchedule) );
	strncpy( p_sched->schedule_name , schedule_name , sizeof(p_sched->schedule_name)-1 );
	strncpy( p_sched->schedule_desc , schedule_desc , sizeof(p_sched->schedule_desc)-1 );
	return 0;
}

void DC4CCleanDagSchedule( struct Dc4cDagSchedule *p_sched )
{
	DestroyList( & (p_sched->root_batches_list) , NULL );
	DestroyList( & (p_sched->executing_batches_list) , NULL );
	
	memset( p_sched , 0x00 , sizeof(struct Dc4cDagSchedule) );
	
	return;
}

struct Dc4cDagBatch *DC4CAllocDagBatch( struct Dc4cDagSchedule *p_sched , char *batch_name , char *batch_desc , int view_pos_x , int view_pos_y )
{
	struct Dc4cDagBatch	*p_batch = NULL ;
	
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
	
	return p_batch;
}

BOOL DC4CFreeDagBatch( void *pv )
{
	struct Dc4cDagBatch	*p_batch = (struct Dc4cDagBatch *) pv ;
	
	SList			*p_parents_node = NULL ;
	struct Dc4cDagBatch	*p_parents_batch = NULL ;
	SList			*p_parents_postdepend_node ;
	struct Dc4cDagBatch	*p_parents_postdepend_batch = NULL ;
	int			parents_postdepend_exist_flag ;
	
	parents_postdepend_exist_flag = 0 ;
	if( p_batch )
	{
		DestroyList( & (p_batch->postdepend_batches_list) , NULL );
		
		for( p_parents_node = FindFirstListNode(p_batch->predepend_batches_list) ; p_parents_node ; p_parents_node = FindNextListNode(p_parents_node) )
		{
			p_parents_batch = (struct Dc4cDagBatch *)GetNodeMember(p_parents_node) ;
			
			for( p_parents_postdepend_node = FindFirstListNode(p_parents_batch->postdepend_batches_list) ; p_parents_postdepend_node ; p_parents_postdepend_node = FindNextListNode(p_parents_postdepend_node) )
			{
				p_parents_postdepend_batch = (struct Dc4cDagBatch *)GetNodeMember(p_parents_postdepend_node) ;
				
				if( p_parents_postdepend_batch == p_batch )
				{
					if( parents_postdepend_exist_flag == 1 )
					{
						p_parents_postdepend_node->FreeNodeMember = NULL ;
						DeleteListNode( & (p_parents_batch->postdepend_batches_list) , & (p_parents_postdepend_node) , NULL );
					}
					else
					{
						parents_postdepend_exist_flag = 1 ;
					}
					break;
				}
			}
		}
		
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
	
	p_node = AddListNode( & (p_parent_batch->postdepend_batches_list) , p_batch , sizeof(struct Dc4cDagBatch) , & DC4CFreeDagBatch ) ;
	if( p_node == NULL )
		return -1;
	
	p_node = AddListNode( & (p_batch->predepend_batches_list) , p_parent_batch , sizeof(struct Dc4cDagBatch) , NULL ) ;
	if( p_node == NULL )
		return -1;
	
	return 0;
}

void DC4CSetBatchTasks( struct Dc4cDagBatch *p_batch , struct Dc4cBatchTask *tasks_array , int tasks_count )
{
	if( p_batch->tasks_array )
	{
		free( p_batch->tasks_array );
	}
	
	p_batch->tasks_array = tasks_array ;
	p_batch->tasks_count = tasks_count ;
	
	return;
}

struct Dc4cApiEnv **DC4CGetApiEnvPPtr( struct Dc4cDagBatch *p_batch )
{
	return & (p_batch->penv);
}

void DC4CGetBatchBeginDatetime( struct Dc4cDagBatch *p_batch , char begin_datetime[19+1] , long *p_begin_datetime_stamp )
{
	strcpy( begin_datetime , p_batch->begin_datetime );
	(*p_begin_datetime_stamp) = p_batch->begin_datetime_stamp ;
	return;
}

void DC4CGetBatchEndDatetime( struct Dc4cDagBatch *p_batch , char end_datetime[19+1] , long *p_end_datetime_stamp )
{
	strcpy( end_datetime , p_batch->end_datetime );
	(*p_end_datetime_stamp) = p_batch->end_datetime_stamp ;
	return;
}

int DC4CGetBatchProgress( struct Dc4cDagBatch *p_batch )
{
	return p_batch->progress;
}

int DC4CGetBatchResult( struct Dc4cDagBatch *p_batch )
{
	return p_batch->result;
}

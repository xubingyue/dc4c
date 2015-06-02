/*
 * tfc_dag - DAG tasks engine for dc4c
 * author	: calvin
 * email	: calvinwilliams@163.com
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "dc4c_tfc_dag.h"
#include "IDL_dag_schedule.dsc.ESQL.eh"
#include "IDL_dag_batches_info.dsc.ESQL.eh"
#include "IDL_dag_batches_tasks.dsc.ESQL.eh"
#include "IDL_dag_batches_direction.dsc.ESQL.eh"

int DC4CLoadDagScheduleFromDatabase( struct Dc4cDagSchedule **pp_sched , char *schedule_name , char *rservers_ip_port , int options )
{
	dag_schedule_configfile *p_config = NULL ;
	
	dag_schedule		schedule ;
	dag_batches_info	batches_info ;
	dag_batches_tasks	batches_tasks ;
	dag_batches_direction	batches_direction ;
	
	int			nret = 0 ;
	
	p_config = (dag_schedule_configfile *)malloc( sizeof(dag_schedule_configfile) ) ;
	if( p_config == NULL )
	{
		ErrorLog( __FILE__ , __LINE__ , "alloc failed , errno[%d]" , errno );
		return DC4C_ERROR_ALLOC;
	}
	
	DSCINIT_dag_schedule_configfile( p_config );
	
	memset( & schedule , 0x00 , sizeof(dag_schedule) );
	strncpy( schedule.schedule_name , schedule_name , sizeof(schedule.schedule_name)-1 );
	DSCSQLACTION_SELECT_A_FROM_dag_schedule_WHERE_schedule_name_E( & schedule );
	if( SQLCODE )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_SELECT_A_FROM_dag_schedule_WHERE_schedule_name_E failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
		free( p_config );
		return DC4C_ERROR_DATABASE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_SELECT_A_FROM_dag_schedule_WHERE_schedule_name_E ok" );
	}
	
	strncpy( p_config->schedule.schedule_name , schedule.schedule_name , sizeof(p_config->schedule.schedule_name)-1 );
	strncpy( p_config->schedule.schedule_desc , schedule.schedule_desc , sizeof(p_config->schedule.schedule_desc)-1 );
	strncpy( p_config->schedule.begin_datetime , schedule.begin_datetime , sizeof(p_config->schedule.begin_datetime)-1 );
	strncpy( p_config->schedule.end_datetime , schedule.end_datetime , sizeof(p_config->schedule.end_datetime)-1 );
	p_config->schedule.progress = schedule.progress ;
	
	memset( & batches_info , 0x00 , sizeof(dag_batches_info) );
	strncpy( batches_info.schedule_name , schedule.schedule_name , sizeof(batches_info.schedule_name)-1 );
	DSCSQLACTION_OPEN_CURSOR_dag_batches_info_cursor1_SELECT_A_FROM_dag_batches_info_WHERE_schedule_name_E( & batches_info );
	if( SQLCODE )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_info_cursor1_SELECT_A_FROM_dag_batches_info_WHERE_schedule_name_E failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
		free( p_config );
		return DC4C_ERROR_DATABASE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_info_cursor1_SELECT_A_FROM_dag_batches_info_WHERE_schedule_name_E ok" );
	}
	
	for(	p_config->batches._batches_info_count = 0
		; p_config->batches._batches_info_count < p_config->batches._batches_info_size
		; p_config->batches._batches_info_count++ )
	{
		DSCSQLACTION_FETCH_CURSOR_dag_batches_info_cursor1( & batches_info );
		if( SQLCODE == SQLNOTFOUND )
		{
			break;
		}
		else if( SQLCODE )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_info_cursor1 failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
			free( p_config );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_info_cursor1 ok" );
		}
		
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].batch_name , batches_info.batch_name , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].batch_name)-1 );
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].batch_desc , batches_info.batch_desc , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].batch_desc)-1 );
		p_config->batches.batches_info[p_config->batches._batches_info_count].view_pos_x = batches_info.view_pos_x ;
		p_config->batches.batches_info[p_config->batches._batches_info_count].view_pos_y = batches_info.view_pos_y ;
		
		memset( & batches_tasks , 0x00 , sizeof(dag_batches_tasks) );
		strncpy( batches_tasks.schedule_name , schedule.schedule_name , sizeof(batches_tasks.schedule_name)-1 );
		strncpy( batches_tasks.batch_name , batches_info.batch_name , sizeof(batches_tasks.batch_name)-1 );
		DSCSQLACTION_OPEN_CURSOR_dag_batches_tasks_cursor2_SELECT_A_FROM_dag_batches_tasks_WHERE_schedule_name_E_AND_batch_name_E_ORDER_BY_order_index( & batches_tasks );
		if( SQLCODE )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_tasks_cursor2_SELECT_A_FROM_dag_batches_tasks_WHERE_schedule_name_E_AND_batch_name_E_ORDER_BY_order_index failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
			free( p_config );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_tasks_cursor2_SELECT_A_FROM_dag_batches_tasks_WHERE_schedule_name_E_AND_batch_name_E_ORDER_BY_order_index ok" );
		}
		
		for(	p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count = 0
			; p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count < p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_size
			; )
		{
			DSCSQLACTION_FETCH_CURSOR_dag_batches_tasks_cursor2( & batches_tasks );
			if( SQLCODE == SQLNOTFOUND )
			{
				break;
			}
			else if( SQLCODE )
			{
				ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_tasks_cursor2 failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
				free( p_config );
				return DC4C_ERROR_DATABASE;
			}
			else
			{
				DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_tasks_cursor2 ok" );
			}
			
			if( batches_tasks.progress == DC4C_DAGTASK_PROGRESS_FINISHED )
				continue;
			
			strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].program_and_params , batches_tasks.program_and_params , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].program_and_params)-1 );
			p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].timeout = batches_tasks.timeout ;
			p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].order_index = batches_tasks.order_index ;
			p_config->batches.batches_info[p_config->batches._batches_info_count].tasks[p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count].progress = batches_tasks.progress ;
			
			p_config->batches.batches_info[p_config->batches._batches_info_count]._tasks_count++;
		}
		
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].begin_datetime , batches_info.begin_datetime , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].begin_datetime)-1 );
		strncpy( p_config->batches.batches_info[p_config->batches._batches_info_count].end_datetime , batches_info.end_datetime , sizeof(p_config->batches.batches_info[p_config->batches._batches_info_count].end_datetime)-1 );
		p_config->batches.batches_info[p_config->batches._batches_info_count].progress = batches_info.progress ;
		
		DSCSQLACTION_CLOSE_CURSOR_dag_batches_tasks_cursor2();
		DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_CLOSE_CURSOR_dag_batches_tasks_cursor2 ok" );
	}
	
	DSCSQLACTION_CLOSE_CURSOR_dag_batches_info_cursor1();
	DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_CLOSE_CURSOR_dag_batches_info_cursor1 ok" );
	
	memset( & batches_direction , 0x00 , sizeof(dag_batches_direction) );
	strncpy( batches_direction.schedule_name , schedule.schedule_name , sizeof(batches_direction.schedule_name)-1 );
	DSCSQLACTION_OPEN_CURSOR_dag_batches_direction_cursor1_SELECT_A_FROM_dag_batches_direction_WHERE_schedule_name_E( & batches_direction );
	if( SQLCODE )
	{
		ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_direction_cursor1_SELECT_A_FROM_dag_batches_direction_WHERE_schedule_name_E failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
		free( p_config );
		return DC4C_ERROR_DATABASE;
	}
	else
	{
		DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_OPEN_CURSOR_dag_batches_direction_cursor1_SELECT_A_FROM_dag_batches_direction_WHERE_schedule_name_E ok" );
	}
	
	for(	p_config->batches._batches_direction_count = 0
		; p_config->batches._batches_direction_count < p_config->batches._batches_direction_size
		; p_config->batches._batches_direction_count++ )
	{
		DSCSQLACTION_FETCH_CURSOR_dag_batches_direction_cursor1( & batches_direction );
		if( SQLCODE == SQLNOTFOUND )
		{
			break;
		}
		else if( SQLCODE )
		{
			ErrorLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_direction_cursor1 failed , SQLCODE[%d][%s][%s]" , SQLCODE , SQLSTATE , SQLDESC );
			free( p_config );
			return DC4C_ERROR_DATABASE;
		}
		else
		{
			DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_FETCH_CURSOR_dag_batches_direction_cursor1 ok" );
		}
		
		strncpy( p_config->batches.batches_direction[p_config->batches._batches_direction_count].from_batch , batches_direction.from_batch , sizeof(p_config->batches.batches_direction[p_config->batches._batches_direction_count].from_batch)-1 );
		strncpy( p_config->batches.batches_direction[p_config->batches._batches_direction_count].to_batch , batches_direction.to_batch , sizeof(p_config->batches.batches_direction[p_config->batches._batches_direction_count].to_batch)-1 );
	}
	
	DSCSQLACTION_CLOSE_CURSOR_dag_batches_direction_cursor1();
	DebugLog( __FILE__ , __LINE__ , "DSCSQLACTION_CLOSE_CURSOR_dag_batches_direction_cursor1 ok" );
	
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

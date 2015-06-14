STRUCT	dag_batches_info
{
	STRING	64	schedule_name	NOTNULL
	STRING	64	batch_name	NOTNULL
	STRING	64	batch_desc
	INT	4	view_pos_x
	INT	4	view_pos_y
	INT	4	interrupt_by_app
	STRING	19	begin_datetime
	STRING	19	end_datetime
	INT	4	progress
	
	CREATE_SQL	"CREATE UNIQUE INDEX dag_batches_info_idx1 ON dag_batches_info ( schedule_name , batch_name ) ;"
	DROP_SQL	"DROP INDEX dag_batches_info_idx1 ;"
	
	SQLACTION	"CURSOR dag_batches_info_cursor1 SELECT * FROM dag_batches_info WHERE schedule_name ="
	SQLACTION	"UPDATE dag_batches_info SET begin_datetime,end_datetime,progress WHERE schedule_name = AND batch_name ="
}


STRUCT	dag_batches_info
{
	STRING	64	schedule_name
	STRING	64	batch_name
	STRING	64	batch_desc
	INT	4	view_pos_x
	INT	4	view_pos_y
	
	CREATE_SQL	"CREATE UNIQUE INDEX dag_batches_info_idx1 ON dag_batches_info ( schedule_name , batch_name ) ;"
	DROP_SQL	"DROP INDEX dag_batches_info_idx1 ;"
	
	SQLACTION	"CURSOR dag_batches_info_cursor1 SELECT * FROM dag_batches_info WHERE schedule_name ="
}


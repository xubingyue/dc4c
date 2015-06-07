STRUCT	dag_batches_direction
{
	STRING	64	schedule_name	NOTNULL
	STRING	64	from_batch	NOTNULL
	STRING	64	to_batch
	
	CREATE_SQL	"CREATE INDEX dag_batches_direction_idx1 ON dag_batches_direction ( schedule_name , from_batch ) ;"
	DROP_SQL	"DROP INDEX dag_batches_direction_idx1 ;"
	
	SQLACTION	"CURSOR dag_batches_direction_cursor1 SELECT * FROM dag_batches_direction WHERE schedule_name ="
}


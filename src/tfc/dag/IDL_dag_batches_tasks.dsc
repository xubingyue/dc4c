STRUCT	dag_batches_tasks
{
	STRING	64	schedule_name
	STRING	64	batch_name
	STRING	256	program_and_params
	INT	4	timeout
	
	CREATE_SQL	"CREATE INDEX dag_batches_tasks_idx1 ON dag_batches_tasks ( schedule_name , batch_name ) ;"
	DROP_SQL	"DROP INDEX dag_batches_tasks_idx1 ;"
	
	SQLACTION	"CURSOR dag_batches_tasks_cursor1 SELECT * FROM dag_batches_tasks WHERE schedule_name = AND batch_name ="
}


STRUCT	dag_batches_tasks
{
	STRING	64	schedule_name		NOTNULL
	STRING	64	batch_name		NOTNULL
	INT	4	order_index		NOTNULL
	STRING	256	program_and_params	NOTNULL
	INT	4	timeout			NOTNULL
	
	CREATE_SQL	"CREATE UNIQUE INDEX dag_batches_tasks_idx1 ON dag_batches_tasks ( schedule_name , batch_name , order_index ) ;"
	DROP_SQL	"DROP INDEX dag_batches_tasks_idx1 ;"
	
	SQLACTION	"CURSOR dag_batches_tasks_cursor1 SELECT * FROM dag_batches_tasks WHERE schedule_name ="
	SQLACTION	"CURSOR dag_batches_tasks_cursor2 SELECT * FROM dag_batches_tasks WHERE schedule_name = AND batch_name = ORDER BY order_index"
}


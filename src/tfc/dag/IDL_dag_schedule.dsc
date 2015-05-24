STRUCT	dag_schedule
{
	INT	4	order_index
	STRING	64	schedule_name
	STRING	256	schedule_desc
	
	SQLCONN
	
	CREATE_SQL	"CREATE UNIQUE INDEX dag_schedule_idx1 ON dag_schedule ( schedule_name ) ;"
	DROP_SQL	"DROP INDEX dag_schedule_idx1 ;"
	
	SQLACTION	"SELECT * FROM dag_schedule WHERE schedule_name ="
	SQLACTION	"CURSOR dag_schedule_cursor1 SELECT * FROM dag_schedule ORDER BY order_index"
}


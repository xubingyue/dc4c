#rem*  This File "DB_Pm_Hzbat_Batches_Tasks.sql"
#rem*  Genenated by
#rem*  Application <dbaction> for MySQL
#rem*  with the action file "DB_Pm_Hzbat_Batches_Tasks.act".
#rem*  Create: Thu Jul  2 14:35:28 2015
 
CREATE TABLE pm_hzbat_batches_tasks 
  (
	schedule_name	 VARCHAR(64) NOT NULL,
	batch_name	 VARCHAR(64) NOT NULL,
	order_index	 INT NOT NULL,
	program_and_params	 VARCHAR(256) NOT NULL,
	timeout	 INT,
	begin_datetime	 VARCHAR(19),
	end_datetime	 VARCHAR(19),
	progress	 INT,
	error	 INT,
	status	 INT
  ); 

CREATE UNIQUE INDEX Pm_Hzbat_Batches_Tasks_I0 ON pm_hzbat_batches_tasks(schedule_name,batch_name,order_index);

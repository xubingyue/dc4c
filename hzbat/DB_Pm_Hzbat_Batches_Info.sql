#rem*  This File "DB_Pm_Hzbat_Batches_Info.sql"
#rem*  Genenated by
#rem*  Application <dbaction> for MySQL
#rem*  with the action file "DB_Pm_Hzbat_Batches_Info.act".
#rem*  Create: Thu Jul  2 14:35:27 2015
 
CREATE TABLE pm_hzbat_batches_info 
  (
	schedule_name	 VARCHAR(64) NOT NULL,
	batch_name	 VARCHAR(64) NOT NULL,
	batch_desc	 VARCHAR(64),
	view_pos_x	 INT,
	view_pos_y	 INT,
	interrupt_by_app	 INT,
	begin_datetime	 VARCHAR(19),
	end_datetime	 VARCHAR(19),
	progress	 INT,
	pretask_program_and_params	 VARCHAR(256),
	pretask_timeout	 INT,
	pretask_progress	 INT,
	pretask_error	 INT,
	pretask_status	 INT
  ); 

CREATE UNIQUE INDEX Pm_Hzbat_Batches_Info_I0 ON pm_hzbat_batches_info(schedule_name,batch_name);

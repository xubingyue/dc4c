#rem*  This File "DB_Pm_Hzbat_Batches_Filter.sql"
#rem*  Genenated by
#rem*  Application <dbaction> for MySQL
#rem*  with the action file "DB_Pm_Hzbat_Batches_Filter.act".
#rem*  Create: Thu Jul  2 14:35:28 2015
 
CREATE TABLE pm_hzbat_batches_filter 
  (
	schedule_name	 VARCHAR(64) NOT NULL,
	batch_name	 VARCHAR(64) NOT NULL,
	filter_type	 VARCHAR(16) NOT NULL,
	filter_param	 VARCHAR(1024)
  ); 

CREATE UNIQUE INDEX Pm_Hzbat_Batches_Filter_I0 ON pm_hzbat_batches_filter(schedule_name,batch_name);

#rem*  This File "DB_Pm_Hzbat_Batches_Direction.sql"
#rem*  Genenated by
#rem*  Application <dbaction> for MySQL
#rem*  with the action file "DB_Pm_Hzbat_Batches_Direction.act".
#rem*  Create: Thu Jul  2 14:35:27 2015
 
CREATE TABLE pm_hzbat_batches_direction 
  (
	schedule_name	 VARCHAR(64) NOT NULL,
	from_batch	 VARCHAR(64),
	to_batch	 VARCHAR(64)
  ); 

CREATE INDEX Pm_Hzbat_Batches_Direction_I0 ON pm_hzbat_batches_direction(schedule_name,from_batch);

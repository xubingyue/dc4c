#rem*  This File "DB_Pm_Hzbat_Schedule.sql"
#rem*  Genenated by
#rem*  Application <dbaction> for MySQL
#rem*  with the action file "DB_Pm_Hzbat_Schedule.act".
#rem*  Create: Thu Jul  2 14:35:27 2015
 
CREATE TABLE pm_hzbat_schedule 
  (
	order_index	 INT NOT NULL,
	schedule_name	 VARCHAR(64) NOT NULL,
	schedule_desc	 VARCHAR(256) NOT NULL,
	begin_datetime	 VARCHAR(19),
	end_datetime	 VARCHAR(19),
	progress	 INT
  ); 

CREATE UNIQUE INDEX Pm_Hzbat_Schedule_I0 ON pm_hzbat_schedule(order_index);
CREATE UNIQUE INDEX Pm_Hzbat_Schedule_I1 ON pm_hzbat_schedule(schedule_name);

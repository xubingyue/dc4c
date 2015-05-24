truncate table dag_schedule ;
truncate table dag_batches_info ;
truncate table dag_batches_tasks ;
truncate table dag_batches_direction ;

INSERT INTO dag_schedule VALUES ( 1 , 'test_schedule_name' , 'test_schedule_desc' );

INSERT INTO dag_batches_info VALUES ( 'test_schedule_name' , 'BATCH_A1' , '' , 0 , 0 );
INSERT INTO dag_batches_info VALUES ( 'test_schedule_name' , 'BATCH_B1' , '' , 0 , 0 );
INSERT INTO dag_batches_info VALUES ( 'test_schedule_name' , 'BATCH_A21' , '' , 0 , 0 );
INSERT INTO dag_batches_info VALUES ( 'test_schedule_name' , 'BATCH_A22' , '' , 0 , 0 );
INSERT INTO dag_batches_info VALUES ( 'test_schedule_name' , 'BATCH_A3' , '' , 0 , 0 );
INSERT INTO dag_batches_info VALUES ( 'test_schedule_name' , 'BATCH_B2' , '' , 0 , 0 );

INSERT INTO dag_batches_tasks VALUES ( 'test_schedule_name' , 'BATCH_A1' , 'dc4c_test_worker_sleep 1' , 60 );
INSERT INTO dag_batches_tasks VALUES ( 'test_schedule_name' , 'BATCH_A21' , 'dc4c_test_worker_sleep 4' , 60 );
INSERT INTO dag_batches_tasks VALUES ( 'test_schedule_name' , 'BATCH_A22' , 'dc4c_test_worker_sleep 5' , 60 );
INSERT INTO dag_batches_tasks VALUES ( 'test_schedule_name' , 'BATCH_A3' , 'dc4c_test_worker_sleep 2' , 60 );
INSERT INTO dag_batches_tasks VALUES ( 'test_schedule_name' , 'BATCH_A3' , 'dc4c_test_worker_sleep 1' , 60 );
INSERT INTO dag_batches_tasks VALUES ( 'test_schedule_name' , 'BATCH_B1' , 'dc4c_test_worker_sleep 3' , 60 );
INSERT INTO dag_batches_tasks VALUES ( 'test_schedule_name' , 'BATCH_B2' , 'dc4c_test_worker_sleep 6' , 60 );

INSERT INTO dag_batches_direction VALUES ( 'test_schedule_name' , '' , 'BATCH_A1' );
INSERT INTO dag_batches_direction VALUES ( 'test_schedule_name' , '' , 'BATCH_B1' );
INSERT INTO dag_batches_direction VALUES ( 'test_schedule_name' , 'BATCH_A1' , 'BATCH_A21' );
INSERT INTO dag_batches_direction VALUES ( 'test_schedule_name' , 'BATCH_A1' , 'BATCH_A22' );
INSERT INTO dag_batches_direction VALUES ( 'test_schedule_name' , 'BATCH_A21' , 'BATCH_A3' );
INSERT INTO dag_batches_direction VALUES ( 'test_schedule_name' , 'BATCH_A22' , 'BATCH_A3' );
INSERT INTO dag_batches_direction VALUES ( 'test_schedule_name' , 'BATCH_B1' , 'BATCH_B2' );
INSERT INTO dag_batches_direction VALUES ( 'test_schedule_name' , 'BATCH_A3' , '' );
INSERT INTO dag_batches_direction VALUES ( 'test_schedule_name' , 'BATCH_B2' , '' );


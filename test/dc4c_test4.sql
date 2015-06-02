truncate table dag_schedule ;
truncate table dag_batches_info ;
truncate table dag_batches_tasks ;
truncate table dag_batches_direction ;

INSERT INTO dag_schedule VALUES ( 1 , 'TEST_SCHEDULE_NAME' , 'TEST_SCHEDULE_DESC' , '' , '' , 0 );

INSERT INTO dag_batches_info VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A1' , 'BATCH_A1_DESC' , 0 , 0 , '' , '' , 0 );
INSERT INTO dag_batches_info VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_B1' , 'BATCH_B1_DESC' , 0 , 0 , '' , '' , 0 );
INSERT INTO dag_batches_info VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A21' , 'BATCH_A21_DESC' , 0 , 0 , '' , '' , 0 );
INSERT INTO dag_batches_info VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A22' , 'BATCH_A22_DESC' , 0 , 0 , '' , '' , 0 );
INSERT INTO dag_batches_info VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A3' , 'BATCH_A3_DESC' , 0 , 0 , '' , '' , 0 );
INSERT INTO dag_batches_info VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_B2' , 'BATCH_B2_DESC' , 0 , 0 , '' , '' , 0 );

INSERT INTO dag_batches_tasks VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A1' , 1 , 'dc4c_test_worker_sleep 1' , 60 , '' , '' , 0 , 0 , 0 );
INSERT INTO dag_batches_tasks VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A22' , 1 , 'dc4c_test_worker_sleep 5' , 60 , '' , '' , 0 , 0 , 0 );
INSERT INTO dag_batches_tasks VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A3' , 1 , 'dc4c_test_worker_sleep 1' , 60 , '' , '' , 0 , 0 , 0 );
INSERT INTO dag_batches_tasks VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A3' , 2 , 'dc4c_test_worker_sleep 1' , 60 , '' , '' , 0 , 0 , 0 );
INSERT INTO dag_batches_tasks VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A3' , 3 , 'dc4c_test_worker_sleep 1' , 60 , '' , '' , 0 , 0 , 0 );
INSERT INTO dag_batches_tasks VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_B1' , 1 , 'dc4c_test_worker_sleep 3' , 60 , '' , '' , 0 , 0 , 0 );
INSERT INTO dag_batches_tasks VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_B2' , 1 , 'dc4c_test_worker_sleep 6' , 60 , '' , '' , 0 , 0 , 0 );

INSERT INTO dag_batches_direction VALUES ( 'TEST_SCHEDULE_NAME' , '' , 'BATCH_A1' );
INSERT INTO dag_batches_direction VALUES ( 'TEST_SCHEDULE_NAME' , '' , 'BATCH_B1' );
INSERT INTO dag_batches_direction VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A1' , 'BATCH_A21' );
INSERT INTO dag_batches_direction VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A1' , 'BATCH_A22' );
INSERT INTO dag_batches_direction VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A21' , 'BATCH_A3' );
INSERT INTO dag_batches_direction VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A22' , 'BATCH_A3' );
INSERT INTO dag_batches_direction VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_B1' , 'BATCH_B2' );
INSERT INTO dag_batches_direction VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_A3' , '' );
INSERT INTO dag_batches_direction VALUES ( 'TEST_SCHEDULE_NAME' , 'BATCH_B2' , '' );


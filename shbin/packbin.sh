cd ..
VERSION=`head -1 ChangeLog-CN | awk '{print $1}'`

cd
tar cvzf dc4c-BIN-${VERSION}.tar.gz \
	bin/dc4c_rserver \
	bin/dc4c_wserver \
	bin/dc4c_test_master \
	bin/dc4c_test_batch_master \
	bin/dc4c_test_multi_batch_master \
	bin/dc4c_test_tfc_dag_master \
	bin/dc4c_test_worker_pi \
	bin/dc4c_test_worker_sleep \
	lib/libdc4c_util.so \
	lib/libdc4c_proto.so \
	lib/libdc4c_api.so \
	lib/libdc4c_tfc_dag.so \


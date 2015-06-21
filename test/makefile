# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			dc4c_test_master.c \
			dc4c_test_batch_master.c \
			dc4c_test_batch_master_stepbystep.c \
			dc4c_test_multi_batch_master.c \
			dc4c_test_multi_batch_master_stepbystep.c \
			dc4c_test_tfc_dag_master.c \
			dc4c_test_tfc_dag_master_stepbystep.c \
			dc4c_test_worker_hello.c \
			dc4c_test_worker_sleep.c \
			dc4c_test_worker_sleep_or_error.c \
			dc4c_test_worker_pi.c \
			$(c_FILE2) \

c_FILE2		=	\
			IDL_dag_schedule.dsc.c \
			IDL_dag_batches_direction.dsc.c \
			IDL_dag_batches_info.dsc.c \
			IDL_dag_batches_tasks.dsc.c \

#@ ec_FILE
ec_FILE		=	\
			dc4c_test_tfc_dag_master_pgsql_stepbystep.ec \
			$(ec_FILE2) \
			
ec_FILE2	=	\
			IDL_dag_schedule.dsc.ESQL.ec \
			IDL_dag_batches_direction.dsc.ESQL.ec \
			IDL_dag_batches_info.dsc.ESQL.ec \
			IDL_dag_batches_tasks.dsc.ESQL.ec \

###### 目标文件、安装目录配置区
include ../makeinstall
BIN		=	\
			dc4c_test_master \
			dc4c_test_batch_master \
			dc4c_test_batch_master_stepbystep \
			dc4c_test_multi_batch_master \
			dc4c_test_multi_batch_master_stepbystep \
			dc4c_test_tfc_dag_master \
			dc4c_test_tfc_dag_master_stepbystep \
			dc4c_test_tfc_dag_master_pgsql_stepbystep \
			dc4c_test_worker_hello \
			dc4c_test_worker_sleep \
			dc4c_test_worker_sleep_or_error \
			dc4c_test_worker_pi \

BININST		=	$(_BININST)

###### 编译选项
CFLAGS		=	$(_CFLAGS) \
			-I$(_HDERINST_PROTO) \
			-I$(_HDERINST_UTIL) \
			-I$(_HDERINST_API) \
			-I$(_HDERINST_TFC_DAG) \
			-I$(_HDERBASE)/fasterjson \

###### 链接选项
LFLAGS		=	$(_LFLAGS) \
			-L$(_LIBINST) \
			-ldc4c_proto \
			-ldc4c_util \
			-ldc4c_api \
			-ldc4c_tfc_dag \
			-lfasterjson \

###### 额外宏定义区
CLEAN_ADDITION	=

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_BININST
#@ make_uninstall
#@ make_uninstall_BININST

include $(MKTPLDIR)/makeobj_$(MKTPLOS).inc

###### 目标文件依赖关系
dc4c_test_master	:	dc4c_test_master.o
	$(CC) -o $@ dc4c_test_master.o $(LFLAGS)

dc4c_test_batch_master	:	dc4c_test_batch_master.o
	$(CC) -o $@ dc4c_test_batch_master.o $(LFLAGS)

dc4c_test_batch_master_stepbystep	:	dc4c_test_batch_master_stepbystep.o
	$(CC) -o $@ dc4c_test_batch_master_stepbystep.o $(LFLAGS)

dc4c_test_multi_batch_master	:	dc4c_test_multi_batch_master.o
	$(CC) -o $@ dc4c_test_multi_batch_master.o $(LFLAGS)

dc4c_test_multi_batch_master_stepbystep	:	dc4c_test_multi_batch_master_stepbystep.o
	$(CC) -o $@ dc4c_test_multi_batch_master_stepbystep.o $(LFLAGS)

dc4c_test_tfc_dag_master	:	dc4c_test_tfc_dag_master.o
	$(CC) -o $@ dc4c_test_tfc_dag_master.o $(LFLAGS) -ldc4c_tfc_dag

dc4c_test_tfc_dag_master_stepbystep	:	dc4c_test_tfc_dag_master_stepbystep.o
	$(CC) -o $@ dc4c_test_tfc_dag_master_stepbystep.o $(LFLAGS) -ldc4c_tfc_dag

dc4c_test_tfc_dag_master_pgsql_stepbystep	:	dc4c_test_tfc_dag_master_pgsql_stepbystep.o $(ec_FILE2_o)
	$(CC) -o $@ dc4c_test_tfc_dag_master_pgsql_stepbystep.o $(LFLAGS) $(ec_FILE2_o) -lecpg

dc4c_test_worker_hello	:	dc4c_test_worker_hello.o
	$(CC) -o $@ dc4c_test_worker_hello.o $(LFLAGS)

dc4c_test_worker_sleep	:	dc4c_test_worker_sleep.o
	$(CC) -o $@ dc4c_test_worker_sleep.o $(LFLAGS)

dc4c_test_worker_sleep_or_error	:	dc4c_test_worker_sleep_or_error.o
	$(CC) -o $@ dc4c_test_worker_sleep_or_error.o $(LFLAGS)

dc4c_test_worker_pi	:	dc4c_test_worker_pi.o
	$(CC) -o $@ dc4c_test_worker_pi.o $(LFLAGS) -lgmp


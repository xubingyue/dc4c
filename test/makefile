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

ec_FILE		=	\
			dc4c_test_tfc_dag_master_pgsql_stepbystep.ec \
			$(ec_FILE2) \
			
ec_FILE2	=	\
			IDL_dag_schedule.dsc.ESQL.ec \
			IDL_dag_batches_direction.dsc.ESQL.ec \
			IDL_dag_batches_info.dsc.ESQL.ec \
			IDL_dag_batches_tasks.dsc.ESQL.ec \

###### 目标文件、安装目录配置区
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

BININST		=	$(HOME)/bin

###### 编译选项
CFLAGS		=	-I. \
			-I$(HOME)/include/fasterjson \
			-I$(HOME)/include/dc4c_util \
			-I$(HOME)/include/dc4c_proto \
			-I$(HOME)/include/dc4c_api \
			-I$(HOME)/include/dc4c_tfc_dag \

###### 链接选项
LFLAGS		=	-L. \
			-L$(HOME)/lib \
			-lfasterjson \
			-ldc4c_util \
			-ldc4c_proto \
			-ldc4c_api \
			-ldc4c_tfc_dag \

###### 额外宏定义区
CLEAN_ADDITION	=

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_BININST

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


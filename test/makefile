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
			dc4c_test_multi_batch_master.c \
			dc4c_test_tfc_dag_master.c \
			dc4c_test_worker_sleep.c \
			dc4c_test_worker_pi.c \

###### 目标文件、安装目录配置区
BIN		=	\
			dc4c_test_master \
			dc4c_test_batch_master \
			dc4c_test_multi_batch_master \
			dc4c_test_tfc_dag_master \
			dc4c_test_worker_sleep \
			dc4c_test_worker_pi \

BININST		=	$(HOME)/bin

###### 编译选项
CFLAGS		=	-I. \
			-I$(HOME)/include/dc4c_util \
			-I$(HOME)/include/dc4c_proto \
			-I$(HOME)/include/dc4c_api \
			-I$(HOME)/include/dc4c_tfc_dag \

###### 链接选项
LFLAGS		=	-L. \
			-L$(HOME)/lib \
			-ldc4c_util \
			-ldc4c_proto \
			-ldc4c_api \

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

dc4c_test_multi_batch_master	:	dc4c_test_multi_batch_master.o
	$(CC) -o $@ dc4c_test_multi_batch_master.o $(LFLAGS)

dc4c_test_tfc_dag_master	:	dc4c_test_tfc_dag_master.o
	$(CC) -o $@ dc4c_test_tfc_dag_master.o $(LFLAGS) -ldc4c_tfc_dag

dc4c_test_worker_sleep	:	dc4c_test_worker_sleep.o
	$(CC) -o $@ dc4c_test_worker_sleep.o $(LFLAGS)

dc4c_test_worker_pi	:	dc4c_test_worker_pi.o
	$(CC) -o $@ dc4c_test_worker_pi.o $(LFLAGS) -lgmp


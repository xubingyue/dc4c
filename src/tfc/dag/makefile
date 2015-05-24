# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			dc4c_tfc_dag.c \
			IDL_dag_schedule_configfile.dsc.c \

ec_FILE		=	\
			dc4c_tfc_dag_pgsql.ec \
			IDL_dag_schedule.dsc.ESQL.ec \
			IDL_dag_batches_direction.dsc.ESQL.ec \
			IDL_dag_batches_info.dsc.ESQL.ec \
			IDL_dag_batches_tasks.dsc.ESQL.ec \
			
###### 目标文件、安装目录配置区
LIB		=	libdc4c_tfc_dag.so libdc4c_tfc_dag_pgsql.so
LIBINST		=	$(HOME)/lib
HDER		=	\
			dc4c_tfc_dag.h IDL_dag_schedule_configfile.dsc.h IDL_dag_schedule.dsc.ESQL.eh IDL_dag_schedule.dsc.h
HDERINST	=	$(HOME)/include/dc4c_tfc_dag
NOINST_OBJ	=	sizeof

###### 编译选项
CFLAGS		=	-I. \
			-I$(HOME)/include/fasterjson \
			-I$(HOME)/include/dc4c_util \
			-I$(HOME)/include/dc4c_api \
			-I/root/local/postgresql/include \

###### 链接选项
LFLAGS		=	-L. \
			-L$(HOME)/lib \
			-lfasterjson \
			-ldc4c_util \
			-ldc4c_api \

###### 额外宏定义区
CLEAN_ADDITION	=	sizeof.o

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_LIBINST
#@ make_install_HDERINST
#@ make_install_DFTHDERINST

include $(MKTPLDIR)/makeobj_$(MKTPLOS).inc

###### 目标文件依赖关系
libdc4c_tfc_dag.so		:	$(c_FILE_o)
	$(CC) -o $@ $(c_FILE_o) $(SOFLAGS) $(LFLAGS)

libdc4c_tfc_dag_pgsql.so	:	$(c_FILE_o) $(ec_FILE_o)
	$(CC) -o $@ $(c_FILE_o) $(ec_FILE_o) $(SOFLAGS) $(LFLAGS) -L/root/local/postgresql/lib -lecpg

sizeof		:	sizeof.o
	$(CC) -o $@ sizeof.o


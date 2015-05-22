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
			IDL_dag_schedule_config.dsc.c \

###### 目标文件、安装目录配置区
LIB		=	libdc4c_tfc_dag.so
LIBINST		=	$(HOME)/lib
HDER		=	\
			dc4c_tfc_dag.h
HDERINST	=	$(HOME)/include/dc4c_tfc_dag
NOINST_OBJ	=	sizeof

###### 编译选项
CFLAGS		=	-I. \
			-I$(HOME)/include/fasterjson \
			-I$(HOME)/include/dc4c_util \
			-I$(HOME)/include/dc4c_api \

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

sizeof		:	sizeof.o
	$(CC) -o $@ sizeof.o


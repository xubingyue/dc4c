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
			$(c_FILE2) \

c_FILE2		=	\
			IDL_dag_schedule_configfile.dsc.c \

ec_FILE		=	\
			dc4c_tfc_dag_pgsql.ec \

###### 目标文件、安装目录配置区
include ../../../makeinstall
LIB		=	libdc4c_tfc_dag.so
LIBINST		=	$(_LIBINST)
HDER		=	dc4c_tfc_dag.h $(c_FILE2:.c=.h) $(c_FILE2:.c=.LOG.c)
HDERINST	=	$(_HDERINST_TFC_DAG)
NOINST_OBJ	=	sizeof

###### 编译选项
CFLAGS		=	$(_CFLAGS) \
			-I$(_HDERINST_PROTO) \
			-I$(_HDERINST_UTIL) \
			-I$(_HDERINST_API) \
			-I$(_HDERBASE)/fasterjson \

###### 链接选项
LFLAGS		=	$(_LFLAGS) \
			-L$(_LIBINST) \
			-ldc4c_proto \
			-ldc4c_util \
			-ldc4c_api \
			-lfasterjson \

###### 额外宏定义区
CLEAN_ADDITION	=	sizeof.o

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_LIBINST
#@ make_install_HDERINST
#@ make_install_DFTHDERINST
#@ make_uninstall
#@ make_uninstall_LIBINST
#@ make_uninstall_HDERINST
#@ make_uninstall_DFTHDERINST

include $(MKTPLDIR)/makeobj_$(MKTPLOS).inc

###### 目标文件依赖关系
libdc4c_tfc_dag.so		:	$(c_FILE_o)
	$(CC) -o $@ $(c_FILE_o) $(SOFLAGS) $(LFLAGS)

sizeof		:	sizeof.o libdc4c_tfc_dag.so
	$(CC) -o $@ sizeof.o $(ESOFLAGS) $(LFLAGS) libdc4c_tfc_dag.so


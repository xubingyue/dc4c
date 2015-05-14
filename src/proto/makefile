# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			IDL_worker_register_request.dsc.c \
			IDL_worker_register_response.dsc.c \
			IDL_query_workers_request.dsc.c \
			IDL_query_workers_response.dsc.c \
			IDL_worker_notice_request.dsc.c \
			IDL_execute_program_request.dsc.c \
			IDL_execute_program_response.dsc.c \
			IDL_deploy_program_request.dsc.c \

###### 目标文件、安装目录配置区
LIB		=	libdc4c_proto.so
LIBINST		=	$(HOME)/lib
HDER		= \
			$(c_FILE:.c=.h) \
			$(c_FILE:.c=.LOG.c) \

HDERINST	=	$(HOME)/include/dc4c_proto

###### 编译选项
CFLAGS		=	-I. \
			-I/usr/include/fasterjson \

###### 链接选项
LFLAGS		=	-L. \
			-L/usr/lib \
			-lfasterjson \

###### 额外宏定义区
CLEAN_ADDITION	=

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_LIBINST
#@ make_install_HDERINST
#@ make_install_DFTHDERINST

include $(MKTPLDIR)/makeobj_$(MKTPLOS).inc

###### 目标文件依赖关系
libdc4c_proto.so	:	$(c_FILE_o)
	$(CC) -o $@ $(c_FILE_o) $(SOFLAGS) $(LFLAGS)


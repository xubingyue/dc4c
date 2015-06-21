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
include ../../makeinstall
LIB		=	libdc4c_proto.so
LIBINST		=	$(_LIBINST)
HDER		= \
			$(c_FILE:.c=.h) \
			$(c_FILE:.c=.LOG.c) \

HDERINST	=	$(_HDERINST_PROTO)

###### 编译选项
CFLAGS		=	$(_CFLAGS) \
			-I$(_HDERBASE)/fasterjson \

###### 链接选项
LFLAGS		=	$(_LFLAGS) \
			-L$(_LIBINST) \
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
#@ make_uninstall
#@ make_uninstall_LIBINST
#@ make_uninstall_HDERINST
#@ make_uninstall_DFTHDERINST

include $(MKTPLDIR)/makeobj_$(MKTPLOS).inc

###### 目标文件依赖关系
libdc4c_proto.so	:	$(c_FILE_o)
	$(CC) -o $@ $(c_FILE_o) $(SOFLAGS) $(LFLAGS)


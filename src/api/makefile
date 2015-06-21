# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			dc4c_api.c \

###### 目标文件、安装目录配置区
include ../../makeinstall
LIB		=	libdc4c_api.so
LIBINST		=	$(_LIBINST)
HDER		=	dc4c_api.h
HDERINST	=	$(_HDERINST_API)

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
libdc4c_api.so	:	$(c_FILE_o)
	$(CC) $(SOFLAGS) -o $@ $(c_FILE_o) $(ESOFLAGS) $(LFLAGS)


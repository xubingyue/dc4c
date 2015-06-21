# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			main.c \
			server.c \
			comm.c \
			proto.c \
			app.c \
			util.c \

###### 目标文件、安装目录配置区
include ../../makeinstall
BIN		=	dc4c_rserver
BININST		=	$(_BININST)

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
CLEAN_ADDITION	=

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_BININST
#@ make_install_HDERINST
#@ make_install_DFTHDERINST
#@ make_uninstall
#@ make_uninstall_BININST
#@ make_uninstall_HDERINST
#@ make_uninstall_DFTHDERINST

include $(MKTPLDIR)/makeobj_$(MKTPLOS).inc

###### 目标文件依赖关系
dc4c_rserver	:	$(c_FILE_o)
	$(CC) $(ESOFLAGS) -o $@ $(c_FILE_o) $(LFLAGS)


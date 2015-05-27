# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			LOGC.c \
			ListX.c \
			dc4c_util.c \

###### 目标文件、安装目录配置区
LIB		=	libdc4c_util.so
LIBINST		=	$(HOME)/lib
HDER		=	dc4c_util.h ListX.h
HDERINST	=	$(HOME)/include/dc4c_util
HDER2		=	LOGC.h
HDERINST2	=	$(HOME)/include/dc4c_api

###### 编译选项
CFLAGS		=	-I. \

###### 链接选项
LFLAGS		=	-L. \
			-lcrypto \

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
libdc4c_util.so	:	$(c_FILE_o)
	$(CC) $(SOFLAGS) -o $@ $(c_FILE_o) $(ESOFLAGS) $(LFLAGS)


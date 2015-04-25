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
			proto.c \
			app.c \

c_FILE2		=	\
			dc4c_test_master.c \
			dc4c_test_worker_sleep.c \

###### 目标文件、安装目录配置区
LIB		=	libdc4c_api.so
LIBINST		=	$(HOME)/lib
HDER		=	dc4c_api.h
HDERINST	=	$(HOME)/include/dc4c_api
BIN		=	dc4c_test_master dc4c_test_worker_sleep
BININST		=	$(HOME)/bin

###### 编译选项
CFLAGS		=	-I. \
			-I$(HOME)/include/dc4c_util \
			-I$(HOME)/include/dc4c_proto \

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
#@ make_install_LIBINST
#@ make_install_HDERINST
#@ make_install_DFTHDERINST

include $(MKTPLDIR)/makeobj_$(MKTPLOS).inc

###### 目标文件依赖关系
libdc4c_api.so	:	$(c_FILE_o)
	$(CC) $(SOFLAGS) -o $@ $(c_FILE_o) $(ESOFLAGS) $(LFLAGS)

dc4c_test_master	:	dc4c_test_master.o
	$(CC) -o $@ dc4c_test_master.o $(LFLAGS)

dc4c_test_worker_sleep	:	dc4c_test_worker_sleep.o
	$(CC) -o $@ dc4c_test_worker_sleep.o $(LFLAGS)


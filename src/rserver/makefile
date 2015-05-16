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
BIN		=	rserver
BININST		=	$(HOME)/bin

###### 编译选项
CFLAGS		=	-I. \
			-I$(HOME)/include/dc4c_proto \
			-I$(HOME)/include/dc4c_util \
			-I$(HOME)/include/dc4c_api \
			-I$(HOME)/include/fasterjson \

###### 链接选项
LFLAGS		=	-L. \
			-L$(HOME)/lib \
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

include $(MKTPLDIR)/makeobj_$(MKTPLOS).inc

###### 目标文件依赖关系
rserver	:	$(c_FILE_o)
	$(CC) $(ESOFLAGS) -o $@ $(c_FILE_o) $(LFLAGS)


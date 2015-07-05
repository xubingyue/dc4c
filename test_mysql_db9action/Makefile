ACT_SRC_FILE= \
	DB_Pm_Hzbat_Schedule.act \
	DB_Pm_Hzbat_Batches_Info.act \
	DB_Pm_Hzbat_Batches_Direction.act \
	DB_Pm_Hzbat_Batches_Tasks.act \
	DB_Pm_Hzbat_Batches_Filter.act \

PUB_INCLUDE_FILE=$(ACT_SRC_FILE:.act=.h) \

C_FILE=$(ACT_SRC_FILE:.act=.c)
SQL_FILE=$(ACT_SRC_FILE:.act=.sql)
LIB_FILE=libhzbat.so
SERVER_EXEC_FILE=hzbat

include $(PROJECT_DIR)/make/makeobj.inc 

LIB_OBJ=$(ACT_SRC_FILE:.act=.o)

C_FLAG= -I. \
	-I$(HOME)/include/fasterjson \
	-I$(HOME)/include/dc4c_util \
	-I$(HOME)/include/dc4c_api \
	-I$(HOME)/include/dc4c_tfc_dag \

LIBS =  -L. -L$(HOME)/lib \
	-lfasterjson -ldc4c_api -ldc4c_tfc_dag -ldbutil -lIButil -lmysqlclient -lhzb_log \

libhzbat.so : $(LIB_OBJ)
	$(CC) -shared -o $@ $(LIB_OBJ)

hzbat : hzbat.o libhzbat.so
	$(CC) -o hzbat hzbat.o libhzbat.so $(LIBS)


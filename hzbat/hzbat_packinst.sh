OSNAME=`uname -a | awk '{print $1}'`
HZBAT_VERSION=`hzbat | head -1 | tr 'v' ' ' | awk '{print $2}'`
TAR_FILENAME="hzbat-${OSNAME}-${HZBAT_VERSION}.tar.gz"

cd $HOME
tar cvzfh $TAR_FILENAME \
	bin/dc4c_rserver bin/dc4c_wserver bin/dc4c_status bin/hzbat \
	lib/libfasterjson.so lib/libdc4c_util.so lib/libdc4c_proto.so lib/libdc4c_api.so lib/libdc4c_tfc_dag.so lib/libhzbat.so \
	etc/hzbat_init.sql etc/hzbat_reset.sql \
	shbin/*sql* shbin/msqlshzbat.sh shbin/dc4c.do shbin/hzbat.sh
if [ $? -ne 0 ] ; then
	rm $TAR_FILENAME
	exit 1
fi

ls -l $TAR_FILENAME


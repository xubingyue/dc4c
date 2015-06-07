cd ..
OS=`uname`
VERSION=`head -1 ChangeLog-CN | awk '{print $2}'`

cd
tar cvzf dc4c-${OS}-bin-${VERSION}.tar.gz \
	bin/dc4c_* \
	lib/libfasterjson.so \
	lib/libdc4c_*.so \
	shbin/dc4c.do \


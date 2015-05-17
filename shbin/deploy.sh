#!/usr/bin/ksh

PROJECT=$1
FILENAME=$2
PATHNAME=${PWD#$HOME/}

if [ ! -f $HOME/etc/$PROJECT.deploy ] ; then
	echo "$HOME/etc/$PROJECT.deploy not found"
	exit 1
fi

cat $HOME/etc/$PROJECT.deploy | grep -v "^$" | grep -v "^#" | while read IP PORT USERNAME PASSWORD ; do
	ftp -inv <<EOF
		open $IP $PORT
		user $USERNAME $PASSWORD
		cd $PATHNAME
		del $FILENAME
		bin
		put $FILENAME
		chmod 755 $FILENAME
		bye
EOF
done


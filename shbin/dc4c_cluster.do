usage()
{
	echo "USAGE : dc4c_cluster.do [ start | stop | restart | kill | status ]"
}

if [ $# -eq 0 ] ; then
	usage
	exit 7 ;
fi

action=$1
shift

function rsh_call()
{
	U=$1
	H=$2
	CMD=$3
	if [ x"$U" != x"" ] && [ x"$H" != x"" ] ; then
		echo "$U@$H $ $CMD"
		rsh -l $U $H ". ~/.bash_profile;$CMD"
	fi
}

case $action in
	start)
		exec 3<$HOME/etc/dc4c_cluster.conf
		while read -u3 LINE ; do
			echo $LINE | grep -E "^$|^#" >/dev/null 2>&1
			if [ $? -eq 0 ] ; then
				continue
			fi
			H=`echo $LINE | awk '{print $1}'`
			U=`echo $LINE | awk '{print $2}'`
			rsh_call "$U" "$H" "dc4c.do start $*"
		done
		exec 3<&-
		;;
	stop)
		exec 3<$HOME/etc/dc4c_cluster.conf
		while read -u3 LINE ; do
			echo $LINE | grep -E "^$|^#" >/dev/null 2>&1
			if [ $? -eq 0 ] ; then
				continue
			fi
			H=`echo $LINE | awk '{print $1}'`
			U=`echo $LINE | awk '{print $2}'`
			rsh_call "$U" "$H" "dc4c.do stop"
		done
		exec 3<&-
		;;
	restart)
		dc4c_cluster.do stop
		sleep 1
		dc4c_cluster.do status
		dc4c_cluster.do start $*
		;;
	kill)
		exec 3<$HOME/etc/dc4c_cluster.conf
		while read -u3 LINE ; do
			echo $LINE | grep -E "^$|^#" >/dev/null 2>&1
			if [ $? -eq 0 ] ; then
				continue
			fi
			H=`echo $LINE | awk '{print $1}'`
			U=`echo $LINE | awk '{print $2}'`
			rsh_call "$U" "$H" "dc4c.do kill"
		done
		exec 3<&-
		;;
	status)
		exec 3<$HOME/etc/dc4c_cluster.conf
		while read -u3 LINE ; do
			echo $LINE | grep -E "^$|^#" >/dev/null 2>&1
			if [ $? -eq 0 ] ; then
				continue
			fi
			H=`echo $LINE | awk '{print $1}'`
			U=`echo $LINE | awk '{print $2}'`
			rsh_call "$U" "$H" "dc4c.do status"
		done
		exec 3<&-
		;;
	*)
		usage
		;;
esac


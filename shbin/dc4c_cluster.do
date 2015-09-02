usage()
{
	echo "USAGE : dc4c_cluster.do [ start | stop | restart | kill | status ]"
}

if [ $# -eq 0 ] ; then
	usage
	exit 7 ;
fi

ACTION=$1
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

case $ACTION in
	start)
		exec 3<$HOME/etc/dc4c_cluster.conf
		while read -u3 LINE ; do
			echo $LINE | grep -E "^$|^#" >/dev/null 2>&1
			if [ $? -eq 0 ] ; then
				continue
			fi
			H=`echo $LINE | awk '{print $1}'`
			U=`echo $LINE | awk '{print $2}'`
			rsh_call "$U" "$H" "dc4c.do start --delay 5 $*"
		done
		exec 3<&-
		;;
	stop)
		tac $HOME/etc/dc4c_cluster.conf > $HOME/etc/dc4c_cluster.conf.tac
		exec 3<$HOME/etc/dc4c_cluster.conf.tac
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
		rm -f $HOME/etc/dc4c_cluster.conf.tac
		;;
	restart)
		dc4c_cluster.do stop
		sleep 5
		dc4c_cluster.do status
		dc4c_cluster.do start $*
		;;
	kill)
		tac $HOME/etc/dc4c_cluster.conf > $HOME/etc/dc4c_cluster.conf.tac
		exec 3<$HOME/etc/dc4c_cluster.conf.tac
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
		rm -f $HOME/etc/dc4c_cluster.conf.tac
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


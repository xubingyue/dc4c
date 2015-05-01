usage()
{
	echo "USAGE : dc4c.do [ start | stop | kill | status ]"
}

if [ $# -eq 0 ] ; then
	usage
	exit 7 ;
fi

function call()
{
	CMD=$1
	echo $CMD
	$CMD
}

case $1 in
	start)
		call "rserver --rserver-ip-port 0.0.0.0:12001"
		sleep 1
		call "wserver --rserver-ip-port 0.0.0.0:12001 --wserver-ip-port 0.0.0.0:13001"
		call "wserver --rserver-ip-port 0.0.0.0:12001 --wserver-ip-port 0.0.0.0:13002"
		call "wserver --rserver-ip-port 0.0.0.0:12001 --wserver-ip-port 0.0.0.0:13003"
		;;
	stop)
		PID=`ps -f -u $USER | grep -v grep | awk '{if($8=="wserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill $PID"
		fi
		sleep 1
		PID=`ps -f -u $USER | grep -v grep | awk '{if($8=="rserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill $PID"
		fi
		;;
	kill)
		PID=`ps -f -u $USER | grep -v grep | awk '{if($8=="wserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill -9 $PID"
		fi
		sleep 1
		PID=`ps -f -u $USER | grep -v grep | awk '{if($8=="rserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill -9 $PID"
		fi
		;;
	status)
		ps -f -u $USER | grep -v grep | awk '{if($8=="rserver")print $0}'
		ps -f -u $USER | grep -v grep | awk '{if($8=="wserver")print $0}'
		;;
	*)
		usage
		;;
esac



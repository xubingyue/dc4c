usage()
{
	echo "USAGE : dc4c_wserver.do [ start | stop | kill | status ]"
}

if [ $# -eq 0 ] ; then
	usage
	exit 7 ;
fi

action=$1
shift

function call()
{
	CMD=$1
	echo $CMD
	$CMD
}

case $action in
	start)
		# call "dc4c_wserver -r 0.0.0.0:12001 -w 0.0.0.0:13001 -c 1 $*"
		# call "dc4c_wserver -r 0.0.0.0:12001,0.0.0.0:12002 -w 0.0.0.0:13001 -c 1 $*"
		call "dc4c_wserver -r 0.0.0.0:12001,0.0.0.0:12002 -w 0.0.0.0:13001 -c 4 $*"
		;;
	stop)
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3=="1"&&$8=="dc4c_wserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill $PID"
		fi
		sleep 1
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3!="1"&&$8=="dc4c_wserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill $PID"
		fi
		;;
	kill)
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3=="1"&&$8=="dc4c_wserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill -9 $PID"
		fi
		sleep 1
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3!="1"&&$8=="dc4c_wserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill -9 $PID"
		fi
		;;
	status)
		ps -f -u $USER | grep -v grep | awk '{if($3=="1"&&$8=="dc4c_wserver")print $0}'
		ps -f -u $USER | grep -v grep | awk '{if($3!="1"&&$8=="dc4c_wserver")print $0}'
		;;
	*)
		usage
		;;
esac



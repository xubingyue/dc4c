usage()
{
	echo "USAGE : dc4c.do [ start | stop | restart | kill | status ]"
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
		call "dc4c_rserver -r 127.0.0.1:12001 $*"
		call "dc4c_rserver -r 127.0.0.1:12002 $*"
		call "dc4c_wserver -r 127.0.0.1:12001,127.0.0.1:12002 -w 127.0.0.1:13001 -c 5 $*"
		;;
	stop)
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3=="1"&&$8=="dc4c_wserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill $PID"
		fi
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3=="1"&&$8=="dc4c_rserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill $PID"
		fi
		sleep 1
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3!="1"&&$8=="dc4c_wserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill $PID"
		fi
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3!="1"&&$8=="dc4c_rserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill $PID"
		fi
		;;
	restart)
		dc4c.do stop
		sleep 1
		dc4c.do status
		dc4c.do start $*
		;;
	kill)
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3=="1"&&$8=="dc4c_wserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill -9 $PID"
		fi
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3=="1"&&$8=="dc4c_rserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill -9 $PID"
		fi
		sleep 1
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3!="1"&&$8=="dc4c_wserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill -9 $PID"
		fi
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3!="1"&&$8=="dc4c_rserver")print $2}'`
		if [ x"$PID" != x"" ] ; then
			call "kill -9 $PID"
		fi
		;;
	status)
		ps -f -u $USER | grep -v grep | awk '{if($3=="1"&&$8=="dc4c_rserver")print $0}'
		ps -f -u $USER | grep -v grep | awk '{if($3!="1"&&$8=="dc4c_rserver")print $0}'
		ps -f -u $USER | grep -v grep | awk '{if($3=="1"&&$8=="dc4c_wserver")print $0}'
		ps -f -u $USER | grep -v grep | awk '{if($3!="1"&&$8=="dc4c_wserver")print $0}'
		;;
	*)
		usage
		;;
esac


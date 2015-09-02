usage()
{
	echo "USAGE : dc4c.do [ start | stop | restart | kill | status ]"
}

if [ $# -eq 0 ] ; then
	usage
	exit 7 ;
fi

ACTION=$1
shift

function call()
{
	CMD=$1
	echo $CMD
	$CMD
}

case $ACTION in
	start)
		PID=`ps -f -u $USER | grep -v grep | awk '{if($3=="1"&&$8=="dc4c_wserver")print $2}'`
		PID2=`ps -f -u $USER | grep -v grep | awk '{if($3=="1"&&$8=="dc4c_rserver")print $2}'`
		if [ x"$PID" != x"" ] || [ x"$PID2" != x"" ] ; then
			exit 0
		fi
		call "dc4c_rserver -r 192.68.74.231:16001 $*"
		call "dc4c_rserver -r 192.68.74.231:16002 $*"
		call "dc4c_wserver -r 192.68.74.231:16001,192.68.74.231:16002 -w 192.68.74.231:17001 -c 10 $*"
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


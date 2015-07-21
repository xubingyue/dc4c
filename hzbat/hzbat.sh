if [ $# -eq 1 ] ; then
	SCHEUDLE_NAME=$1
elif [ $# -eq 2 ] ; then
	SCHEUDLE_NAME=$1
	BATCH_NAME=$2
else
	echo "USAGE : hzbat.sh (SCHDULE_NAME) [ (BATCH_NAME) ]"
	exit 7 
fi

DATE=`sql "SELECT DATE_FORMAT(business_date,'%Y-%m-%d') FROM pmp_spma_para WHERE system_id='hzbank'"|head -4|tail -1|awk '{print $2}'`
HZBAT_STDOUT_DATE_LOG=$HOME/log/hzbat_stdout_${DATE}.log
> $HZBAT_STDOUT_DATE_LOG

date >>$HZBAT_STDOUT_DATE_LOG

TIME=`date +%H`
SENDSM_FLAG=1
while [ $TIME -lt 6 ] ; do
	FLAG1=`sql "select count(*) from ftp_rar_para where run_flag<>'3' and busi_date=( SELECT business_date FROM pmp_spma_para WHERE system_id='hzbank')"|head -4|tail -1|awk '{print $2}'`
	FLAG2=`sql "select count(*) from ftp_rar_para where run_flag='3' and busi_date=( SELECT business_date FROM pmp_spma_para WHERE system_id='hzbank')"|head -4|tail -1|awk '{print $2}'`
	echo "$FLAG1 $FLAG2" >>$HZBAT_STDOUT_DATE_LOG
	if [ $FLAG1 -eq 0 ] && [ $FLAG2 -gt 0 ] ; then
		SENDSM_FLAG=0
		break;
	fi
	sleep 600
	TIME=`date +%H`
done
if [ $SENDSM_FLAG -eq 1 ] ; then
	echo "强制执行积分批量" >>$HZBAT_STDOUT_DATE_LOG
fi

date >>$HZBAT_STDOUT_DATE_LOG

function gogogo
{
	echo "BUSIDATE[$DATE]"
	if [ x"$BATCH_NAME" = x"" ] ; then
		time hzbat "$DATE" $SCHEUDLE_NAME
	else
		time hzbat "$DATE" $SCHEUDLE_NAME $BATCH_NAME
	fi
	echo $?
}

gogogo | tee -a $HZBAT_STDOUT_DATE_LOG

date >>$HZBAT_STDOUT_DATE_LOG


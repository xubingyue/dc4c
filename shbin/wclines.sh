cd ..

CLINES=`find . -name "*.c" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{printf s}'`
printf "*.c files\t[$CLINES]lines\n"

HLINES=`find . -name "*.h" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{printf s}'`
printf "*.h files\t[$HLINES]lines\n"

IDL_CLINES=`find . -name "IDL_*.c" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{printf s}'`
printf "IDL_*.c files\t[$IDL_CLINES]lines DISCARD\n"

IDL_HLINES=`find . -name "IDL_*.h" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{printf s}'`
printf "IDL_*.c files\t[$IDL_CLINES]lines DISCARD\n"

ECLINES=`find . -name "*.ec" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{printf s}'`
printf "*.ec files\t[$ECLINES]lines\n"

IDL_ECLINES=`find . -name "IDL_*.ec" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{printf s}'`
printf "IDL_*.ec files\t[$IDL_ECLINES]lines DISCARD\n"

LINES=`echo "$CLINES + $HLINES - $IDL_CLINES - $IDL_HLINES + $ECLINES - $IDL_ECLINES" | bc`
echo "----------------------------------"
printf "TOTAL\t\t[$LINES]lines\n"


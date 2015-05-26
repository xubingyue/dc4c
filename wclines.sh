CLINES=`find . -name "*.c" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{print s}'`
HLINES=`find . -name "*.h" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{print s}'`
IDLCLINES=`find . -name "IDL_*.c" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{print s}'`
IDLHLINES=`find . -name "IDL_*.h" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{print s}'`
ECLINES=`find . -name "*.ec" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{print s}'`
IDLECLINES=`find . -name "IDL_*.ec" -exec wc -l {} \; | awk 'BEGIN{s=0}{s+=$1}END{print s}'`

LINES=`echo "$CLINES + $HLINES - $IDLCLINES - $IDLHLINES + $ECLINES - $IDLECLINES" | bc`
echo $LINES


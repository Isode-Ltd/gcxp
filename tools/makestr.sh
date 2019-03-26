#!/bin/sh
BASENAME=`basename $1 .xsl`
HEADER="const std::string $BASENAME ="
TRAILER=";"

rm -f $2
echo ${HEADER} > $2
sed -e 's/$//g' -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/^/"/' -e 's/$/\\n"/' < $1 >> $2
echo ${TRAILER} >> $2

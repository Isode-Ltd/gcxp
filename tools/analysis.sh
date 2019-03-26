#!/bin/sh
set -e
SB="scan-build" export SB
GIT_STATUS_FLAGS="-s" export GIT_STATUS_FLAGS
CCACHE_CPP2="yes" export CCACHE_CPP2
CCACHE_DISABLE="yes" export CCACHE_DISABLE
HTML="../analysis_html"

MAKE=ninja export MAKE

if [ ! -e include/guard/exception.h ] ; then
    echo "analysis: Must be run from Guard src directory"
    exit 1
fi

if [ "$bamboo_shortPlanName" ] ; then
    echo "analysis: Running Bamboo $bammbo_shortPlanName $PARAMAKE"
    SB="${bamboo_capability_system_builder_command_scanbuild}" export SB
else
    echo "analysis: Running in local checkout"
fi

echo "analysis: make clean..."
rm -f CMakeCache.txt
cmake -G Ninja
${MAKE} clean

if [ -d "${HTML}" ] ; then
    rm -rf ${HTML}
fi

echo "analysis: scan-build make all ..."
${SB} -o ${HTML} --use-c++=c++ -maxloop 8 -stats --keep-empty --html-title "M-Guard" \
    ${MAKE} ${PARAMAKE} all

echo "analysis: completed successful."

if [ "$bamboo_shortPlanName" ] ; then
    echo "analysis: Renaming output directory to ${HTML}/analysis"
    mv ${HTML}/* ${HTML}/analysis
fi

exit 0

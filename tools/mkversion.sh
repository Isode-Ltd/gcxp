#!/bin/sh
if [ $# != 1 ] ; then
    echo "mkversion: must have exactly one argument"
    exit 1
fi
VFILE="$1"

if [ ! -e exception.h ] ; then
    echo "mkversion: Must be run from Guard include/guard directory"
    exit 1
fi

if [ ! -e $vfile ] ; then
    echo "mkversion: input file doesn't exist"
    exit 1
fi

VERSION=`cat $VFILE`
sed -e "s;@VERSION@;${VERSION};"

#!/bin/sh
VFILE="../version.txt"

if [ ! -e include/guard/exception.h ] ; then
    echo "describe: Must be run from Guard src directory"
    exit 1
fi

VERSION="`git describe --first-parent --long --dirty --match 'guard_*' --always 2>/dev/null`"
if [ ! "${VERSION}" ] ; then
    VERSION="`git name-rev --name-only HEAD`"
fi

if [ -e ${VFILE} ] ; then
    OLDVERSION="`cat ${VFILE}`"
    if [ "$OLDVERSION" = "$VERSION" ] ; then
        echo "Version unchanged: $VERSION"
        exit 0
    fi
fi

echo "${VERSION}" > ${VFILE}
echo "New version: ${VERSION}"
exit 0

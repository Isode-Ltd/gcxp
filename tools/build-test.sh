#!/bin/sh
set -e
GIT_STATUS_FLAGS="-s" export GIT_STATUS_FLAGS
CCACHE_CPP2="yes" export CCACHE_CPP2

MAKE=ninja
export NINJA
CMAKE=cmake
export CMAKE

if [ ! -e include/guard/exception.h ] ; then
    echo "build-test: Must be run from Guard src directory"
    exit 1
fi

if [ "$bamboo_shortPlanName" ] ; then
    echo "build-test: Running Bamboo $bamboo_shortPlanName $PARAMAKE"
    # CCACHE_CC="${bamboo_capability_system_builder_command_cxx}" export CCACHE_CC
    : ccache -z
    # MAKE="${bamboo_capability_system_builder_command_ninja}" export MAKE
    if [ -d /opt/local/libexec/ccache ] ; then
        PATH=/opt/local/libexec/ccache:$PATH export PATH
    else
        PATH=/usr/local/libexec/ccache:$PATH export PATH
    fi
else
    echo "build-test: Running in local checkout"
fi

rm -f CMakeCache.txt

if [ "`git status --porcelain`" ] ; then
    git status ${GIT_STATUS_FLAGS}
    echo "build-test: repository not clean at start!"
    if [ ! "$bamboo_shortPlanName" ] ; then
        exit 1
    fi
fi

echo "build-test: cmake..."
cmake -G Ninja -DCMAKE_CXX_FLAGS="-Werror"
echo "build-test: make clean..."
${MAKE} ${PARAMAKE} clean
echo "build-test: make..."
${MAKE} ${PARAMAKE}

if [ "`git status --porcelain`" ] ; then
    git status ${GIT_STATUS_FLAGS}
    echo "build-test: repository not clean after make all!"
    if [ ! "$bamboo_shortPlanName" ] ; then
        exit 1
    fi
fi

echo "Check for improper comment use..."
if [ `cat */*.cpp */*.h */*/*.h | egrep -c '/// |/\*\*|\*\*/'` -gt 0 ]; then
    egrep '///|/\*\*|\*\*/' */*.cpp */*.h */*/*.h
    exit 1
fi

echo "Check for improper NULL use..."
if [ `cat */*.cpp */*.h */*/*.h | egrep -c 'NULL'` -gt 0 ]; then
    egrep 'NULL' */*.cpp */*.h */*/*.h
    exit 1
fi

echo "Check for improper typedef use..."
if [ `cat */*.cpp */*.h */*/*.h | egrep -c 'typedef'` -gt 0 ]; then
    egrep 'typdef' */*.cpp */*.h */*/*.h
    exit 1
fi

echo "Check for improper preprocessor use..."
if [ `cat */*.cpp */*.h */*/*.h | grep -v BOOST_TEST_MAIN | egrep -c '^#[ 	]*(define|undef)'` -gt 0 ] ; then
    egrep '^#[ 	]*(define|undef)' --exclude unit.cpp */*.cpp */*.h */*/*.h
    exit 1
fi

echo "Make reformat..."
${MAKE} ${PARAMAKE} reformat

echo "build-test: Running unit tests"
${MAKE} junit-tests

if [ "$bamboo_shortPlanName" ] ; then
    : ccache -s
fi

if [ "`git status --porcelain`" ] ; then
    git status ${GIT_STATUS_FLAGS}
    echo "build-test: repository not clean at end!"
    exit 1
fi

echo "build-test: completed successful."
exit 0

#!/bin/sh
set -e
GIT_STATUS_FLAGS="-s" export GIT_STATUS_FLAGS
CCACHE_CPP2="yes" export CCACHE_CPP2
CCACHE_DISABLE="yes" export CCACHE_DISABLE
LCOV_OUT="libguard.info"
LCOV_HTML="../lcov_html"
#GCOV_TOOL="--gcov-tool gcov"
GCOV_TOOL="--gcov-tool llvm-cov-wrapper"

MAKE=ninja export MAKE
CMAKE=cmake export CMAKE

if [ ! -e include/guard/exception.h ] ; then
    echo "coverage: Must be run from Guard src directory"
    exit 1
fi

if [ "$bamboo_shortPlanName" ] ; then
    echo "coverage: Running Bamboo $bammbo_shortPlanName $PARAMAKE"
else
    echo "coverage: Running in local checkout"

    echo "coverage: make clean-coverage..."
    ${MAKE} clean-coverage
fi

rm -f CMakeCache.txt
rm -f ${LCOV_OUT}
if [ -d "${LCOV_HTML}" ] ; then
    rm -rf ${LCOV_HTML}
fi

echo "coverage: cmake..."
cmake -G Ninja -DCMAKE_CXX_FLAGS="-coverage" -DCMAKE_EXE_LINKER_FLAGS="-coverage"

echo "coverage: make all (-coverage)..."
${MAKE} ${PARAMAKE}

lcov ${GCOV_TOOL} --directory . --zerocounters -q

echo "coverage: make unit-test (junit output)..."
${MAKE} junit-tests

echo "coverage: running lcov -capture..."
lcov ${GCOV_TOOL} --no-external --capture --directory . --output-file ${LCOV_OUT} 

echo "coverage: running genhtml..."
genhtml -o ${LCOV_HTML} -t "M-Guard Test Coverage" --num-spaces 4 --demangle-cpp ${LCOV_OUT}

rm -f ${LCOV_OUT}

if [ ! "$bamboo_shortPlanName" ] ; then
    echo "coverage: make clean-coverage..."
    ${MAKE} clean-coverage
fi

echo "coverage: completed successful."
exit 0

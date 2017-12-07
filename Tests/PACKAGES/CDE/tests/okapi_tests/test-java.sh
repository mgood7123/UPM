#!/bin/sh

source okapi_test_common.sh
okapi_test_init # call init function

$OKAPI_BIN /usr/bin/java "" $TESTDIR

pushd $TESTDIR > /dev/null
find . | xargs file | sort > contents.txt
popd > /dev/null

diff -u $TESTDIR/contents.txt $TESTNAME.golden

#!/bin/sh

source okapi_test_common.sh
okapi_test_init # call init function

$OKAPI_BIN /CDE/strace-4.6/cde.c /home/pgbovine $TESTDIR
$OKAPI_BIN /CDE/scripts/coalesce/bsdiff-4.3.tar.gz /home/pgbovine $TESTDIR

pushd $TESTDIR > /dev/null
find . | xargs file | sort > contents.txt
popd > /dev/null

diff -u $TESTDIR/contents.txt $TESTNAME.golden

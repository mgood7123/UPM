#!/bin/sh

source okapi_test_common.sh
okapi_test_init # call init function

CLONE1_DIR=$TESTDIR/java-clone-1/
CLONE2_DIR=$TESTDIR/java-clone-2/

mkdir $CLONE1_DIR
mkdir $CLONE2_DIR

$OKAPI_BIN /usr/bin/java "" $CLONE1_DIR
$OKAPI_BIN /usr/bin/java $CLONE1_DIR $CLONE2_DIR

pushd $CLONE1_DIR > /dev/null
find . | xargs file | sort > contents.txt
popd > /dev/null

pushd $CLONE2_DIR > /dev/null
find . | xargs file | sort > contents.txt
popd > /dev/null

diff -u $CLONE1_DIR/contents.txt test-java.golden
diff -u $CLONE2_DIR/contents.txt test-java.golden

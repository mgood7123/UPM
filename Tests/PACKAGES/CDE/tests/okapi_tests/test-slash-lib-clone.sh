#!/bin/sh

source okapi_test_common.sh
okapi_test_init # call init function

rm -f $TESTNAME.err.out

CLONE1_DIR=$TESTDIR/slashlib-clone-1/
CLONE2_DIR=$TESTDIR/slashlib-clone-2/

mkdir $CLONE1_DIR
mkdir $CLONE2_DIR

# ok, we're gonna try to copy ALL of /lib into $TESTDIR, which should be fun!
for f in `find /lib`
do
  $OKAPI_BIN $f "" $CLONE1_DIR 2>> $TESTNAME.err.out
done

# also copy the entirety of /usr/src/kernels since something in lib references it
for f in `find /usr/src/kernels`
do
  $OKAPI_BIN $f "" $CLONE1_DIR 2>> $TESTNAME.err.out
done


# now clone from $CLONE1_DIR to $CLONE2_DIR
for f in `find /lib`
do
  $OKAPI_BIN $f $CLONE1_DIR $CLONE2_DIR 2>> $TESTNAME.err.out
done

for f in `find /usr/src/kernels`
do
  $OKAPI_BIN $f $CLONE1_DIR $CLONE2_DIR 2>> $TESTNAME.err.out
done


pushd $CLONE1_DIR > /dev/null
find . | xargs file | sort > contents.txt
popd > /dev/null

pushd $CLONE2_DIR > /dev/null
find . | xargs file | sort > contents.txt
popd > /dev/null


# do recursive directory diffs, silencing warnings
diff -ur /lib/ $CLONE1_DIR/lib/ 2> /dev/null
diff -ur /usr/src/kernels/ $CLONE1_DIR/usr/src/kernels/ 2> /dev/null

diff -u $CLONE1_DIR/contents.txt $TESTNAME.golden
diff -u $CLONE2_DIR/contents.txt $TESTNAME.golden

diff -u $TESTNAME.err.out $TESTNAME.err.golden

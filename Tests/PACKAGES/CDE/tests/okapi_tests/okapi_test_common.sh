# DO NOT execute directly, only import from other scripts

OKAPI_BIN="/home/pgbovine/CDE/okapi"
OKAPI_TESTDIR="/tmp/okapi-testdir"

# initialize these based on the name of the script that imports this file.
# e.g., if the script name is ./test-java.sh, then TESTNAME=test-java
TESTNAME=`basename ${0/%.sh/}`
TESTDIR="$OKAPI_TESTDIR/$TESTNAME"


function okapi_test_init {
  rm -rf $OKAPI_TESTDIR;
  mkdir $OKAPI_TESTDIR;
  mkdir $TESTDIR;
}


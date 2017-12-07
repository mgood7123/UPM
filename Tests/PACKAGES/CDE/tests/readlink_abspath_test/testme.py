import sys
sys.path.insert(0, '..')
from cde_test_common import *

def checker_func():
  assert os.path.isfile('cde-package/cde-root/home/pgbovine/CDE/tests/readlink_abspath_test/libc.so.6')

generic_test_runner(["python", "readlink_abspath_test.py"], checker_func)

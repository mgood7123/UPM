import os, sys
sys.path.insert(0, '..')
from cde_test_common import *

def checker_func():
  assert os.path.isfile(CDE_ROOT_DIR + '/usr/bin/java')
  assert os.path.islink(CDE_ROOT_DIR + '/usr/bin/java')
  assert os.path.isfile(CDE_ROOT_DIR + '/usr/lib/jvm/java-1.6.0-openjdk-1.6.0.0/jre/bin/java')

generic_test_runner(["java", "HelloWorld"], checker_func)

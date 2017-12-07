import sys
sys.path.insert(0, '..')
from cde_test_common import *

def checker_func():
  pass

# do NOT clear cde.options file
generic_test_runner(["python", "proc_maps_test.py"], checker_func, clear_cde_options=False)

# Find all ELF executables (NOT shared libraries) within the package root
# directory and create wrappers for all of them using create_ELF_wrapper.py
#
# Inputs: argv[1] - base directory of package root directory
#         argv[2] - absolute path to the dynamic linker to use for the wrapper
#                   (e.g., '/lib/ld-linux-x86-64.so.2')
#         argv[3] - colon-separated list of directories where shared libraries
#                   should be found (e.g., '/lib:/usr/lib')
#
# Pre-reqs: the 'file' utility

import os, sys
from cde_script_utils import *
from create_ELF_wrapper import create_ELF_wrapper

PACKAGE_ROOT_DIR = sys.argv[1]
assert os.path.isdir(PACKAGE_ROOT_DIR)

LD_LINUX_PATH = sys.argv[2]
assert os.path.isfile(LD_LINUX_PATH)
assert LD_LINUX_PATH[0] == '/' # absolute path
assert os.path.isfile(PACKAGE_ROOT_DIR + LD_LINUX_PATH) # make sure it exists within the package

LD_LIBRARY_PATH_DIRS = sys.argv[3].split(':')

# make sure these are all absolute paths that exist within the package
for p in LD_LIBRARY_PATH_DIRS:
  assert p.startswith('/')
  assert os.path.isdir(PACKAGE_ROOT_DIR + p)



for (d, subdirs, files) in os.walk(PACKAGE_ROOT_DIR):
  for f in files:
    p = os.path.join(d, f)
    # only wrap dynamically-linked ELF executables (NOT shared libraries)
    # and do NOT wrap files that end in ".original", since those have already
    # been wrapped ... in other words, don't double-wrap!
    if is_dynamic_ELF_exe(p) and not os.path.splitext(p)[-1] == '.original':
      print "Creating wrapper for", p
      create_ELF_wrapper(p, PACKAGE_ROOT_DIR, LD_LINUX_PATH, LD_LIBRARY_PATH_DIRS)

# Given a package root directory, argv[1], find all shared libraries (*.so*)
# referenced by constant strings within all ELF binaries within the package, and
# copy all of those shared libraries (and their transitive dependencies) into the
# package.
#
# Pre-reqs: okapi (compile with "cd .. && make okapi"), file, strings, locate
# ---
#
# Implementation:
#
# Use 'file' to find all ELF binaries within the package, then use 'strings' to
# grep through all ELF binaries looking for "[.]so" patterns that are indicative
# of shared libraries, then use 'locate' to find those shared libraries on the
# system, then use 'okapi' to copy those libraries into the package root
# directory.  Repeat until the set of ELF binaries within the package converges.
#
# Note that this script is OVERLY CONSERVATIVE and might grab far more libraries
# than you actually need, since 'locate' finds ALL versions of libraries
# matchine the given base filename.
#
# by Philip Guo

import os, sys
from cde_script_utils import *


script_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
OKAPI_BIN = os.path.normpath(os.path.join(script_dir, "../okapi"))
if not os.path.isfile(OKAPI_BIN):
  print "Error: %s does not exist.\nPlease run 'make okapi' from the top-level CDE/ directory." % OKAPI_BIN
  sys.exit(1)

PACKAGE_ROOT_DIR = sys.argv[1]
assert os.path.isdir(PACKAGE_ROOT_DIR)


# optimization to prevent unnecessary calls to 'locate', which are SLOW
already_seen_set = set()

i = 1
while True:
  print "Iteration:", i

  ELF_files_in_pkg = set()

  for (d, subdirs, files) in os.walk(PACKAGE_ROOT_DIR):
    for f in files:
      p = os.path.join(d, f)
      # file $p | grep "ELF "
      # (note that this picks up both executables AND shared libraries!)
      (file_cmd_stdout, _) = run_cmd(['file', p])
      if "ELF " in file_cmd_stdout:
        ELF_files_in_pkg.add(p)


  possible_libs_set = set()

  for f in ELF_files_in_pkg:
    # strings $f | grep "[.]so"
    (strings_cmd_stdout, _) = run_cmd(['strings', f])
    for s in strings_cmd_stdout.splitlines():
      if ".so" in s:
        possible_libs_set.add(s)


  libfiles_to_copy = set()

  for possible_lib in possible_libs_set:
    # optimization
    if possible_lib in already_seen_set:
      #print "Already seen:", possible_lib
      continue

    already_seen_set.add(possible_lib)
    # if it's an absolute path, use it as-is:
    if possible_lib[0] == '/':
      if os.path.isfile(possible_lib):
        libfiles_to_copy.add(possible_lib)
    # otherwise run 'locate' to find the library
    else:
      (locate_cmd_stdout, _) = run_cmd(['locate', possible_lib])
      for libfile in locate_cmd_stdout.splitlines():
        # only find EXACT basename matches with possible_lib
        if os.path.isfile(libfile) and os.path.basename(libfile) == possible_lib:
          libfiles_to_copy.add(libfile)

  files_to_remove = set()
  # check to see what's already in PACKAGE_ROOT_DIR:
  for f in libfiles_to_copy:
    assert f[0] == '/' # abspath!
    file_in_package = PACKAGE_ROOT_DIR + '/' + f
    if os.path.exists(file_in_package):
      files_to_remove.add(f)

  libfiles_to_copy -= files_to_remove

  for f in libfiles_to_copy:
    print "  okapi-ing", f, "into", PACKAGE_ROOT_DIR
    (okapi_stdout, okapi_stderr) = run_cmd([OKAPI_BIN, f, '', PACKAGE_ROOT_DIR])
    err = okapi_stderr.strip()
    if err:
      print err

  # exit condition
  if len(libfiles_to_copy) == 0:
    break

  i += 1


print "Done okapi-ing all shared libraries into %s" % PACKAGE_ROOT_DIR

# Creates a wrapper script for a specified program, which:
#   1.) Explicitly invokes the dynamic linker (ld-linux*) within the package
#       that the program requires (specified by the LD_LINUX_PATH constant)
#   2.) Sets the ld-linux --library-path parameter to refer to versions of
#       libraries within the package
#
# Renames the original program by appending '.original' to its filename, and
# then substitutes the wrapper for the original program.
#
# Inputs: argv[1] - executable file to wrap   (origfile)
#         argv[2] - base directory of package root directory (package_basedir)
#         argv[3] - absolute path to the dynamic linker to use
#                   (e.g., '/lib/ld-linux-x86-64.so.2')
#         argv[4] - colon-separated list of directories where shared libraries
#                   should be found (e.g., '/lib:/usr/lib')
#
# Pre-reqs: A package has already been created in package_basedir, which
#           contains all necessary files.  Also, the 'file' utility must exist.
#
# This script emulates the behavior of cde-exec and allows some programs to
# execute natively without the limitations of cde-exec (e.g., minor slowdowns,
# ptrace limitations).  However, this wrapper script approach has some limitations
# of its own, such as:
#
# 1.) Since the original executable binary has been renamed with a .original
#     suffix, if a program tries to access its own argv[0] or other programs try
#     to access its name by inspecting, say, /proc/, then the returned name will
#     be different than its original name. This discrepancy might be a problem
#     for applications that dispatch on specific program names.
#
# 2.) Complications arise when your programs hard-code absolute paths to, say
#     /bin or /lib (or other system directories). When your package is
#     transported to another machine, your programs will attempt to access the
#     files in the other machine's /bin or /lib directories, respectively,
#     rather than the versions within the package. Some programs use hard-coded
#     absolute paths by default, but those paths can be altered with the proper
#     command-line options, so they have some hope of working within the package.


import os, sys
from cde_script_utils import *


def create_ELF_wrapper(origfile, package_basedir, ld_linux_path, ld_library_path_dirs_lst):
  dn = os.path.dirname(origfile)

  # strip off trailing '/' for more reliable string comparisons
  if package_basedir[-1] == '/':
    package_basedir = package_basedir[:-1]
  assert package_basedir[-1] != '/'

  # ok, we need to check that dn is within a sub-directory of
  # package_basedir, and figure out how many levels of '../'
  # are required to get from dn to package_basedir
  assert dn.startswith(package_basedir) # very crude sub-directory test

  levels = 0
  tmp = dn
  while tmp:
    tmp = os.path.dirname(tmp)
    levels += 1
    if tmp == package_basedir:
      break


  ld_library_path_str = ':'.join(["$HERE" + ('/..' * levels) + p for p in ld_library_path_dirs_lst])

  if not is_dynamic_ELF_exe(origfile):
    print >> sys.stderr, "Skipping", origfile, "because it doesn't appear to be a dynamically-linked ELF executable"
    sys.exit(-1)


  # mv origfile renamed_file
  renamed_file = origfile + '.original'
  os.rename(origfile, renamed_file)
  renamed_file_basename = os.path.basename(renamed_file)

  # create wrapper script
  wrapper = open(origfile, 'w')
  print >> wrapper, "#!/bin/sh"
  print >> wrapper, 'HERE="$(dirname "$(readlink -f "${0}")")"'
  print >> wrapper, '"$HERE' + ('/..' * levels) + ld_linux_path + '" --library-path "' +  ld_library_path_str + '"' + ' "$HERE/' + renamed_file_basename + '" "$@"'
  wrapper.close()

  # chmod both original and wrapper to "-rwxr-xr-x"
  os.chmod(origfile, 0755)
  os.chmod(renamed_file, 0755)



if __name__ == "__main__":
  origfile = sys.argv[1]
  assert os.path.isfile(origfile), origfile

  package_basedir = sys.argv[2]
  assert os.path.isdir(package_basedir)

  LD_LINUX_PATH = sys.argv[3]
  assert os.path.isfile(LD_LINUX_PATH)
  assert LD_LINUX_PATH[0] == '/' # absolute path
  assert os.path.isfile(package_basedir + LD_LINUX_PATH) # make sure it exists within the package

  LD_LIBRARY_PATH_DIRS = sys.argv[4].split(':')

  # make sure these are all absolute paths that exist within the package
  for p in LD_LIBRARY_PATH_DIRS:
    assert p.startswith('/')
    assert os.path.isdir(package_basedir + p)


  create_ELF_wrapper(origfile, package_basedir, LD_LINUX_PATH, LD_LIBRARY_PATH_DIRS)

# Deep copy an entire directory argv[1] into another directory argv[2]
# ---
#
# Use okapi to copy over all sub-directories and symlinks, and to make
# sure that all symlinks are properly munged to point to relative paths
# within the package.  (Note that rsync does NOT munge symlinks or
# faithfully re-create the original directory structure in the presence
# of symlinks to directories.)
#
# by Philip Guo

'''
A brief attempt to explain how okapi_dir.py should be used:

argv[1] should be an ABSOLUTE PATH to a directory on your system
argv[2] can be an absolute or relative path to a directory

okapi_dir.py copies the entire directory tree in argv[1] into argv[2],
preserving all sub-directory and symlink structure.

For example, let's say you run:

  mkdir /tmp/A/

Then populate /tmp/A/ with some contents so that it looks like this:

  /tmp/A
  /tmp/A/A-subdir
  /tmp/A/A-subdir/one.txt
  /tmp/A/A-subdir/two.txt
  /tmp/A/A-subdir/A-subsubdir
  /tmp/A/A-subdir/A-subsubdir/three.txt

Now you run:

  mkdir B/

In order to copy the entirety of /tmp/A into B/, you run:

  python CDE/scripts/okapi_dir.py /tmp/A/ B/

and now the contents of B will look like:

  B/tmp/A
  B/tmp/A/A-subdir
  B/tmp/A/A-subdir/one.txt
  B/tmp/A/A-subdir/two.txt
  B/tmp/A/A-subdir/A-subsubdir
  B/tmp/A/A-subdir/A-subsubdir/three.txt

'''

import os, sys, subprocess

def run_cmd_print_stderr(args):
  (cmd_stdout, cmd_stderr) = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
  cmd_stderr = cmd_stderr.strip()
  if cmd_stderr:
    print cmd_stderr


script_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
OKAPI_BIN = os.path.normpath(os.path.join(script_dir, "../okapi"))
assert os.path.isfile(OKAPI_BIN)


def okapi_dir(basedir, dst_root_dir):
  assert os.path.isdir(basedir)

  for (d, subdirs, files) in os.walk(basedir):
    # first copy over the directory so that it exists even if it's empty:
    run_cmd_print_stderr([OKAPI_BIN, d, '', dst_root_dir])

    # now copy over all the files
    for f in files:
      p = os.path.join(d, f)
      run_cmd_print_stderr([OKAPI_BIN, p, '', dst_root_dir])

    # if any subdirs are symlinks, then copy them over as well to
    # preserve the original directory/symlink structure:
    for sd in subdirs:
      p = os.path.join(d, sd)
      if os.path.islink(p):
        run_cmd_print_stderr([OKAPI_BIN, p, '', dst_root_dir])
        # follow the symlink
        dir_symlink_target = os.path.realpath(p)

        # only recurse if dir_symlink_target is OUTSIDE of basedir
        # to (hopefully) prevent infinite loops
        base_realpath = os.path.realpath(basedir)
        if not dir_symlink_target.startswith(base_realpath):
          okapi_dir(dir_symlink_target, dst_root_dir)


if __name__ == "__main__":
  dst = sys.argv[2]
  assert os.path.isdir(dst)
  okapi_dir(sys.argv[1], dst)


'''
Setup: libc.so.6 in this directory is a symlink to the following absolute path:
  /lib/libc.so.6

Test to make sure that calling readlink('libc.so.6') returns '/lib/libc.so.6'

---

Note that this requires some special handling in CDE's readlink handler,
since the version of the symlink within the package actually refers to
the following RELATIVE path:

  ./../../../../..//lib/libc.so.6

This is because the libc.so.6 file is actually located in:

  cde-package/cde-root/home/pgbovine/CDE/tests/readlink_abspath_test/libc.so.6

within the package, so in order to reference the version of "/lib/libc.so.6"
WITHIN THE PACKAGE, the symlink is actually a relative link with the following
ugly prefix: ./../../../../../

'''

import os

assert os.path.islink('libc.so.6')
print os.readlink('libc.so.6')

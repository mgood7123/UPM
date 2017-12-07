// test using absolute paths, so cde-exec should redirect all of these
// calls within the package

#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <utime.h>

#define PWD "/home/pgbovine/CDE/tests/at_syscalls_test/"


// extracted from fcntl.h

# define AT_FDCWD		-100	/* Special value used to indicate
					   the *at functions should use the
					   current working directory. */
# define AT_SYMLINK_NOFOLLOW	0x100	/* Do not follow symbolic links.  */
# define AT_REMOVEDIR		0x200	/* Remove directory instead of
					   unlinking file.  */
# define AT_SYMLINK_FOLLOW	0x400	/* Follow symbolic links.  */
# define AT_EACCESS		0x200	/* Test access permitted for
					   effective IDs, not real IDs.  */


// testing strategy:
//
// first:
// - use absolute paths to make *at syscalls
// - use relative paths to do assertion checks
//
//
// then flip it around and use relative paths to make syscalls and
// absolute paths to do assertion checks (TODO: implement this!)

int main() {
  struct stat st;
  struct stat st2;

  openat(AT_FDCWD, PWD "openat.txt", O_CREAT | O_WRONLY, 0644);
  assert(stat("openat.txt", &st) == 0); // relative path

  assert(faccessat(AT_FDCWD, PWD "openat.txt", F_OK, 0) == 0);
  assert(fstatat(AT_FDCWD, PWD "openat.txt", &st2, 0) == 0);
  assert(fchmodat(AT_FDCWD, PWD "openat.txt", 0777, 0) == 0);

  struct timeval my_times[2];
  my_times[0].tv_sec = 0;
  my_times[0].tv_usec = 0;
  my_times[1].tv_sec = 0;
  my_times[1].tv_usec = 0;
  assert(futimesat(AT_FDCWD, PWD "openat.txt", my_times, 0) == 0);

  // see /etc/passwd, user 'pgbovine' is 508:100
  assert(fchownat(AT_FDCWD, PWD "openat.txt", 508, 100, 0) == 0);

  assert(linkat(AT_FDCWD, PWD "openat.txt", AT_FDCWD, PWD "openat_hardlink.txt", 0) == 0);
  assert(stat("openat_hardlink.txt", &st) == 0); // relative path

  assert(symlinkat(PWD "openat.txt", AT_FDCWD, PWD "openat_symlink.txt") == 0);
  assert(lstat("openat_symlink.txt", &st) == 0); // relative path

  char res[300];
  assert(readlinkat(AT_FDCWD, PWD "openat_symlink.txt", res, sizeof(res)) > 0);

  assert(renameat(AT_FDCWD, PWD "openat.txt", AT_FDCWD, PWD "openat_newname.txt", 0) == 0);
  assert(stat("openat.txt", &st) != 0); // should not exist anymore
  assert(stat("openat_newname.txt", &st) == 0); // relative path

  unlinkat(AT_FDCWD, PWD "openat_newname.txt", 0);
  unlinkat(AT_FDCWD, PWD "openat_hardlink.txt", 0);
  unlinkat(AT_FDCWD, PWD "openat_symlink.txt", 0);


  mknodat(AT_FDCWD, PWD "mknodat.fifo", S_IFIFO);
  assert(stat("mknodat.fifo", &st) == 0); // relative path
  unlinkat(AT_FDCWD, PWD "mknodat.fifo", 0);

  mkdirat(AT_FDCWD, PWD "mkdirat_dir", 0);
  assert(stat("mkdirat_dir", &st) == 0); // relative path
  unlinkat(AT_FDCWD, PWD "mkdirat_dir", AT_REMOVEDIR); // like 'rmdir'

  return 0;
}


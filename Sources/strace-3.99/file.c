/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	$Id: file.c,v 1.10 1999/04/21 15:57:38 wichert Exp $
 */

#include "defs.h"

#include <dirent.h>

#ifdef linux
#  include <asm/stat.h>
#  define stat libc_stat
#  include <sys/stat.h>
#  undef stat
#else
#  include <sys/stat.h>
#endif

#include <fcntl.h>

#ifdef SVR4
#  include <sys/cred.h>
#endif /* SVR4 */

#include <sys/vfs.h>

#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif

#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif

#ifdef HAVE_SYS_ASYNCH_H
#include <sys/asynch.h>
#endif

#ifdef SUNOS4
#include <ustat.h>
#endif

/*
 * This is a really dirty trick but it should always work.  Traditional
 * Unix says r/w/rw are 0/1/2, so we make them true flags 1/2/3 by
 * adding 1.  Just remember to add 1 to any arg decoded with openmodes.
 */
struct xlat openmodes[] = {
	{ O_RDWR+1,	"O_RDWR"	},
	{ O_RDONLY+1,	"O_RDONLY"	},
	{ O_WRONLY+1,	"O_WRONLY"	},
	{ O_NONBLOCK,	"O_NONBLOCK"	},
	{ O_APPEND,	"O_APPEND"	},
	{ O_CREAT,	"O_CREAT"	},
	{ O_TRUNC,	"O_TRUNC"	},
	{ O_EXCL,	"O_EXCL"	},
	{ O_NOCTTY,	"O_NOCTTY"	},
#ifdef O_SYNC
	{ O_SYNC,	"O_SYNC"	},
#endif
#ifdef O_ASYNC
	{ O_ASYNC,	"O_ASYNC"	},
#endif
#ifdef O_DSYNC
	{ O_DSYNC,	"O_DSYNC"	},
#endif
#ifdef O_RSYNC
	{ O_RSYNC,	"O_RSYNC"	},
#endif
#ifdef O_NDELAY
	{ O_NDELAY,	"O_NDELAY"	},
#endif
#ifdef O_PRIV
	{ O_PRIV,	"O_PRIV"	},
#endif
#ifdef O_DIRECT
   { O_DIRECT, "O_DIRECT"  },
#endif
#ifdef O_LARGEFILE
   { O_LARGEFILE,  "O_LARGEFILE"   },
#endif
#ifdef O_DIRECTORY
   { O_DIRECTORY,  "O_DIRECTORY"   },
#endif

#ifdef FNDELAY
	{ FNDELAY,	"FNDELAY"	},
#endif
#ifdef FAPPEND
	{ FAPPEND,	"FAPPEND"	},
#endif
#ifdef FMARK
	{ FMARK,	"FMARK"		},
#endif
#ifdef FDEFER
	{ FDEFER,	"FDEFER"	},
#endif
#ifdef FASYNC
	{ FASYNC,	"FASYNC"	},
#endif
#ifdef FSHLOCK
	{ FSHLOCK,	"FSHLOCK"	},
#endif
#ifdef FEXLOCK
	{ FEXLOCK,	"FEXLOCK"	},
#endif
#ifdef FCREAT
	{ FCREAT,	"FCREAT"	},
#endif
#ifdef FTRUNC
	{ FTRUNC,	"FTRUNC"	},
#endif
#ifdef FEXCL
	{ FEXCL,	"FEXCL"		},
#endif
#ifdef FNBIO
	{ FNBIO,	"FNBIO"		},
#endif
#ifdef FSYNC
	{ FSYNC,	"FSYNC"		},
#endif
#ifdef FNOCTTY
	{ FNOCTTY,	"FNOCTTY"	},
#endif
	{ 0,		NULL		},
};

int
sys_open(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		/* flags */
		printflags(openmodes, tcp->u_arg[1] + 1);
		if (tcp->u_arg[1] & O_CREAT) {
			/* mode */
			tprintf(", %#lo", tcp->u_arg[2]);
		}
	}
	return 0;
}

#ifdef LINUXSPARC
struct xlat openmodessol[] = {
	{ 0,		"O_RDWR"	},
	{ 1,		"O_RDONLY"	},
	{ 2,		"O_WRONLY"	},
	{ 0x80,		"O_NONBLOCK"	},
	{ 8,		"O_APPEND"	},
	{ 0x100,	"O_CREAT"	},
	{ 0x200,	"O_TRUNC"	},
	{ 0x400,	"O_EXCL"	},
	{ 0x800,	"O_NOCTTY"	},
	{ 0x10,		"O_SYNC"	},
	{ 0x40,		"O_DSYNC"	},
	{ 0x8000,	"O_RSYNC"	},
	{ 4,		"O_NDELAY"	},
	{ 0x1000,	"O_PRIV"	},
	{ 0,		NULL		},
};

int
solaris_open(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		/* flags */
		printflags(openmodessol, tcp->u_arg[1] + 1);
		if (tcp->u_arg[1] & 0x100) {
			/* mode */
			tprintf(", %#lo", tcp->u_arg[2]);
		}
	}
	return 0;
}

#endif

int
sys_creat(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %#lo", tcp->u_arg[1]);
	}
	return 0;
}

static struct xlat access_flags[] = {
	{ F_OK,		"F_OK",		},
	{ R_OK,		"R_OK"		},
	{ W_OK,		"W_OK"		},
	{ X_OK,		"X_OK"		},
#ifdef EFF_ONLY_OK
	{ EFF_ONLY_OK,	"EFF_ONLY_OK"	},
#endif
#ifdef EX_OK
	{ EX_OK,	"EX_OK"		},
#endif
	{ 0,		NULL		},
};

int
sys_access(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		printflags(access_flags, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_umask(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%#lo", tcp->u_arg[0]);
	}
	return RVAL_OCTAL;
}

static struct xlat whence[] = {
	{ SEEK_SET,	"SEEK_SET"	},
	{ SEEK_CUR,	"SEEK_CUR"	},
	{ SEEK_END,	"SEEK_END"	},
	{ 0,		NULL		},
};

int
sys_lseek(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		if (tcp->u_arg[2] == SEEK_SET)
			tprintf("%lu, ", tcp->u_arg[1]);
		else
			tprintf("%ld, ", tcp->u_arg[1]);
		printxval(whence, tcp->u_arg[2], "SEEK_???");
	}
	return RVAL_UDECIMAL;
}

#ifdef linux
int
sys_llseek (tcp)
struct tcb *tcp;
{
    if (entering(tcp)) {
	if (tcp->u_arg[4] == SEEK_SET)
	    tprintf("%ld, %llu, ", tcp->u_arg[0],
		    (((unsigned long long int) tcp->u_arg[1]) << 32
		     | (unsigned long) tcp->u_arg[2]));
	else
	    tprintf("%ld, %lld, ", tcp->u_arg[0],
		    (((long long int) tcp->u_arg[1]) << 32
		     | (unsigned long) tcp->u_arg[2]));
    }
    else {
	if (syserror(tcp))
	    tprintf("%#lx, ", tcp->u_arg[3]);
	else {
	    long long int off;
	    umove(tcp, tcp->u_arg[3], &off);
	    tprintf("{%lld}, ", off);
	}
	printxval(whence, tcp->u_arg[4], "SEEK_???");
    }
    return 0;
}
#endif

int
sys_truncate(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_ftruncate(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, %lu", tcp->u_arg[0], tcp->u_arg[1]);
	}
	return 0;
}

/* several stats */

static struct xlat modetypes[] = {
	{ S_IFREG,	"S_IFREG"	},
	{ S_IFSOCK,	"S_IFSOCK"	},
	{ S_IFIFO,	"S_IFIFO"	},
	{ S_IFLNK,	"S_IFLNK"	},
	{ S_IFDIR,	"S_IFDIR"	},
	{ S_IFBLK,	"S_IFBLK"	},
	{ S_IFCHR,	"S_IFCHR"	},
	{ 0,		NULL		},
};

static char *
sprintmode(mode)
int mode;
{
	static char buf[64];
	char *s;

	if ((mode & S_IFMT) == 0)
		s = "";
	else if ((s = xlookup(modetypes, mode & S_IFMT)) == NULL) {
		sprintf(buf, "%#o", mode);
		return buf;
	}
	sprintf(buf, "%s%s%s%s", s,
		(mode & S_ISUID) ? "|S_ISUID" : "",
		(mode & S_ISGID) ? "|S_ISGID" : "",
		(mode & S_ISVTX) ? "|S_ISVTX" : "");
	mode &= ~(S_IFMT|S_ISUID|S_ISGID|S_ISVTX);
	if (mode)
		sprintf(buf + strlen(buf), "|%#o", mode);
	s = (*buf == '|') ? buf + 1 : buf;
	return *s ? s : "0";
}

static char *
sprinttime(t)
time_t t;
{
	struct tm *tmp;
	static char buf[32];

	if (t == 0) {
		sprintf(buf, "0");
		return buf;
	}
	tmp = localtime(&t);
	sprintf(buf, "%02d/%02d/%02d-%02d:%02d:%02d",
		tmp->tm_year, tmp->tm_mon + 1, tmp->tm_mday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
	return buf;
}

#ifdef LINUXSPARC
typedef struct {
        int     tv_sec;
        int     tv_nsec;
} timestruct_t;

struct solstat {
        unsigned        st_dev;
        int             st_pad1[3];     /* network id */
        unsigned        st_ino;
        unsigned        st_mode;
        unsigned        st_nlink;
        unsigned        st_uid;
        unsigned        st_gid;
        unsigned        st_rdev;
        int             st_pad2[2];
        int             st_size;
        int             st_pad3;        /* st_size, off_t expansion */
        timestruct_t    st_atime;
        timestruct_t    st_mtime;
        timestruct_t    st_ctime;
        int             st_blksize;
        int             st_blocks;
        char            st_fstype[16];
        int             st_pad4[8];     /* expansion area */
};

static void
printstatsol(tcp, addr)
struct tcb *tcp;
int addr;
{
	struct solstat statbuf;

	if (!addr) {
		tprintf("NULL");
		return;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#x", addr);
		return;
	}
	if (umove(tcp, addr, &statbuf) < 0) {
		tprintf("{...}");
		return;
	}
	if (!abbrev(tcp)) {
		tprintf("{st_dev=makedev(%lu, %lu), st_ino=%lu, st_mode=%s, ",
			(unsigned long) ((statbuf.st_dev >> 18) & 0x3fff),
			(unsigned long) (statbuf.st_dev & 0x3ffff),
			(unsigned long) statbuf.st_ino,
			sprintmode(statbuf.st_mode));
		tprintf("st_nlink=%lu, st_uid=%lu, st_gid=%lu, ",
			(unsigned long) statbuf.st_nlink,
			(unsigned long) statbuf.st_uid,
			(unsigned long) statbuf.st_gid);
		tprintf("st_blksize=%lu, ", (unsigned long) statbuf.st_blksize);
		tprintf("st_blocks=%lu, ", (unsigned long) statbuf.st_blocks);
	}
	else
		tprintf("{st_mode=%s, ", sprintmode(statbuf.st_mode));
	switch (statbuf.st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
		tprintf("st_rdev=makedev(%lu, %lu), ",
			(unsigned long) ((statbuf.st_rdev >> 18) & 0x3fff),
			(unsigned long) (statbuf.st_rdev & 0x3ffff));
		break;
	default:
		tprintf("st_size=%u, ", statbuf.st_size);
		break;
	}
	if (!abbrev(tcp)) {
		tprintf("st_atime=%s, ", sprinttime(statbuf.st_atime));
		tprintf("st_mtime=%s, ", sprinttime(statbuf.st_mtime));
		tprintf("st_ctime=%s}", sprinttime(statbuf.st_ctime));
	}
	else
		tprintf("...}");
}
#endif /* LINUXSPARC */

static void
realprintstat(tcp, statbuf)
struct tcb *tcp;
struct stat *statbuf;
{
    if (!abbrev(tcp)) {
	    tprintf("{st_dev=makedev(%lu, %lu), st_ino=%lu, st_mode=%s, ",
		    (unsigned long) major(statbuf->st_dev),
		    (unsigned long) minor(statbuf->st_dev),
		    (unsigned long) statbuf->st_ino,
		    sprintmode(statbuf->st_mode));
	    tprintf("st_nlink=%lu, st_uid=%lu, st_gid=%lu, ",
		    (unsigned long) statbuf->st_nlink,
		    (unsigned long) statbuf->st_uid,
		    (unsigned long) statbuf->st_gid);
#ifdef HAVE_ST_BLKSIZE
	    tprintf("st_blksize=%lu, ", (unsigned long) statbuf->st_blksize);
#endif /* HAVE_ST_BLKSIZE */
#ifdef HAVE_ST_BLOCKS
	    tprintf("st_blocks=%lu, ", (unsigned long) statbuf->st_blocks);
#endif /* HAVE_ST_BLOCKS */
    }
    else
	    tprintf("{st_mode=%s, ", sprintmode(statbuf->st_mode));
    switch (statbuf->st_mode & S_IFMT) {
    case S_IFCHR: case S_IFBLK:
#ifdef HAVE_ST_RDEV
	    tprintf("st_rdev=makedev(%lu, %lu), ",
		    (unsigned long) major(statbuf->st_rdev),
		    (unsigned long) minor(statbuf->st_rdev));
#else /* !HAVE_ST_RDEV */
	    tprintf("st_size=makedev(%lu, %lu), ",
		    (unsigned long) major(statbuf->st_size),
		    (unsigned long) minor(statbuf->st_size));
#endif /* !HAVE_ST_RDEV */
	    break;
    default:
	    tprintf("st_size=%lu, ", statbuf->st_size);
	    break;
    }
    if (!abbrev(tcp)) {
	    tprintf("st_atime=%s, ", sprinttime(statbuf->st_atime));
	    tprintf("st_mtime=%s, ", sprinttime(statbuf->st_mtime));
	    tprintf("st_ctime=%s}", sprinttime(statbuf->st_ctime));
    }
    else
	    tprintf("...}");
}


static void
printstat(tcp, addr)
struct tcb *tcp;
int addr;
{
	struct stat statbuf;

#ifdef LINUXSPARC
 	if (current_personality == 1) {
 		printstatsol(tcp, addr);
 		return;
 	}
#endif /* LINUXSPARC */

	if (!addr) {
		tprintf("NULL");
		return;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#x", addr);
		return;
	}
	if (umove(tcp, addr, &statbuf) < 0) {
		tprintf("{...}");
		return;
	}

	realprintstat(tcp, &statbuf);
}

#ifdef linux
static void
convertoldstat(oldbuf, newbuf)
const struct __old_kernel_stat *oldbuf;
struct stat *newbuf;
{
    newbuf->st_dev=oldbuf->st_dev;
    newbuf->st_ino=oldbuf->st_ino;
    newbuf->st_mode=oldbuf->st_mode;
    newbuf->st_nlink=oldbuf->st_nlink;
    newbuf->st_uid=oldbuf->st_uid;
    newbuf->st_gid=oldbuf->st_gid;
    newbuf->st_rdev=oldbuf->st_rdev;
    newbuf->st_size=oldbuf->st_size;
    newbuf->st_atime=oldbuf->st_atime;
    newbuf->st_mtime=oldbuf->st_mtime;
    newbuf->st_ctime=oldbuf->st_ctime;
    newbuf->st_blksize=0;	/* not supported in old_stat */
    newbuf->st_blocks=0;		/* not supported in old_stat */
}
#endif


#ifdef linux
static void
printoldstat(tcp, addr)
struct tcb *tcp;
int addr;
{
	struct __old_kernel_stat statbuf;
	struct stat newstatbuf;

#ifdef LINUXSPARC
 	if (current_personality == 1) {
 		printstatsol(tcp, addr);
 		return;
 	}
#endif /* LINUXSPARC */

	if (!addr) {
		tprintf("NULL");
		return;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#x", addr);
		return;
	}
	if (umove(tcp, addr, &statbuf) < 0) {
		tprintf("{...}");
		return;
	}

	convertoldstat(&statbuf, &newstatbuf);
	realprintstat(tcp, &newstatbuf);
}
#endif


int
sys_stat(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
	} else {
		printstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef linux
int
sys_oldstat(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
	} else {
		printoldstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}
#endif

int
sys_fstat(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		tprintf("%ld, ", tcp->u_arg[0]);
	else {
		printstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef linux
int
sys_oldfstat(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		tprintf("%ld, ", tcp->u_arg[0]);
	else {
		printoldstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}
#endif

int
sys_lstat(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
	} else {
		printstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef linux
int
sys_oldlstat(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
	} else {
		printoldstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}
#endif


#if defined(SVR4) || defined(LINUXSPARC)

int
sys_xstat(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprintf(", ");
	} else {
		printstat(tcp, tcp->u_arg[2]);
	}
	return 0;
}

int
sys_fxstat(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		tprintf("%ld, %ld, ", tcp->u_arg[0], tcp->u_arg[1]);
	else {
		printstat(tcp, tcp->u_arg[2]);
	}
	return 0;
}

int
sys_lxstat(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprintf(", ");
	} else {
		printstat(tcp, tcp->u_arg[2]);
	}
	return 0;
}

int
sys_xmknod(tcp)
struct tcb *tcp;
{
	int mode = tcp->u_arg[2];

	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprintf(", %s", sprintmode(mode));
		switch (mode & S_IFMT) {
		case S_IFCHR: case S_IFBLK:
#ifdef LINUXSPARC
			tprintf(", makedev(%lu, %lu)",
				(unsigned long) ((tcp->u_arg[3] >> 18) & 0x3fff),
				(unsigned long) (tcp->u_arg[3] & 0x3ffff));
#else		
			tprintf(", makedev(%lu, %lu)",
				(unsigned long) major(tcp->u_arg[3]),
				(unsigned long) minor(tcp->u_arg[3]));
#endif				
			break;
		default:
			break;
		}
	}
	return 0;
}

#endif /* SVR4 || LINUXSPARC */

#ifdef linux

static struct xlat fsmagic[] = {
	{ 0xef51,	"EXT2_OLD_SUPER_MAGIC"	},
	{ 0xef53,	"EXT2_SUPER_MAGIC"	},
	{ 0x137d,	"EXT_SUPER_MAGIC"	},
	{ 0x9660,	"ISOFS_SUPER_MAGIC"	},
	{ 0x137f,	"MINIX_SUPER_MAGIC"	},
	{ 0x138f,	"MINIX_SUPER_MAGIC2"	},
	{ 0x2468,	"NEW_MINIX_SUPER_MAGIC"	},
	{ 0x4d44,	"MSDOS_SUPER_MAGIC"	},
	{ 0x6969,	"NFS_SUPER_MAGIC"	},
	{ 0x9fa0,	"PROC_SUPER_MAGIC"	},
	{ 0x012fd16d,	"XIAFS_SUPER_MAGIC"	},
	{ 0,		NULL			},
};

#endif /* linux */

#ifndef SVR4

static char *
sprintfstype(magic)
int magic;
{
	static char buf[32];
#ifdef linux
	char *s;

	s = xlookup(fsmagic, magic);
	if (s) {
		sprintf(buf, "\"%s\"", s);
		return buf;
	}
#endif /* linux */
	sprintf(buf, "%#x", magic);
	return buf;
}

static void
printstatfs(tcp, addr)
struct tcb *tcp;
long addr;
{
	struct statfs statbuf;

	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &statbuf) < 0) {
		tprintf("{...}");
		return;
	}
#ifdef ALPHA

	tprintf("{f_type=%s, f_fbsize=%u, f_blocks=%u, f_bfree=%u, ",
		sprintfstype(statbuf.f_type),
		statbuf.f_bsize, statbuf.f_blocks, statbuf.f_bfree);
	tprintf("f_bavail=%u, f_files=%u, f_ffree=%u, f_namelen=%u}",
		statbuf.f_bavail,statbuf.f_files, statbuf.f_ffree, statbuf.f_namelen);
#else /* !ALPHA */
	tprintf("{f_type=%s, f_bsize=%lu, f_blocks=%lu, f_bfree=%lu, ",
		sprintfstype(statbuf.f_type),
		(unsigned long)statbuf.f_bsize,
		(unsigned long)statbuf.f_blocks,
		(unsigned long)statbuf.f_bfree);
	tprintf("f_files=%lu, f_ffree=%lu",
		(unsigned long)statbuf.f_files,
		(unsigned long)statbuf.f_ffree);
#ifdef linux
	tprintf(", f_namelen=%lu}", (unsigned long)statbuf.f_namelen);
#endif /* linux */
#endif /* !ALPHA */
	tprintf("}");
}

int
sys_statfs(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_fstatfs(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#if defined(linux) && defined(__alpha)

int
osf_statfs(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

int
osf_fstatfs(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}
#endif /* linux && __alpha */

#endif /* !SVR4 */

#ifdef SUNOS4

int
sys_ustat(tcp)
struct tcb *tcp;
{
	struct ustat statbuf;

	if (entering(tcp)) {
		tprintf("makedev(%lu, %lu), ",
				(long) major(tcp->u_arg[0]),
				(long) minor(tcp->u_arg[0]));
	}
	else {
		if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umove(tcp, tcp->u_arg[1], &statbuf) < 0)
			tprintf("{...}");
		else {
			tprintf("{f_tfree=%lu, f_tinode=%lu, ",
				statbuf.f_tfree, statbuf.f_tinode);
			tprintf("f_fname=\"%.*s\", ",
				(int) sizeof(statbuf.f_fname),
				statbuf.f_fname);
			tprintf("f_fpack=\"%.*s\"}",
				(int) sizeof(statbuf.f_fpack),
				statbuf.f_fpack);
		}
	}
	return 0;
}

#endif /* SUNOS4 */

/* directory */
int
sys_chdir(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
	}
	return 0;
}

int
sys_mkdir(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %#lo", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_rmdir(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
	}
	return 0;
}

int
sys_fchdir(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld", tcp->u_arg[0]);
	}
	return 0;
}

int
sys_chroot(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
	}
	return 0;
}

int
sys_fchroot(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld", tcp->u_arg[0]);
	}
	return 0;
}

int
sys_link(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		printpath(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_unlink(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
	}
	return 0;
}

int
sys_symlink(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		printpath(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_readlink(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printpathn(tcp, tcp->u_arg[1], tcp->u_rval);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_rename(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		printpath(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_chown(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu, %lu", tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

int
sys_fchown(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, %lu, %lu",
			tcp->u_arg[0], tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

int
sys_chmod(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %#lo", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_fchmod(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, %#lo", tcp->u_arg[0], tcp->u_arg[1]);
	}
	return 0;
}

int
sys_utimes(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_utime(tcp)
struct tcb *tcp;
{
	long ut[2];

	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		if (!tcp->u_arg[1])
			tprintf("NULL");
		else if (!verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umoven(tcp, tcp->u_arg[1], sizeof ut,
		    (char *) ut) < 0)
			tprintf("[?, ?]");
		else {
			tprintf("[%s,", sprinttime(ut[0]));
			tprintf(" %s]", sprinttime(ut[1]));
		}
	}
	return 0;
}

int
sys_mknod(tcp)
struct tcb *tcp;
{
	int mode = tcp->u_arg[1];

	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %s", sprintmode(mode));
		switch (mode & S_IFMT) {
		case S_IFCHR: case S_IFBLK:
#ifdef LINUXSPARC
			if (current_personality == 1)
			tprintf(", makedev(%lu, %lu)",
				(unsigned long) ((tcp->u_arg[2] >> 18) & 0x3fff),
				(unsigned long) (tcp->u_arg[2] & 0x3ffff));
			else
#endif	
			tprintf(", makedev(%lu, %lu)",
				(unsigned long) major(tcp->u_arg[2]),
				(unsigned long) minor(tcp->u_arg[2]));
			break;
		default:
			break;
		}
	}
	return 0;
}

int
sys_mkfifo(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %#lo", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_fsync(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld", tcp->u_arg[0]);
	}
	return 0;
}

#ifdef linux

static void
printdir(tcp, addr)
struct tcb *tcp;
long addr;
{
	struct dirent d;

	if (!verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &d) < 0) {
		tprintf("{...}");
		return;
	}
	tprintf("{d_ino=%ld, ", (unsigned long) d.d_ino);
	tprintf("d_name=");
	printpathn(tcp, (long) ((struct dirent *) addr)->d_name, d.d_reclen);
	tprintf("}");
}

int
sys_readdir(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp) || tcp->u_rval == 0 || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printdir(tcp, tcp->u_arg[1]);
		/* Not much point in printing this out, it is always 1. */
		if (tcp->u_arg[2] != 1)
			tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

#endif /* linux */

int
sys_getdents(tcp)
struct tcb *tcp;
{
	int i, len, dents = 0;
	char *buf;

	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
		return 0;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx, %lu", tcp->u_arg[1], tcp->u_arg[2]);
		return 0;
	}
	len = tcp->u_rval;
	if ((buf = malloc(len)) == NULL) {
		tprintf("out of memory\n");
		return 0;
	}
	if (umoven(tcp, tcp->u_arg[1], len, buf) < 0) {
		tprintf("{...}, %lu", tcp->u_arg[2]);
		free(buf);
		return 0;
	}
	if (!abbrev(tcp))
		tprintf("{");
	for (i = 0; i < len;) {
		struct dirent *d = (struct dirent *) &buf[i];
#ifdef linux
		if (!abbrev(tcp)) {
			tprintf("%s{d_ino=%lu, d_off=%lu, ",
				i ? " " : "", d->d_ino, d->d_off);
			tprintf("d_reclen=%u, d_name=\"%s\"}",
				d->d_reclen, d->d_name);
		}
#endif /* linux */
#ifdef SVR4
		if (!abbrev(tcp)) {
			tprintf("%s{d_ino=%lu, d_off=%lu, ",
				i ? " " : "", d->d_ino, d->d_off);
			tprintf("d_reclen=%u, d_name=\"%s\"}",
				d->d_reclen, d->d_name);
		}
#endif /* SVR4 */
#ifdef SUNOS4
		if (!abbrev(tcp)) {
			tprintf("%s{d_off=%lu, d_fileno=%lu, d_reclen=%u, ",
				i ? " " : "", d->d_off, d->d_fileno,
				d->d_reclen);
			tprintf("d_namlen=%u, d_name=\"%.*s\"}",
				d->d_namlen, d->d_namlen, d->d_name);
		}
#endif /* SUNOS4 */
		i += d->d_reclen;
		dents++;
	}
	if (!abbrev(tcp))
		tprintf("}");
	else
		tprintf("/* %u entries */", dents);
	tprintf(", %lu", tcp->u_arg[2]);
	free(buf);
	return 0;
}

#ifdef linux

int
sys_getcwd(tcp)
struct tcb *tcp;
{
    if (exiting(tcp)) {
	if (syserror(tcp))
	    tprintf("%#lx", tcp->u_arg[0]);
	else
	    printstr(tcp, tcp->u_arg[0], tcp->u_arg[1]);
	tprintf(", %lu", tcp->u_arg[1]);
    }
    return 0;
}
#endif /* linux */

#ifdef HAVE_SYS_ASYNCH_H

int
sys_aioread(tcp)
struct tcb *tcp;
{
	struct aio_result_t res;

	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu, %lu, ", tcp->u_arg[2], tcp->u_arg[3]);
		printxval(whence, tcp->u_arg[4], "L_???");
		if (syserror(tcp) || tcp->u_arg[5] == 0
		    || umove(tcp, tcp->u_arg[5], &res) < 0)
			tprintf(", %#lx", tcp->u_arg[5]);
		else
			tprintf(", {aio_return %d aio_errno %d}",
				res.aio_return, res.aio_errno);
	}
	return 0;
}

int
sys_aiowrite(tcp)
struct tcb *tcp;
{
	struct aio_result_t res;

	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
		printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu, %lu, ", tcp->u_arg[2], tcp->u_arg[3]);
		printxval(whence, tcp->u_arg[4], "L_???");
	}
	else {
		if (tcp->u_arg[5] == 0)
			tprintf(", NULL");
		else if (syserror(tcp)
		    || umove(tcp, tcp->u_arg[5], &res) < 0)
			tprintf(", %#lx", tcp->u_arg[5]);
		else
			tprintf(", {aio_return %d aio_errno %d}",
				res.aio_return, res.aio_errno);
	}
	return 0;
}

int
sys_aiowait(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		printtv(tcp, tcp->u_arg[0]);
	return 0;
}

int
sys_aiocancel(tcp)
struct tcb *tcp;
{
	struct aio_result_t res;

	if (exiting(tcp)) {
		if (tcp->u_arg[0] == 0)
			tprintf("NULL");
		else if (syserror(tcp)
		    || umove(tcp, tcp->u_arg[0], &res) < 0)
			tprintf("%#lx", tcp->u_arg[0]);
		else
			tprintf("{aio_return %d aio_errno %d}",
				res.aio_return, res.aio_errno);
	}
	return 0;
}

#endif /* HAVE_SYS_ASYNCH_H */

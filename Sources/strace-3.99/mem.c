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
 *	$Id: mem.c,v 1.4 1999/04/21 15:57:38 wichert Exp $
 */

#include "defs.h"

#ifdef LINUXSPARC
#include <linux/mman.h>
#else
#include <sys/mman.h>
#endif

#if defined(LINUX) && defined(__i386__)
#include <asm/ldt.h>
#endif

int
sys_brk(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%#lx", tcp->u_arg[0]);
	}
#ifdef LINUX
	return RVAL_HEX;
#else
	return 0;
#endif
}

int
sys_sbrk(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
	}
	return RVAL_HEX;
}

static struct xlat mmap_prot[] = {
	{ PROT_NONE,	"PROT_NONE",	},
	{ PROT_READ,	"PROT_READ"	},
	{ PROT_WRITE,	"PROT_WRITE"	},
	{ PROT_EXEC,	"PROT_EXEC"	},
	{ 0,		NULL		},
};

static struct xlat mmap_flags[] = {
	{ MAP_SHARED,	"MAP_SHARED"	},
	{ MAP_PRIVATE,	"MAP_PRIVATE"	},
	{ MAP_FIXED,	"MAP_FIXED"	},
#ifdef MAP_ANONYMOUS
	{ MAP_ANONYMOUS,"MAP_ANONYMOUS"	},
#endif
#ifdef MAP_RENAME
	{ MAP_RENAME,	"MAP_RENAME"	},
#endif
#ifdef MAP_NORESERVE
	{ MAP_NORESERVE,"MAP_NORESERVE"	},
#endif
#ifdef _MAP_NEW
	{ _MAP_NEW,	"_MAP_NEW"	},
#endif
#ifdef MAP_GROWSDOWN
	{ MAP_GROWSDOWN,"MAP_GROWSDOWN"	},
#endif
#ifdef MAP_DENYWRITE
	{ MAP_DENYWRITE,"MAP_DENYWRITE"	},
#endif
#ifdef MAP_EXECUTABLE
	{ MAP_EXECUTABLE,"MAP_EXECUTABLE"},
#endif
#ifdef MAP_FILE
	{ MAP_FILE,"MAP_FILE"},
#endif
#ifdef MAP_LOCKED
	{ MAP_LOCKED,"MAP_LOCKED"},
#endif
	{ 0,		NULL		},
};

int
sys_mmap(tcp)
struct tcb *tcp;
{
#ifdef LINUX
#  if defined(ALPHA) || defined(sparc)
	long *u_arg = tcp->u_arg;
#  else /* !ALPHA */
	long u_arg[6];
#  endif /* !ALPHA */
#else /* !LINUX */
	long *u_arg = tcp->u_arg;
#endif /* !LINUX */

	if (entering(tcp)) {
#if defined(LINUX) && !defined(ALPHA) && !defined(__sparc__)
		if (umoven(tcp, tcp->u_arg[0], sizeof u_arg,
				(char *) u_arg) == -1)
			return 0;
#endif /* LINUX && !ALPHA && !sparc */

		/* addr */
		tprintf("%#lx, ", u_arg[0]);
		/* len */
		tprintf("%lu, ", u_arg[1]);
		/* prot */
		printflags(mmap_prot, u_arg[2]);
		tprintf(", ");
		/* flags */
		printxval(mmap_flags, u_arg[3] & MAP_TYPE, "MAP_???");
		addflags(mmap_flags, u_arg[3] & ~MAP_TYPE);
		/* fd */
		tprintf(", %ld, ", u_arg[4]);
		/* offset */
		tprintf("%#lx", u_arg[5]);
	}
	return RVAL_HEX;
}

int
sys_munmap(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu",
			tcp->u_arg[0], tcp->u_arg[1]);
	}
	return 0;
}

int
sys_mprotect(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, ",
			tcp->u_arg[0], tcp->u_arg[1]);
		if (!printflags(mmap_prot, tcp->u_arg[2]))
			tprintf("PROT_???");
	}
	return 0;
}

#ifdef MS_ASYNC

static struct xlat mctl_sync[] = {
	{ MS_ASYNC,	"MS_ASYNC"	},
	{ MS_INVALIDATE,"MS_INVALIDATE"	},
#ifdef MS_SYNC
	{ MS_SYNC,	"MS_SYNC"	},
#endif
	{ 0,		NULL		},
};

int
sys_msync(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		/* addr */
		tprintf("%#lx", tcp->u_arg[0]);
		/* len */
		tprintf(", %lu, ", tcp->u_arg[1]);
		/* flags */
		if (!printflags(mctl_sync, tcp->u_arg[2]))
			tprintf("MS_???");
	}
	return 0;
}

#endif /* MS_ASYNC */

#ifdef MC_SYNC

static struct xlat mctl_funcs[] = {
	{ MC_LOCK,	"MC_LOCK"	},
	{ MC_LOCKAS,	"MC_LOCKAS"	},
	{ MC_SYNC,	"MC_SYNC"	},
	{ MC_UNLOCK,	"MC_UNLOCK"	},
	{ MC_UNLOCKAS,	"MC_UNLOCKAS"	},
	{ 0,		NULL		},
};

static struct xlat mctl_lockas[] = {
	{ MCL_CURRENT,	"MCL_CURRENT"	},
	{ MCL_FUTURE,	"MCL_FUTURE"	},
	{ 0,		NULL		},
};

int
sys_mctl(tcp)
struct tcb *tcp;
{
	int arg, function;

	if (entering(tcp)) {
		/* addr */
		tprintf("%#lx", tcp->u_arg[0]);
		/* len */
		tprintf(", %lu, ", tcp->u_arg[1]);
		/* function */
		function = tcp->u_arg[2];
		if (!printflags(mctl_funcs, function))
			tprintf("MC_???");
		/* arg */
		arg = tcp->u_arg[3];
		tprintf(", ");
		switch (function) {
		case MC_SYNC:
			if (!printflags(mctl_sync, arg))
				tprintf("MS_???");
			break;
		case MC_LOCKAS:
			if (!printflags(mctl_lockas, arg))
				tprintf("MCL_???");
			break;
		default:
			tprintf("%#x", arg);
			break;
		}
	}
	return 0;
}

#endif /* MC_SYNC */

int
sys_mincore(tcp)
struct tcb *tcp;
{
	int i, len;
	char *vec = NULL;

	if (entering(tcp)) {
		tprintf("%#lx, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
	} else {
		len = tcp->u_arg[1];
		if (syserror(tcp) || tcp->u_arg[2] == 0 ||
			(vec = malloc((u_int)len)) == NULL ||
			umoven(tcp, tcp->u_arg[2], len, vec) < 0)
			tprintf("%#lx", tcp->u_arg[2]);
		else {
			tprintf("[");
			for (i = 0; i < len; i++) {
				if (abbrev(tcp) && i >= max_strlen) {
					tprintf("...");
					break;
				}
				tprintf((vec[i] & 1) ? "1" : "0");
			}
			tprintf("]");
		}
		if (vec)
			free(vec);
	}
	return 0;
}

int
sys_getpagesize(tcp)
struct tcb *tcp;
{
	if (exiting(tcp))
		return RVAL_HEX;
	return 0;
}

#if defined(LINUX) && defined(__i386__)
int
sys_modify_ldt(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		struct modify_ldt_ldt_s copy;
		tprintf("%ld", tcp->u_arg[0]);
		if (tcp->u_arg[1] == 0
				|| tcp->u_arg[2] != sizeof (struct modify_ldt_ldt_s)
				|| umove(tcp, tcp->u_arg[1], &copy) == -1)
			tprintf(", %lx", tcp->u_arg[1]);
		else {
			tprintf(", {entry_number:%d, ", copy.entry_number);
			if (!verbose(tcp))
				tprintf("...}");
			else {
				tprintf("base_addr:%#08lx, "
						"limit:%d, "
						"seg_32bit:%d, "
						"contents:%d, "
						"read_exec_only:%d, "
						"limit_in_pages:%d, "
						"seg_not_present:%d, "
						"useable:%d}",
						copy.base_addr,
						copy.limit,
						copy.seg_32bit,
						copy.contents,
						copy.read_exec_only,
						copy.limit_in_pages,
						copy.seg_not_present,
						copy.useable);
			}
		}
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}
#endif /* LINUX && __i386__ */


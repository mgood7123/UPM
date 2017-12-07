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
 *	$Id: io.c,v 1.1.1.1 1999/02/19 00:22:12 wichert Exp $
 */

#include "defs.h"

#include <fcntl.h>
#include <sys/uio.h>

int
sys_read(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_write(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_readv(tcp)
struct tcb *tcp;
{
	struct iovec *iov;
	int i, len;

	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp)) {
			tprintf("%#lx, %lu",
					tcp->u_arg[1], tcp->u_arg[2]);
			return 0;
		}
		len = tcp->u_arg[2];
		if ((iov = (struct iovec *) malloc(len * sizeof *iov)) == NULL) {
			fprintf(stderr, "No memory");
			return 0;
		}
		if (umoven(tcp, tcp->u_arg[1],
				len * sizeof *iov, (char *) iov) < 0) {
			tprintf("%#lx", tcp->u_arg[1]);
		} else {
			tprintf("[");
			for (i = 0; i < len; i++) {
				if (i)
					tprintf(", ");
				tprintf("{");
				printstr(tcp, (long) iov[i].iov_base,
					iov[i].iov_len);
				tprintf(", %u}", iov[i].iov_len);
			}
			tprintf("]");
		}
		free((char *) iov);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_writev(tcp)
struct tcb *tcp;
{
	struct iovec *iov;
	int i, len;

	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		len = tcp->u_arg[2];
		iov = (struct iovec *) malloc(len * sizeof *iov);
		if (iov == NULL) {
			fprintf(stderr, "No memory");
			return 0;
		}
		if (umoven(tcp, tcp->u_arg[1],
				len * sizeof *iov, (char *) iov) < 0) {
			tprintf("%#lx", tcp->u_arg[1]);
		} else {
			tprintf("[");
			for (i = 0; i < len; i++) {
				if (i)
					tprintf(", ");
				tprintf("{");
				printstr(tcp, (long) iov[i].iov_base,
					iov[i].iov_len);
				tprintf(", %u}", iov[i].iov_len);
			}
			tprintf("]");
		}
		free((char *) iov);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

#ifdef SVR4

int
sys_pread(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);
		tprintf(", %lu, %llu", tcp->u_arg[2],
				(((unsigned long long) tcp->u_arg[4]) << 32
				 | tcp->u_arg[3]));
	}
	return 0;
}

int
sys_pwrite(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu, %llu", tcp->u_arg[2],
				(((unsigned long long) tcp->u_arg[4]) << 32
				 | tcp->u_arg[3]));
	}
	return 0;
}
#endif /* SVR4 */

#ifdef LINUX
int
sys_pread(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);
		tprintf(", %lu, %lu", tcp->u_arg[2], tcp->u_arg[3]);
	}
	return 0;
}

int
sys_pwrite(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu, %lu", tcp->u_arg[2], tcp->u_arg[3]);
	}
	return 0;
}

#endif /* LINUX */

int
sys_ioctl(tcp)
struct tcb *tcp;
{
	char *symbol;

	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		symbol = ioctl_lookup(tcp->u_arg[1]);
		if (symbol)
			tprintf("%s", symbol);
		else
			tprintf("%#lx", tcp->u_arg[1]);
		ioctl_decode(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	else {
		if (ioctl_decode(tcp, tcp->u_arg[1], tcp->u_arg[2]) == 0)
			tprintf(", %#lx", tcp->u_arg[2]);
	}
	return 0;
}

/*
 * Support for decoding of KVM_* ioctl commands.
 *
 * Copyright (c) 2017 Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2017 Red Hat, Inc.
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
 */

#include "defs.h"

#ifdef HAVE_LINUX_KVM_H
# include <linux/kvm.h>

static int
kvm_ioctl_create_vcpu(struct tcb *const tcp, const kernel_ulong_t arg)
{
	uint32_t cpuid = arg;

	tprintf(", %u", cpuid);
	return RVAL_IOCTL_DECODED | RVAL_FD;
}

int
kvm_ioctl(struct tcb *const tcp, const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case KVM_CREATE_VCPU:
		return kvm_ioctl_create_vcpu(tcp, arg);
	case KVM_CREATE_VM:
		return RVAL_DECODED | RVAL_FD;
	case KVM_RUN:
	case KVM_GET_VCPU_MMAP_SIZE:
	case KVM_GET_API_VERSION:
	default:
		return RVAL_DECODED;
	}
}

#endif /* HAVE_LINUX_KVM_H */

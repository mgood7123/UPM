/*
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1995-2017 The strace developers.
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

[  0] = { 6,	0,		SEN(printargs),			"osf_syscall"		}, /* not implemented */
[  1] = { 1,	TP|SE,		SEN(exit),			"exit"			},
[  2] = { 0,	TP,		SEN(fork),			"fork"			},
[  3] = { 3,	TD,		SEN(read),			"read"			},
[  4] = { 3,	TD,		SEN(write),			"write"			},
[  5] = { 5,	0,		SEN(printargs),			"osf_old_open"		}, /* not implemented */
[  6] = { 1,	TD,		SEN(close),			"close"			},
[  7] = { 4,	TP,		SEN(osf_wait4),			"osf_wait4"		},
[  8] = { 5,	0,		SEN(printargs),			"osf_old_creat"		}, /* not implemented */
[  9] = { 2,	TF,		SEN(link),			"link"			},
[ 10] = { 1,	TF,		SEN(unlink),			"unlink"		},
[ 11] = { 5,	0,		SEN(printargs),			"osf_execve"		}, /* not implemented */
[ 12] = { 1,	TF,		SEN(chdir),			"chdir"			},
[ 13] = { 1,	TD,		SEN(fchdir),			"fchdir"		},
[ 14] = { 3,	TF,		SEN(mknod),			"mknod"			},
[ 15] = { 2,	TF,		SEN(chmod),			"chmod"			},
[ 16] = { 3,	TF,		SEN(chown),			"chown"			},
[ 17] = { 1,	TM|SI,		SEN(brk),			"brk"			},
[ 18] = { 5,	TSFA,		SEN(printargs),			"osf_getfsstat"		}, /* not implemented */
[ 19] = { 3,	TD,		SEN(lseek),			"lseek"			},
[ 20] = { 0,	NF,		SEN(getxpid),			"getxpid"		},
[ 21] = { 4,	0,		SEN(printargs),			"osf_mount"		},
[ 22] = { 2,	TF,		SEN(umount2),			"umount"		},
[ 23] = { 1,	0,		SEN(setuid),			"setuid"		},
[ 24] = { 0,	NF,		SEN(getxuid),			"getxuid"		},
[ 25] = { 5,	0,		SEN(printargs),			"exec_with_loader"	}, /* not implemented */
[ 26] = { 4,	0,		SEN(ptrace),			"ptrace"		},
[ 27] = { 5,	0,		SEN(printargs),			"osf_nrecvmsg"		}, /* not implemented */
[ 28] = { 5,	0,		SEN(printargs),			"osf_nsendmsg"		}, /* not implemented */
[ 29] = { 5,	0,		SEN(printargs),			"osf_nrecvfrom"		}, /* not implemented */
[ 30] = { 5,	0,		SEN(printargs),			"osf_naccept"		}, /* not implemented */
[ 31] = { 5,	0,		SEN(printargs),			"osf_ngetpeername"	}, /* not implemented */
[ 32] = { 5,	0,		SEN(printargs),			"osf_ngetsockname"	}, /* not implemented */
[ 33] = { 2,	TF,		SEN(access),			"access"		},
[ 34] = { 5,	0,		SEN(printargs),			"osf_chflags"		}, /* not implemented */
[ 35] = { 5,	0,		SEN(printargs),			"osf_fchflags"		}, /* not implemented */
[ 36] = { 0,	0,		SEN(sync),			"sync"			},
[ 37] = { 2,	TS,		SEN(kill),			"kill"			},
[ 38] = { 5,	TF|TST|TSTA,	SEN(printargs),			"osf_old_stat"		}, /* not implemented */
[ 39] = { 2,	0,		SEN(setpgid),			"setpgid"		},
[ 40] = { 5,	TF|TLST|TSTA,	SEN(printargs),			"osf_old_lstat"		}, /* not implemented */
[ 41] = { 1,	TD,		SEN(dup),			"dup"			},
[ 42] = { 0,	TD,		SEN(pipe),			"pipe"			},
[ 43] = { 4,	0,		SEN(printargs),			"osf_set_program_attributes"	},
[ 44] = { 5,	0,		SEN(printargs),			"osf_profil"		}, /* not implemented */
[ 45] = { 3,	TD|TF,		SEN(open),			"open"			},
[ 46] = { 5,	0,		SEN(printargs),			"osf_old_sigaction"	}, /* not implemented */
[ 47] = { 0,	NF,		SEN(getxgid),			"getxgid"		},
[ 48] = { 2,	TS,		SEN(osf_sigprocmask),		"osf_sigprocmask"	},
[ 49] = { 5,	0,		SEN(printargs),			"osf_getlogin"		}, /* not implemented */
[ 50] = { 5,	0,		SEN(printargs),			"osf_setlogin"		}, /* not implemented */
[ 51] = { 1,	TF,		SEN(acct),			"acct"			},
[ 52] = { 1,	TS,		SEN(sigpending),		"sigpending"		},
[ 53] = { },
[ 54] = { 3,	TD,		SEN(ioctl),			"ioctl"			},
[ 55] = { 5,	0,		SEN(printargs),			"osf_reboot"		}, /* not implemented */
[ 56] = { 5,	0,		SEN(printargs),			"osf_revoke"		}, /* not implemented */
[ 57] = { 2,	TF,		SEN(symlink),			"symlink"		},
[ 58] = { 3,	TF,		SEN(readlink),			"readlink"		},
[ 59] = { 3,	TF|TP|SE|SI,	SEN(execve),			"execve"		},
[ 60] = { 1,	NF,		SEN(umask),			"umask"			},
[ 61] = { 1,	TF,		SEN(chroot),			"chroot"		},
[ 62] = { 5,	TD|TFST|TSTA,	SEN(printargs),			"osf_old_fstat"		}, /* not implemented */
[ 63] = { 0,	0,		SEN(getpgrp),			"getpgrp"		},
[ 64] = { 0,	0,		SEN(getpagesize),		"getpagesize"		},
[ 65] = { 5,	TM,		SEN(printargs),			"osf_mremap"		}, /* not implemented */
[ 66] = { 0,	TP,		SEN(vfork),			"vfork"			},
[ 67] = { 2,	TF|TST|TSTA,	SEN(stat),			"stat"			},
[ 68] = { 2,	TF|TLST|TSTA,	SEN(lstat),			"lstat"			},
[ 69] = { 5,	TM,		SEN(printargs),			"osf_sbrk"		}, /* not implemented */
[ 70] = { 5,	0,		SEN(printargs),			"osf_sstk"		}, /* not implemented */
[ 71] = { 6,	TD|TM|SI,	SEN(mmap),			"mmap"			},
[ 72] = { 5,	0,		SEN(printargs),			"osf_old_vadvise"	}, /* not implemented */
[ 73] = { 2,	TM|SI,		SEN(munmap),			"munmap"		},
[ 74] = { 3,	TM|SI,		SEN(mprotect),			"mprotect"		},
[ 75] = { 3,	TM,		SEN(madvise),			"madvise"		},
[ 76] = { 0,	0,		SEN(vhangup),			"vhangup"		},
[ 77] = { 5,	0,		SEN(printargs),			"osf_kmodcall"		}, /* not implemented */
[ 78] = { 5,	TM,		SEN(printargs),			"osf_mincore"		}, /* not implemented */
[ 79] = { 2,	0,		SEN(getgroups),			"getgroups"		},
[ 80] = { 2,	0,		SEN(setgroups),			"setgroups"		},
[ 81] = { 5,	0,		SEN(printargs),			"osf_old_getpgrp"	}, /* not implemented */
[ 82] = { 2,	0,		SEN(setpgrp),			"setpgrp"		},
[ 83] = { 3,	0,		SEN(osf_setitimer),		"osf_setitimer"		},
[ 84] = { 5,	0,		SEN(printargs),			"osf_old_wait"		}, /* not implemented */
[ 85] = { 5,	0,		SEN(printargs),			"osf_table"		}, /* not implemented */
[ 86] = { 2,	0,		SEN(osf_getitimer),		"osf_getitimer"		},
[ 87] = { 2,	0,		SEN(gethostname),		"gethostname"		},
[ 88] = { 2,	0,		SEN(sethostname),		"sethostname"		},
[ 89] = { 0,	0,		SEN(getdtablesize),		"getdtablesize"		},
[ 90] = { 2,	TD,		SEN(dup2),			"dup2"			},
[ 91] = { 2,	TD|TFST|TSTA,	SEN(fstat),			"fstat"			},
[ 92] = { 3,	TD,		SEN(fcntl),			"fcntl"			},
[ 93] = { 5,	TD,		SEN(osf_select),		"osf_select"		},
[ 94] = { 3,	TD,		SEN(poll),			"poll"			},
[ 95] = { 1,	TD,		SEN(fsync),			"fsync"			},
[ 96] = { 3,	0,		SEN(setpriority),		"setpriority"		},
[ 97] = { 3,	TN,		SEN(socket),			"socket"		},
[ 98] = { 3,	TN,		SEN(connect),			"connect"		},
[ 99] = { 3,	TN,		SEN(accept),			"accept"		},
[100] = { 2,	0,		SEN(getpriority),		"getpriority"		},
[101] = { 4,	TN,		SEN(send),			"send"			},
[102] = { 4,	TN,		SEN(recv),			"recv"			},
[103] = { 0,	TS,		SEN(sigreturn),			"sigreturn"		},
[104] = { 3,	TN,		SEN(bind),			"bind"			},
[105] = { 5,	TN,		SEN(setsockopt),		"setsockopt"		},
[106] = { 2,	TN,		SEN(listen),			"listen"		},
[107] = { 5,	0,		SEN(printargs),			"osf_plock"		}, /* not implemented */
[108] = { 5,	0,		SEN(printargs),			"osf_old_sigvec"	}, /* not implemented */
[109] = { 5,	0,		SEN(printargs),			"osf_old_sigblock"	}, /* not implemented */
[110] = { 5,	0,		SEN(printargs),			"osf_old_sigsetmask"	}, /* not implemented */
[111] = { 1,	TS,		SEN(sigsuspend),		"sigsuspend"		},
[112] = { 2,	0,		SEN(printargs),			"osf_sigstack"		},
[113] = { 3,	TN,		SEN(recvmsg),			"recvmsg"		},
[114] = { 3,	TN,		SEN(sendmsg),			"sendmsg"		},
[115] = { 5,	0,		SEN(printargs),			"osf_old_vtrace"	}, /* not implemented */
[116] = { 2,	0,		SEN(osf_gettimeofday),		"osf_gettimeofday"	},
[117] = { 2,	0,		SEN(osf_getrusage),		"osf_getrusage"		},
[118] = { 5,	TN,		SEN(getsockopt),		"getsockopt"		},
[119] = { },
[120] = { 3,	TD,		SEN(readv),			"readv"			},
[121] = { 3,	TD,		SEN(writev),			"writev"		},
[122] = { 2,	0,		SEN(osf_settimeofday),		"osf_settimeofday"	},
[123] = { 3,	TD,		SEN(fchown),			"fchown"		},
[124] = { 2,	TD,		SEN(fchmod),			"fchmod"		},
[125] = { 6,	TN,		SEN(recvfrom),			"recvfrom"		},
[126] = { 2,	0,		SEN(setreuid),			"setreuid"		},
[127] = { 2,	0,		SEN(setregid),			"setregid"		},
[128] = { 2,	TF,		SEN(rename),			"rename"		},
[129] = { 2,	TF,		SEN(truncate),			"truncate"		},
[130] = { 2,	TD,		SEN(ftruncate),			"ftruncate"		},
[131] = { 2,	TD,		SEN(flock),			"flock"			},
[132] = { 1,	0,		SEN(setgid),			"setgid"		},
[133] = { 6,	TN,		SEN(sendto),			"sendto"		},
[134] = { 2,	TN,		SEN(shutdown),			"shutdown"		},
[135] = { 4,	TN,		SEN(socketpair),		"socketpair"		},
[136] = { 2,	TF,		SEN(mkdir),			"mkdir"			},
[137] = { 1,	TF,		SEN(rmdir),			"rmdir"			},
[138] = { 2,	TF,		SEN(osf_utimes),		"osf_utimes"		},
[139] = { 5,	0,		SEN(printargs),			"osf_old_sigreturn"	}, /* not implemented */
[140] = { 5,	0,		SEN(printargs),			"osf_adjtime"		}, /* not implemented */
[141] = { 3,	TN,		SEN(getpeername),		"getpeername"		},
[142] = { 5,	0,		SEN(printargs),			"osf_gethostid"		}, /* not implemented */
[143] = { 5,	0,		SEN(printargs),			"osf_sethostid"		}, /* not implemented */
[144] = { 2,	0,		SEN(getrlimit),			"getrlimit"		},
[145] = { 2,	0,		SEN(setrlimit),			"setrlimit"		},
[146] = { 5,	0,		SEN(printargs),			"osf_old_killpg"	}, /* not implemented */
[147] = { 0,	0,		SEN(setsid),			"setsid"		},
[148] = { 4,	TF,		SEN(quotactl),			"quotactl"		},
[149] = { 5,	0,		SEN(printargs),			"osf_oldquota"		}, /* not implemented */
[150] = { 3,	TN,		SEN(getsockname),		"getsockname"		},
[151 ... 152] = { },
[153] = { 5,	0,		SEN(printargs),			"osf_pid_block"		}, /* not implemented */
[154] = { 5,	0,		SEN(printargs),			"osf_pid_unblock"	}, /* not implemented */
[155] = { },
[156] = { 3,	TS,		SEN(sigaction),			"sigaction"		},
[157] = { 5,	0,		SEN(printargs),			"osf_sigwaitprim"	}, /* not implemented */
[158] = { 5,	0,		SEN(printargs),			"osf_nfssvc"		}, /* not implemented */
[159] = { 4,	0,		SEN(printargs),			"osf_getdirentries"	},
[160] = { 3,	TF|TSF|TSFA,	SEN(osf_statfs),		"osf_statfs"		},
[161] = { 3,	TD|TFSF|TSFA,	SEN(osf_fstatfs),		"osf_fstatfs"		},
[162] = { },
[163] = { 5,	0,		SEN(printargs),			"osf_asynch_daemon"	}, /* not implemented */
[164] = { 5,	0,		SEN(printargs),			"osf_getfh"		}, /* not implemented */
[165] = { 2,	0,		SEN(printargs),			"osf_getdomainname"	},
[166] = { 2,	0,		SEN(setdomainname),		"setdomainname"		},
[167 ... 168] = { },
[169] = { 5,	0,		SEN(printargs),			"osf_exportfs"		}, /* not implemented */
[170 ... 180] = { },
[181] = { 5,	0,		SEN(printargs),			"osf_alt_plock"		}, /* not implemented */
[182 ... 183] = { },
[184] = { 5,	0,		SEN(printargs),			"osf_getmnt"		}, /* not implemented */
[185 ... 186] = { },
[187] = { 5,	0,		SEN(printargs),			"osf_alt_sigpending"	}, /* not implemented */
[188] = { 5,	0,		SEN(printargs),			"osf_alt_setsid"	}, /* not implemented */
[189 ... 198] = { },
[199] = { 4,	0,		SEN(printargs),			"osf_swapon"		},
[200] = { 3,	TI,		SEN(msgctl),			"msgctl"		},
[201] = { 2,	TI,		SEN(msgget),			"msgget"		},
[202] = { 5,	TI,		SEN(msgrcv),			"msgrcv"		},
[203] = { 4,	TI,		SEN(msgsnd),			"msgsnd"		},
[204] = { 4,	TI,		SEN(semctl),			"semctl"		},
[205] = { 3,	TI,		SEN(semget),			"semget"		},
[206] = { 3,	TI,		SEN(semop),			"semop"			},
[207] = { 1,	0,		SEN(printargs),			"osf_utsname"		},
[208] = { 3,	TF,		SEN(chown),			"lchown"		},
[209] = { 3,	TI|TM|SI,	SEN(shmat),			"osf_shmat"		},
[210] = { 3,	TI,		SEN(shmctl),			"shmctl"		},
[211] = { 1,	TI|TM|SI,	SEN(shmdt),			"shmdt"			},
[212] = { 3,	TI,		SEN(shmget),			"shmget"		},
[213] = { 5,	0,		SEN(printargs),			"osf_mvalid"		}, /* not implemented */
[214] = { 5,	0,		SEN(printargs),			"osf_getaddressconf"	}, /* not implemented */
[215] = { 5,	0,		SEN(printargs),			"osf_msleep"		}, /* not implemented */
[216] = { 5,	0,		SEN(printargs),			"osf_mwakeup"		}, /* not implemented */
[217] = { 3,	TM,		SEN(msync),			"msync"			},
[218] = { 5,	0,		SEN(printargs),			"osf_signal"		}, /* not implemented */
[219] = { 5,	0,		SEN(printargs),			"osf_utc_gettime"	}, /* not implemented */
[220] = { 5,	0,		SEN(printargs),			"osf_utc_adjtime"	}, /* not implemented */
[221] = { },
[222] = { 5,	0,		SEN(printargs),			"osf_security"		}, /* not implemented */
[223] = { 5,	0,		SEN(printargs),			"osf_kloadcall"		}, /* not implemented */
[224] = { 2,	TF|TST|TSTA,	SEN(printargs),			"osf_stat"		},
[225] = { 2,	TF|TLST|TSTA,	SEN(printargs),			"osf_lstat"		},
[226] = { 2,	TD|TFST|TSTA,	SEN(printargs),			"osf_fstat"		},
[227] = { 3,	TF|TSF|TSFA,	SEN(osf_statfs),		"osf_statfs64"		},
[228] = { 3,	TD|TFSF|TSFA,	SEN(osf_fstatfs),		"osf_fstatfs64"		},
[229 ... 232] = { },
[233] = { 1,	0,		SEN(getpgid),			"getpgid"		},
[234] = { 1,	0,		SEN(getsid),			"getsid"		},
[235] = { 2,	TS,		SEN(sigaltstack),		"sigaltstack"		},
[236] = { 5,	0,		SEN(printargs),			"osf_waitid"		}, /* not implemented */
[237] = { 5,	0,		SEN(printargs),			"osf_priocntlset"	}, /* not implemented */
[238] = { 5,	0,		SEN(printargs),			"osf_sigsendset"	}, /* not implemented */
[239] = { 5,	0,		SEN(printargs),			"osf_set_speculative"	}, /* not implemented */
[240] = { 5,	0,		SEN(printargs),			"osf_msfs_syscall"	}, /* not implemented */
[241] = { 3,	0,		SEN(printargs),			"osf_sysinfo"		},
[242] = { 5,	0,		SEN(printargs),			"osf_uadmin"		}, /* not implemented */
[243] = { 5,	0,		SEN(printargs),			"osf_fuser"		}, /* not implemented */
[244] = { 2,	0,		SEN(printargs),			"osf_proplist_syscall"	},
[245] = { 5,	0,		SEN(printargs),			"osf_ntp_adjtime"	}, /* not implemented */
[246] = { 5,	0,		SEN(printargs),			"osf_ntp_gettime"	}, /* not implemented */
[247] = { 5,	0,		SEN(printargs),			"osf_pathconf"		}, /* not implemented */
[248] = { 5,	0,		SEN(printargs),			"osf_fpathconf"		}, /* not implemented */
[249] = { },
[250] = { 5,	0,		SEN(printargs),			"osf_uswitch"		}, /* not implemented */
[251] = { 2,	0,		SEN(printargs),			"osf_usleep_thread"	},
[252] = { 5,	0,		SEN(printargs),			"osf_audcntl"		}, /* not implemented */
[253] = { 5,	0,		SEN(printargs),			"osf_audgen"		}, /* not implemented */
[254] = { 3,	0,		SEN(sysfs),			"sysfs"			},
[255] = { 5,	0,		SEN(printargs),			"osf_subsys_info"	}, /* not implemented */
[256] = { 5,	0,		SEN(printargs),			"osf_getsysinfo"	},
[257] = { 5,	0,		SEN(printargs),			"osf_setsysinfo"	},
[258] = { 5,	0,		SEN(printargs),			"osf_afs_syscall"	}, /* not implemented */
[259] = { 5,	0,		SEN(printargs),			"osf_swapctl"		}, /* not implemented */
[260] = { 5,	0,		SEN(printargs),			"osf_memcntl"		}, /* not implemented */
[261] = { 5,	0,		SEN(printargs),			"osf_fdatasync"		}, /* not implemented */
[262 ... 299] = { },
[300] = { 2,	0,		SEN(bdflush),			"bdflush"		},
[301] = { 1,	0,		SEN(printargs),			"sethae"		},
[302] = { 5,	TF,		SEN(mount),			"mount"			},
[303] = { 1,	0,		SEN(adjtimex),			"old_adjtimex"		},
[304] = { 1,	TF,		SEN(swapoff),			"swapoff"		},
[305] = { 3,	TD,		SEN(getdents),			"getdents"		},
[306] = { 2,	0,		SEN(create_module),		"create_module"		}, /* not implemented */
[307] = { 3,	0,		SEN(init_module),		"init_module"		},
[308] = { 2,	0,		SEN(delete_module),		"delete_module"		},
[309] = { 1,	0,		SEN(get_kernel_syms),		"get_kernel_syms"	}, /* not implemented */
[310] = { 3,	0,		SEN(syslog),			"syslog"		},
[311] = { 4,	0,		SEN(reboot),			"reboot"		},
[312] = { 5,	TP,		SEN(clone),			"clone"			},
[313] = { 1,	TF,		SEN(uselib),			"uselib"		},
[314] = { 2,	TM,		SEN(mlock),			"mlock"			},
[315] = { 2,	TM,		SEN(munlock),			"munlock"		},
[316] = { 1,	TM,		SEN(mlockall),			"mlockall"		},
[317] = { 0,	TM,		SEN(munlockall),		"munlockall"		},
[318] = { 1,	0,		SEN(sysinfo),			"sysinfo"		},
[319] = { 1,	0,		SEN(sysctl),			"_sysctl"		},
[320] = { },
[321] = { 1,	TF,		SEN(umount),			"oldumount"		},
[322] = { 2,	TF,		SEN(swapon),			"swapon"		},
[323] = { 1,	0,		SEN(times),			"times"			},
[324] = { 1,	NF,		SEN(personality),		"personality"		},
[325] = { 1,	NF,		SEN(setfsuid),			"setfsuid"		},
[326] = { 1,	NF,		SEN(setfsgid),			"setfsgid"		},
[327] = { 2,	TSFA,		SEN(ustat),			"ustat"			},
[328] = { 2,	TF|TSF|TSFA,	SEN(statfs),			"statfs"		},
[329] = { 2,	TD|TFSF|TSFA,	SEN(fstatfs),			"fstatfs"		},
[330] = { 2,	0,		SEN(sched_setparam),		"sched_setparam"	},
[331] = { 2,	0,		SEN(sched_getparam),		"sched_getparam"	},
[332] = { 3,	0,		SEN(sched_setscheduler),	"sched_setscheduler"	},
[333] = { 1,	0,		SEN(sched_getscheduler),	"sched_getscheduler"	},
[334] = { 0,	0,		SEN(sched_yield),		"sched_yield"		},
[335] = { 1,	0,		SEN(sched_get_priority_max),	"sched_get_priority_max"},
[336] = { 1,	0,		SEN(sched_get_priority_min),	"sched_get_priority_min"},
[337] = { 2,	0,		SEN(sched_rr_get_interval),	"sched_rr_get_interval"	},
[338] = { 5,	0,		SEN(afs_syscall),		"afs_syscall"		}, /* not implemented */
[339] = { 1,	0,		SEN(uname),			"uname"			},
[340] = { 2,	0,		SEN(nanosleep),			"nanosleep"		},
[341] = { 5,	TM|SI,		SEN(mremap),			"mremap"		},
[342] = { 3,	0,		SEN(nfsservctl),		"nfsservctl"		}, /* not implemented */
[343] = { 3,	0,		SEN(setresuid),			"setresuid"		},
[344] = { 3,	0,		SEN(getresuid),			"getresuid"		},
[345] = { 5,	0,		SEN(printargs),			"pciconfig_read"	},
[346] = { 5,	0,		SEN(printargs),			"pciconfig_write"	},
[347] = { 5,	0,		SEN(query_module),		"query_module"		}, /* not implemented */
[348] = { 5,	0,		SEN(prctl),			"prctl"			},
[349] = { 4,	TD,		SEN(pread),			"pread64"		},
[350] = { 4,	TD,		SEN(pwrite),			"pwrite64"		},
[351] = { 0,	TS,		SEN(rt_sigreturn),		"rt_sigreturn"		},
[352] = { 5,	TS,		SEN(rt_sigaction),		"rt_sigaction"		},
[353] = { 4,	TS,		SEN(rt_sigprocmask),		"rt_sigprocmask"	},
[354] = { 2,	TS,		SEN(rt_sigpending),		"rt_sigpending"		},
[355] = { 4,	TS,		SEN(rt_sigtimedwait),		"rt_sigtimedwait"	},
[356] = { 3,	TS,		SEN(rt_sigqueueinfo),		"rt_sigqueueinfo"	},
[357] = { 2,	TS,		SEN(rt_sigsuspend),		"rt_sigsuspend"		},
[358] = { 5,	TD,		SEN(select),			"select"		},
[359] = { 2,	0,		SEN(gettimeofday),		"gettimeofday"		},
[360] = { 2,	0,		SEN(settimeofday),		"settimeofday"		},
[361] = { 2,	0,		SEN(getitimer),			"getitimer"		},
[362] = { 3,	0,		SEN(setitimer),			"setitimer"		},
[363] = { 2,	TF,		SEN(utimes),			"utimes"		},
[364] = { 2,	0,		SEN(getrusage),			"getrusage"		},
[365] = { 4,	TP,		SEN(wait4),			"wait4"			},
[366] = { 1,	0,		SEN(adjtimex),			"adjtimex"		},
[367] = { 2,	TF,		SEN(getcwd),			"getcwd"		},
[368] = { 2,	0,		SEN(capget),			"capget"		},
[369] = { 2,	0,		SEN(capset),			"capset"		},
[370] = { 4,	TD|TN,		SEN(sendfile),			"sendfile"		},
[371] = { 3,	0,		SEN(setresgid),			"setresgid"		},
[372] = { 3,	0,		SEN(getresgid),			"getresgid"		},
[373] = { 4,	0,		SEN(printargs),			"dipc"			}, /* not implemented */
[374] = { 2,	TF,		SEN(pivotroot),			"pivot_root"		},
[375] = { 3,	TM,		SEN(mincore),			"mincore"		},
[376] = { 3,	0,		SEN(printargs),			"pciconfig_iobase"	},
[377] = { 3,	TD,		SEN(getdents64),		"getdents64"		},
[378] = { 0,	NF,		SEN(gettid),			"gettid"		},
[379] = { 3,	TD,		SEN(readahead),			"readahead"		},
[380] = { },
[381] = { 2,	TS,		SEN(kill),			"tkill"			},
[382] = { 5,	TF,		SEN(setxattr),			"setxattr"		},
[383] = { 5,	TF,		SEN(setxattr),			"lsetxattr"		},
[384] = { 5,	TD,		SEN(fsetxattr),			"fsetxattr"		},
[385] = { 4,	TF,		SEN(getxattr),			"getxattr"		},
[386] = { 4,	TF,		SEN(getxattr),			"lgetxattr"		},
[387] = { 4,	TD,		SEN(fgetxattr),			"fgetxattr"		},
[388] = { 3,	TF,		SEN(listxattr),			"listxattr"		},
[389] = { 3,	TF,		SEN(listxattr),			"llistxattr"		},
[390] = { 3,	TD,		SEN(flistxattr),		"flistxattr"		},
[391] = { 2,	TF,		SEN(removexattr),		"removexattr"		},
[392] = { 2,	TF,		SEN(removexattr),		"lremovexattr"		},
[393] = { 2,	TD,		SEN(fremovexattr),		"fremovexattr"		},
[394] = { 6,	0,		SEN(futex),			"futex"			},
[395] = { 3,	0,		SEN(sched_setaffinity),		"sched_setaffinity"	},
[396] = { 3,	0,		SEN(sched_getaffinity),		"sched_getaffinity"	},
[397] = { 5,	0,		SEN(tuxcall),			"tuxcall"		}, /* not implemented */
[398] = { 2,	TM,		SEN(io_setup),			"io_setup"		},
[399] = { 1,	TM,		SEN(io_destroy),		"io_destroy"		},
[400] = { 5,	0,		SEN(io_getevents),		"io_getevents"		},
[401] = { 3,	0,		SEN(io_submit),			"io_submit"		},
[402] = { 3,	0,		SEN(io_cancel),			"io_cancel"		},
[403 ... 404] = { },
[405] = { 1,	TP|SE,		SEN(exit),			"exit_group"		},
[406] = { 3,	0,		SEN(lookup_dcookie),		"lookup_dcookie"	},
[407] = { 1,	TD,		SEN(epoll_create),		"epoll_create"		},
[408] = { 4,	TD,		SEN(epoll_ctl),			"epoll_ctl"		},
[409] = { 4,	TD,		SEN(epoll_wait),		"epoll_wait"		},
[410] = { 5,	TM|SI,		SEN(remap_file_pages),		"remap_file_pages"	},
[411] = { 1,	0,		SEN(set_tid_address),		"set_tid_address"	},
[412] = { 0,	0,		SEN(restart_syscall),		"restart_syscall"	},
[413] = { 4,	TD,		SEN(fadvise64),			"fadvise64"		},
[414] = { 3,	0,		SEN(timer_create),		"timer_create"		},
[415] = { 4,	0,		SEN(timer_settime),		"timer_settime"		},
[416] = { 2,	0,		SEN(timer_gettime),		"timer_gettime"		},
[417] = { 1,	0,		SEN(timer_getoverrun),		"timer_getoverrun"	},
[418] = { 1,	0,		SEN(timer_delete),		"timer_delete"		},
[419] = { 2,	0,		SEN(clock_settime),		"clock_settime"		},
[420] = { 2,	0,		SEN(clock_gettime),		"clock_gettime"		},
[421] = { 2,	0,		SEN(clock_getres),		"clock_getres"		},
[422] = { 4,	0,		SEN(clock_nanosleep),		"clock_nanosleep"	},
[423] = { 4,	TI,		SEN(semtimedop),		"semtimedop"		},
[424] = { 3,	TS,		SEN(tgkill),			"tgkill"		},
[425] = { 2,	TF|TST|TSTA,	SEN(stat64),			"stat64"		},
[426] = { 2,	TF|TLST|TSTA,	SEN(lstat64),			"lstat64"		},
[427] = { 2,	TD|TFST|TSTA,	SEN(fstat64),			"fstat64"		},
[428] = { 5,	0,		SEN(vserver),			"vserver"		}, /* not implemented */
[429] = { 6,	TM,		SEN(mbind),			"mbind"			}, /* not implemented */
[430] = { 5,	TM,		SEN(get_mempolicy),		"get_mempolicy"		}, /* not implemented */
[431] = { 3,	TM,		SEN(set_mempolicy),		"set_mempolicy"		}, /* not implemented */
[432] = { 4,	0,		SEN(mq_open),			"mq_open"		},
[433] = { 1,	0,		SEN(mq_unlink),			"mq_unlink"		},
[434] = { 5,	0,		SEN(mq_timedsend),		"mq_timedsend"		},
[435] = { 5,	0,		SEN(mq_timedreceive),		"mq_timedreceive"	},
[436] = { 2,	0,		SEN(mq_notify),			"mq_notify"		},
[437] = { 3,	0,		SEN(mq_getsetattr),		"mq_getsetattr"		},
[438] = { 5,	TP,		SEN(waitid),			"waitid"		},
[439] = { 5,	0,		SEN(add_key),			"add_key"		},
[440] = { 4,	0,		SEN(request_key),		"request_key"		},
[441] = { 5,	0,		SEN(keyctl),			"keyctl"		},
[442] = { 3,	0,		SEN(ioprio_set),		"ioprio_set"		},
[443] = { 2,	0,		SEN(ioprio_get),		"ioprio_get"		},
[444] = { 0,	TD,		SEN(inotify_init),		"inotify_init"		},
[445] = { 3,	TD|TF,		SEN(inotify_add_watch),		"inotify_add_watch"	},
[446] = { 2,	TD,		SEN(inotify_rm_watch),		"inotify_rm_watch"	},
[447] = { 1,	TD,		SEN(fdatasync),			"fdatasync"		},
[448] = { 4,	0,		SEN(kexec_load),		"kexec_load"		},
[449] = { 4,	TM,		SEN(migrate_pages),		"migrate_pages"		},
[450] = { 4,	TD|TF,		SEN(openat),			"openat"		},
[451] = { 3,	TD|TF,		SEN(mkdirat),			"mkdirat"		},
[452] = { 4,	TD|TF,		SEN(mknodat),			"mknodat"		},
[453] = { 5,	TD|TF,		SEN(fchownat),			"fchownat"		},
[454] = { 3,	TD|TF,		SEN(futimesat),			"futimesat"		},
[455] = { 4,	TD|TF|TFST|TSTA,SEN(fstatat64),			"fstatat64"		},
[456] = { 3,	TD|TF,		SEN(unlinkat),			"unlinkat"		},
[457] = { 4,	TD|TF,		SEN(renameat),			"renameat"		},
[458] = { 5,	TD|TF,		SEN(linkat),			"linkat"		},
[459] = { 3,	TD|TF,		SEN(symlinkat),			"symlinkat"		},
[460] = { 4,	TD|TF,		SEN(readlinkat),		"readlinkat"		},
[461] = { 3,	TD|TF,		SEN(fchmodat),			"fchmodat"		},
[462] = { 3,	TD|TF,		SEN(faccessat),			"faccessat"		},
[463] = { 6,	TD,		SEN(pselect6),			"pselect6"		},
[464] = { 5,	TD,		SEN(ppoll),			"ppoll"			},
[465] = { 1,	TP,		SEN(unshare),			"unshare"		},
[466] = { 2,	0,		SEN(set_robust_list),		"set_robust_list"	},
[467] = { 3,	0,		SEN(get_robust_list),		"get_robust_list"	},
[468] = { 6,	TD,		SEN(splice),			"splice"		},
[469] = { 4,	TD,		SEN(sync_file_range),		"sync_file_range"	},
[470] = { 4,	TD,		SEN(tee),			"tee"			},
[471] = { 4,	TD,		SEN(vmsplice),			"vmsplice"		},
[472] = { 6,	TM,		SEN(move_pages),		"move_pages"		},
[473] = { 3,	0,		SEN(getcpu),			"getcpu"		},
[474] = { 6,	TD,		SEN(epoll_pwait),		"epoll_pwait"		},
[475] = { 4,	TD|TF,		SEN(utimensat),			"utimensat"		},
[476] = { 3,	TD|TS,		SEN(signalfd),			"signalfd"		},
[477] = { 4,	TD,		SEN(timerfd),			"timerfd"		}, /* not implemented */
[478] = { 1,	TD,		SEN(eventfd),			"eventfd"		},
[479] = { 5,	TN,		SEN(recvmmsg),			"recvmmsg"		},
[480] = { 4,	TD,		SEN(fallocate),			"fallocate"		},
[481] = { 2,	TD,		SEN(timerfd_create),		"timerfd_create"	},
[482] = { 4,	TD,		SEN(timerfd_settime),		"timerfd_settime"	},
[483] = { 2,	TD,		SEN(timerfd_gettime),		"timerfd_gettime"	},
[484] = { 4,	TD|TS,		SEN(signalfd4),			"signalfd4"		},
[485] = { 2,	TD,		SEN(eventfd2),			"eventfd2"		},
[486] = { 1,	TD,		SEN(epoll_create1),		"epoll_create1"		},
[487] = { 3,	TD,		SEN(dup3),			"dup3"			},
[488] = { 2,	TD,		SEN(pipe2),			"pipe2"			},
[489] = { 1,	TD,		SEN(inotify_init1),		"inotify_init1"		},
[490] = { 4,	TD,		SEN(preadv),			"preadv"		},
[491] = { 4,	TD,		SEN(pwritev),			"pwritev"		},
[492] = { 4,	TP|TS,		SEN(rt_tgsigqueueinfo),		"rt_tgsigqueueinfo"	},
[493] = { 5,	TD,		SEN(perf_event_open),		"perf_event_open"	},
[494] = { 2,	TD,		SEN(fanotify_init),		"fanotify_init"		},
[495] = { 5,	TD|TF,		SEN(fanotify_mark),		"fanotify_mark"		},
[496] = { 4,	0,		SEN(prlimit64),			"prlimit64"		},
[497] = { 5,	TD|TF,		SEN(name_to_handle_at),		"name_to_handle_at"	},
[498] = { 3,	TD,		SEN(open_by_handle_at),		"open_by_handle_at"	},
[499] = { 2,	0,		SEN(clock_adjtime),		"clock_adjtime"		},
[500] = { 1,	TD,		SEN(syncfs),			"syncfs"		},
[501] = { 2,	TD,		SEN(setns),			"setns"			},
[502] = { 4,	TN,		SEN(accept4),			"accept4"		},
[503] = { 4,	TN,		SEN(sendmmsg),			"sendmmsg"		},
[504] = { 6,	0,		SEN(process_vm_readv),		"process_vm_readv"	},
[505] = { 6,	0,		SEN(process_vm_writev),		"process_vm_writev"	},
[506] = { 5,	0,		SEN(kcmp),			"kcmp"			},
[507] = { 3,	TD,		SEN(finit_module),		"finit_module"		},
[508] = { 3,	0,		SEN(sched_setattr),		"sched_setattr"		},
[509] = { 4,	0,		SEN(sched_getattr),		"sched_getattr"		},
[510] = { 5,	TD|TF,		SEN(renameat2),			"renameat2"		},
[511] = { 3,	0,		SEN(getrandom),			"getrandom"		},
[512] = { 2,	TD,		SEN(memfd_create),		"memfd_create"		},
[513] = { 5,	TD|TF|TP|SE|SI,	SEN(execveat),			"execveat"		},
[514] = { 3,	0,		SEN(seccomp),			"seccomp"		},
[515] = { 3,	TD,		SEN(bpf),			"bpf"			},
[516] = { 1,	TD,		SEN(userfaultfd),		"userfaultfd"		},
[517] = { 2,	0,		SEN(membarrier),		"membarrier"		},
[518] = { 3,	TM,		SEN(mlock2),			"mlock2"		},
[519] = { 6,	TD,		SEN(copy_file_range),		"copy_file_range"	},
[520] = { 6,	TD,		SEN(preadv2),			"preadv2"		},
[521] = { 6,	TD,		SEN(pwritev2),			"pwritev2"		},
[522] = { 5,	TD|TF|TSTA,	SEN(statx),			"statx"			},

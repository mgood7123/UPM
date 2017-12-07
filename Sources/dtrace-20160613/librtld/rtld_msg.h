/*
#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#

#
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

# Message file for cmd/sgs/librtld_db.

@ MSG_ID_LIBRTLD_DB

# There presently seem little point in internationalizing these messages
# as they provide debugging information for dbx developers and shouldn't
# be see by anyone else.

@ _END_
*/

// Basic strings

#define MSG_DB_NOLDDATA	"rtld_db: rr: no LDDATA found in aux vector - \
			 falling back to symbol lookup"
#define MSG_DB_FLDDATA	"rtld_db: rl: found LDDATA auxv ld.so.1 data seg \
			 at: 0x%p"
#define MSG_DB_SYMRDEBUG	"rtld_db: rr: found ld.so.1::rdebug address: 0x%llx"
#define MSG_DB_LOOKFAIL	"rtld_db: rr: lookup of %s failed"
#define MSG_DB_NOAUXV		"rtld_db: rr: call to ps_pauxv() failed"
#define MSG_DB_DMLOOKFAIL	"rtld_db: rr: lookup of data model failed"
#define MSG_DB_NODYNAMIC	"rtld_db: rr: unable to find _DYNAMIC in exec"
#define MSG_DB_INITFAILED	"rtld_db: rr: can't find r_debug, rtld_db init failed"
#define MSG_DB_RESETFAIL	"rtld_db: rn: rd_reset failed"
#define MSG_DB_LKMAPFAIL	"rtld_db: im: failed in reading link_maps"
#define MSG_DB_CALLBACKR0	"rtld_db: im: itermap() callback returned 0 - \
			 terminating link-map traversal"
#define MSG_DB_ITERMAP	"rtld_db: im: itermap_cb(cb=0x%p, data=0x%p,\n\
			 rtld_db: im:      objbase=0x%llx, ident=0x%llx)"
#define MSG_DB_READDBGFAIL_1	"rtld_db: rli: failed to read rtld_db_priv: 0x%llx"
#define MSG_DB_READDBGFAIL_2	"rtld_db: reg: failed to read rdebug: 0x%llx"
#define MSG_DB_READDBGFAIL_3	"rtld_db: rli: failed to read rtd_dynlmlst: 0x%llx"
#define MSG_DB_READDBGFAIL_4	"rtld_db: rli: failed to read dynlm_list: 0x%llx"
#define MSG_DB_READDBGFAIL_5	"rtld_db: rli: failed to read dynlm_list->ap_data[]: \
			 0x%llx"
#define MSG_DB_READDBGFAIL_6	"rtld_db: rli: failed to read Lm_list: 0x%llx"
#define MSG_DB_READPRIVFAIL_1	"rtld_db: rr: fail to read rtld_db_priv: 0x%llx"
#define MSG_DB_BADPVERS	"rtld_db: ve: rtld vs. rtld_db version mismatch: \
			 %d != %d"
#define MSG_DB_VALIDRDEBUG	"rtld_db: ve: found valid r_debug structure. \
			 Addr: 0x%llx\n\
			 rtld_db: ve:	rtld_dbVers: %d rtldVers: %d \
			 Corefile: %d"
#define MSG_DB_LKMAPNOINIT	"rtld_db: rli: link maps are not yet initialized: \
			 rtd_dynlmlst: 0x%llx"
#define MSG_DB_LKMAPNOINIT_1	"rtld_db: rli: dynlm_list is not yet initialized: \
			 dynlm_list: 0x%llx"
#define MSG_DB_NULLITER	"rtld_db: rli: called with null iterator"
#define MSG_DB_UNEXPEVENT	"rtld_db: rea: unexpected event num: %d"
#define MSG_DB_READFAIL_1	"rtld_db: vr: read of 0x%llx failed"
#define MSG_DB_READFAIL_2	"rtld_db: rpr: read of 0x%llx failed"
#define MSG_DB_READFAIL_3	"rtld_db: roe: read of 0x%llx failed"
#define MSG_DB_READFAIL_4	"rtld_db: fde: read of 0x%llx failed"
#define MSG_DB_READFAIL_5	"rtld_db: ge: read of 0x%llx failed"
#define MSG_DB_READFAIL_6	"rtld_db: gd: read of 0x%llx failed"
#define MSG_DB_WRITEFAIL_1	"rtld_db: ree: write of 0x%llx failed"
#define MSG_DB_WRITEFAIL_2	"rtld_db: roe: write of 0x%llx failed"
#define MSG_DB_UNFNDSYM	"rtld_db: rbea: unable to find sym: %s"
#define MSG_DB_NODYNDEBUG	"rtld_db: fde: no %lld found in .dynamic"
#define MSG_DB_FINDDYNAMIC	"rtld_db: fde: DYNAMIC entry found tag: %d found. \
			 val: 0x%llx"
#define MSG_DB_HELPER_PREFIX	"/usr/lib/brand"



// Diagnostic messages

#define MSG_DB_LOGENABLE	"rtld_db: logging enabled!"
#define MSG_DB_RDINIT		"rtld_db: rd_init(%d)"
#define MSG_DB_RDNEW		"rtld_db: rd_new(0x%p)"
#define MSG_DB_RDDELETE	"rtld_db: rd_delete(0x%p)"
#define MSG_DB_LOADOBJITER	"rtld_db: rd_loadobj_iter32(dmodel=%d, cb=0x%p, \
			 d=0x%p)"
#define MSG_DB_RDEVENTADDR	"rtld_db: rd_event_addr(event=%d, addr=0x%llx)"
#define MSG_DB_RDRESET	"rtld_db: rd_reset(dmodel=%d)"
#define MSG_DB_RDEVENTENABLE	"rtld_db: rd_event_enable(dmodel=%d, onoff=%d)"
#define MSG_DB_RDEVENTGETMSG	"rtld_db: rd_event_getmsg(dmodel=%d, type=%d, \
			 state=%d)"
#define MSG_DB_RDOBJPADE	"rtld_db: rd_objpad_enable(padsize=0x%llx)"
#define MSG_DB_64BIT_PREFIX	"64/"
#define MSG_DB_BRAND_HELPERPATH_PREFIX "%s/%s/%s/%s%s_librtld_db.so.1"
#define MSG_DB_BRAND_HELPERPATH "%s/%s/%s%s_librtld_db.so.1"
#define MSG_DB_HELPERNOOPS	"rtld_db: helper lib loaded but ops not preset"
#define MSG_DB_HELPERLOADED	"rtld_db: helper library loaded for brand \"%s\""
#define MSG_DB_HELPERLOADFAILED "rtld_db: couldn't load brand helper library %s"
#define MSG_DB_HELPERINITFAILED "rtld_db: brand helper library initialization failed"

#define MSG_ER_OK		"no error"
#define MSG_ER_ERR		"generic rtld_db.so error"
#define MSG_ER_DBERR		"debugger service failed"
#define MSG_ER_NOCAPAB	"capability not available"
#define MSG_ER_NODYNAM	"couldn't find '_DYNAMIC'"
#define MSG_ER_NOBASE		"couldn't find auxv tag 'AT_BASE'"
#define MSG_ER_NOMAPS		"link-maps are not initialized"
#define MSG_ER_DEFAULT	"unknown rtld_db.so error code"

#define MSG_SYM_DEBUG		"r_debug"
#define MSG_SYM_PREINIT	"rtld_db_preinit"
#define MSG_SYM_POSTINIT	"rtld_db_postinit"
#define MSG_SYM_DLACT		"rtld_db_dlactivity"
#define MSG_SYM_RTBIND	"elf_rtbndr"
#define MSG_SYM_DYNAMIC	"_DYNAMIC"
#define MSG_SYM_BRANDOPS_32	"rtld_db_brand_ops32"
#define MSG_SYM_BRANDOPS_64	"rtld_db_brand_ops64"

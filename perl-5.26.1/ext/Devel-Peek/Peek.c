/*
 * This file was generated automatically by ExtUtils::ParseXS version 3.34 from the
 * contents of Peek.xs. Do not edit this file, edit Peek.xs instead.
 *
 *    ANY CHANGES MADE HERE WILL BE LOST!
 *
 */

#line 1 "Peek.xs"
#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

static bool
_runops_debug(int flag)
{
    dTHX;
    const bool d = PL_runops == Perl_runops_debug;

    if (flag >= 0)
	PL_runops = flag ? Perl_runops_debug : Perl_runops_standard;
    return d;
}

static SV *
DeadCode(pTHX)
{
#ifdef PURIFY
    return Nullsv;
#else
    SV* sva;
    SV* sv;
    SV* ret = newRV_noinc((SV*)newAV());
    SV* svend;
    int tm = 0, tref = 0, ts = 0, ta = 0, tas = 0;

    for (sva = PL_sv_arenaroot; sva; sva = (SV*)SvANY(sva)) {
	svend = &sva[SvREFCNT(sva)];
	for (sv = sva + 1; sv < svend; ++sv) {
	    if (SvTYPE(sv) == SVt_PVCV) {
		CV *cv = (CV*)sv;
		PADLIST* padlist;
                AV *argav;
		SV** svp;
		SV** pad;
		int i = 0, j, levelm, totm = 0, levelref, totref = 0;
		int levels, tots = 0, levela, tota = 0, levelas, totas = 0;
		int dumpit = 0;

		if (CvISXSUB(sv)) {
		    continue;		/* XSUB */
		}
		if (!CvGV(sv)) {
		    continue;		/* file-level scope. */
		}
		if (!CvROOT(cv)) {
		    /* PerlIO_printf(Perl_debug_log, "  no root?!\n"); */
		    continue;		/* autoloading stub. */
		}
		do_gvgv_dump(0, Perl_debug_log, "GVGV::GV", CvGV(sv));
		if (CvDEPTH(cv)) {
		    PerlIO_printf(Perl_debug_log, "  busy\n");
		    continue;
		}
		padlist = CvPADLIST(cv);
		svp = (SV**) PadlistARRAY(padlist);
		while (++i <= PadlistMAX(padlist)) { /* Depth. */
		    SV **args;
		    
		    if (!svp[i]) continue;
		    pad = AvARRAY((AV*)svp[i]);
		    argav = (AV*)pad[0];
		    if (!argav || (SV*)argav == &PL_sv_undef) {
			PerlIO_printf(Perl_debug_log, "    closure-template\n");
			continue;
		    }
		    args = AvARRAY(argav);
		    levelm = levels = levelref = levelas = 0;
		    levela = sizeof(SV*) * (AvMAX(argav) + 1);
		    if (AvREAL(argav)) {
			for (j = 0; j < AvFILL(argav); j++) {
			    if (SvROK(args[j])) {
				PerlIO_printf(Perl_debug_log, "     ref in args!\n");
				levelref++;
			    }
			    /* else if (SvPOK(args[j]) && SvPVX(args[j])) { */
			    else if (SvTYPE(args[j]) >= SVt_PV && SvLEN(args[j])) {
				levelas += SvLEN(args[j])/SvREFCNT(args[j]);
			    }
			}
		    }
		    for (j = 1; j < AvFILL((AV*)svp[1]); j++) {	/* Vars. */
			if (!pad[j]) continue;
			if (SvROK(pad[j])) {
			    levelref++;
			    do_sv_dump(0, Perl_debug_log, pad[j], 0, 4, 0, 0);
			    dumpit = 1;
			}
			/* else if (SvPOK(pad[j]) && SvPVX(pad[j])) { */
			else if (SvTYPE(pad[j]) >= SVt_PVAV) {
			    if (!SvPADMY(pad[j])) {
				levelref++;
				do_sv_dump(0, Perl_debug_log, pad[j], 0, 4, 0, 0);
				dumpit = 1;
			    }
			}
			else if (SvTYPE(pad[j]) >= SVt_PV && SvLEN(pad[j])) {
			    levels++;
			    levelm += SvLEN(pad[j])/SvREFCNT(pad[j]);
				/* Dump(pad[j],4); */
			}
		    }
		    PerlIO_printf(Perl_debug_log, "    level %i: refs: %i, strings: %i in %i,\targsarray: %i, argsstrings: %i\n", 
			    i, levelref, levelm, levels, levela, levelas);
		    totm += levelm;
		    tota += levela;
		    totas += levelas;
		    tots += levels;
		    totref += levelref;
		    if (dumpit)
			do_sv_dump(0, Perl_debug_log, (SV*)cv, 0, 2, 0, 0);
		}
		if (PadlistMAX(padlist) > 1) {
		    PerlIO_printf(Perl_debug_log, "  total: refs: %i, strings: %i in %i,\targsarrays: %i, argsstrings: %i\n", 
			    totref, totm, tots, tota, totas);
		}
		tref += totref;
		tm += totm;
		ts += tots;
		ta += tota;
		tas += totas;
	    }
	}
    }
    PerlIO_printf(Perl_debug_log, "total: refs: %i, strings: %i in %i\targsarray: %i, argsstrings: %i\n", tref, tm, ts, ta, tas);

    return ret;
#endif /* !PURIFY */
}

#if defined(MYMALLOC)
#   define mstat(str) dump_mstats(str)
#else
#   define mstat(str) \
	PerlIO_printf(Perl_debug_log, "%s: perl not compiled with MYMALLOC\n",str);
#endif

#if defined(MYMALLOC)

/* Very coarse overestimate, 2-per-power-of-2, one more to determine NBUCKETS. */
#  define _NBUCKETS (2*8*IVSIZE+1)

struct mstats_buffer 
{
    perl_mstats_t buffer;
    UV buf[_NBUCKETS*4];
};

static void
_fill_mstats(struct mstats_buffer *b, int level)
{
    dTHX;
    b->buffer.nfree  = b->buf;
    b->buffer.ntotal = b->buf + _NBUCKETS;
    b->buffer.bucket_mem_size = b->buf + 2*_NBUCKETS;
    b->buffer.bucket_available_size = b->buf + 3*_NBUCKETS;
    Zero(b->buf, (level ? 4*_NBUCKETS: 2*_NBUCKETS), unsigned long);
    get_mstats(&(b->buffer), _NBUCKETS, level);
}

static void
fill_mstats(SV *sv, int level)
{
    dTHX;

    if (SvREADONLY(sv))
	croak("Cannot modify a readonly value");
    sv_grow(sv, sizeof(struct mstats_buffer)+1);
    _fill_mstats((struct mstats_buffer*)SvPVX(sv),level);
    SvCUR_set(sv, sizeof(struct mstats_buffer));
    *SvEND(sv) = '\0';
    SvPOK_only(sv);
}

static void
_mstats_to_hv(HV *hv, const struct mstats_buffer *b, int level)
{
    dTHX;
    SV **svp;
    int type;

    svp = hv_fetchs(hv, "topbucket", 1);
    sv_setiv(*svp, b->buffer.topbucket);

    svp = hv_fetchs(hv, "topbucket_ev", 1);
    sv_setiv(*svp, b->buffer.topbucket_ev);

    svp = hv_fetchs(hv, "topbucket_odd", 1);
    sv_setiv(*svp, b->buffer.topbucket_odd);

    svp = hv_fetchs(hv, "totfree", 1);
    sv_setiv(*svp, b->buffer.totfree);

    svp = hv_fetchs(hv, "total", 1);
    sv_setiv(*svp, b->buffer.total);

    svp = hv_fetchs(hv, "total_chain", 1);
    sv_setiv(*svp, b->buffer.total_chain);

    svp = hv_fetchs(hv, "total_sbrk", 1);
    sv_setiv(*svp, b->buffer.total_sbrk);

    svp = hv_fetchs(hv, "sbrks", 1);
    sv_setiv(*svp, b->buffer.sbrks);

    svp = hv_fetchs(hv, "sbrk_good", 1);
    sv_setiv(*svp, b->buffer.sbrk_good);

    svp = hv_fetchs(hv, "sbrk_slack", 1);
    sv_setiv(*svp, b->buffer.sbrk_slack);

    svp = hv_fetchs(hv, "start_slack", 1);
    sv_setiv(*svp, b->buffer.start_slack);

    svp = hv_fetchs(hv, "sbrked_remains", 1);
    sv_setiv(*svp, b->buffer.sbrked_remains);
    
    svp = hv_fetchs(hv, "minbucket", 1);
    sv_setiv(*svp, b->buffer.minbucket);
    
    svp = hv_fetchs(hv, "nbuckets", 1);
    sv_setiv(*svp, b->buffer.nbuckets);

    if (_NBUCKETS < b->buffer.nbuckets) 
	warn("FIXME: internal mstats buffer too short");
    
    for (type = 0; type < (level ? 4 : 2); type++) {
	UV *p = 0, *p1 = 0, i;
	AV *av;
	static const char *types[4] = { 
	    "free", "used", "mem_size", "available_size"    
	};

	svp = hv_fetch(hv, types[type], strlen(types[type]), 1);

	if (SvOK(*svp) && !(SvROK(*svp) && SvTYPE(SvRV(*svp)) == SVt_PVAV))
	    croak("Unexpected value for the key '%s' in the mstats hash", types[type]);
	if (!SvOK(*svp)) {
	    av = newAV();
	    (void)SvUPGRADE(*svp, SVt_RV);
	    SvRV_set(*svp, (SV*)av);
	    SvROK_on(*svp);
	} else
	    av = (AV*)SvRV(*svp);

	av_extend(av, b->buffer.nbuckets - 1);
	/* XXXX What is the official way to reduce the size of the array? */
	switch (type) {
	case 0:
	    p = b->buffer.nfree;
	    break;
	case 1:
	    p = b->buffer.ntotal;
	    p1 = b->buffer.nfree;
	    break;
	case 2:
	    p = b->buffer.bucket_mem_size;
	    break;
	case 3:
	    p = b->buffer.bucket_available_size;
	    break;
	}
	for (i = 0; i < b->buffer.nbuckets; i++) {
	    svp = av_fetch(av, i, 1);
	    if (type == 1)
		sv_setiv(*svp, p[i]-p1[i]);
	    else
		sv_setuv(*svp, p[i]);
	}
    }
}

static void
mstats_fillhash(SV *sv, int level)
{
    struct mstats_buffer buf;

    if (!(SvROK(sv) && SvTYPE(SvRV(sv)) == SVt_PVHV))
	croak("Not a hash reference");
    _fill_mstats(&buf, level);
    _mstats_to_hv((HV *)SvRV(sv), &buf, level);
}

static void
mstats2hash(SV *sv, SV *rv, int level)
{
    if (!(SvROK(rv) && SvTYPE(SvRV(rv)) == SVt_PVHV))
	croak("Not a hash reference");
    if (!SvPOK(sv))
	croak("Undefined value when expecting mstats buffer");
    if (SvCUR(sv) != sizeof(struct mstats_buffer))
	croak("Wrong size for a value with a mstats buffer");
    _mstats_to_hv((HV *)SvRV(rv), (struct mstats_buffer*)SvPVX(sv), level);
}
#else	/* defined(MYMALLOC) */ 
static void
fill_mstats(SV *sv, int level)
{
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(level);
    croak("Cannot report mstats without Perl malloc");
}

static void
mstats_fillhash(SV *sv, int level)
{
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(level);
    croak("Cannot report mstats without Perl malloc");
}

static void
mstats2hash(SV *sv, SV *rv, int level)
{
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(rv);
    PERL_UNUSED_ARG(level);
    croak("Cannot report mstats without Perl malloc");
}
#endif	/* defined(MYMALLOC) */ 

#define _CvGV(cv)					\
	(SvROK(cv) && (SvTYPE(SvRV(cv))==SVt_PVCV)	\
	 ? SvREFCNT_inc(CvGV((CV*)SvRV(cv))) : &PL_sv_undef)

static void
S_do_dump(pTHX_ SV *const sv, I32 lim)
{
    dVAR;
    SV *pv_lim_sv = perl_get_sv("Devel::Peek::pv_limit", 0);
    const STRLEN pv_lim = pv_lim_sv ? SvIV(pv_lim_sv) : 0;
    SV *dumpop = perl_get_sv("Devel::Peek::dump_ops", 0);
    const U16 save_dumpindent = PL_dumpindent;
    PL_dumpindent = 2;
    do_sv_dump(0, Perl_debug_log, sv, 0, lim,
	       (bool)(dumpop && SvTRUE(dumpop)), pv_lim);
    PL_dumpindent = save_dumpindent;
}

static OP *
S_pp_dump(pTHX)
{
    dSP;
    const I32 lim = PL_op->op_private == 2 ? (I32)POPi : 4;
    dPOPss;
    S_do_dump(aTHX_ sv, lim);
    RETPUSHUNDEF;
}

static OP *
S_ck_dump(pTHX_ OP *entersubop, GV *namegv, SV *cv)
{
    OP *parent, *pm, *first, *second;
    BINOP *newop;

    PERL_UNUSED_ARG(cv);

    ck_entersub_args_proto(entersubop, namegv,
			   newSVpvn_flags("$;$", 3, SVs_TEMP));

    parent = entersubop;
    pm = cUNOPx(entersubop)->op_first;
    if (!OpHAS_SIBLING(pm)) {
        parent = pm;
	pm = cUNOPx(pm)->op_first;
    }
    first = OpSIBLING(pm);
    second = OpSIBLING(first);
    if (!second) {
	/* It doesn’t really matter what we return here, as this only
	   occurs after yyerror.  */
	return entersubop;
    }
    /* we either have Dump($x):   [pushmark]->[first]->[ex-cvop]
     * or             Dump($x,1); [pushmark]->[first]->[second]->[ex-cvop]
     */
    if (!OpHAS_SIBLING(second))
        second = NULL;

    if (first->op_type == OP_RV2AV ||
	first->op_type == OP_PADAV ||
	first->op_type == OP_RV2HV ||
	first->op_type == OP_PADHV
    )
	first->op_flags |= OPf_REF;
    else
	first->op_flags &= ~OPf_MOD;

    /* splice out first (and optionally second) ops, then discard the rest
     * of the op tree */

    op_sibling_splice(parent, pm, second ? 2 : 1, NULL);
    op_free(entersubop);

    /* then attach first (and second) to a new binop */

    NewOp(1234, newop, 1, BINOP);
    newop->op_type   = OP_CUSTOM;
    newop->op_ppaddr = S_pp_dump;
    newop->op_private= second ? 2 : 1;
    newop->op_flags  = OPf_KIDS|OPf_WANT_SCALAR;
    op_sibling_splice((OP*)newop, NULL, 0, first);

    return (OP *)newop;
}

static const XOP my_xop = {
    XOPf_xop_name|XOPf_xop_desc|XOPf_xop_class,		/* xop_flags */
    "Devel_Peek_Dump",					/* xop_name */
    "Dump",						/* xop_desc */
    OA_BINOP,						/* xop_class */
    NULL						/* xop_peep */
};

#line 427 "Peek.c"
#ifndef PERL_UNUSED_VAR
#  define PERL_UNUSED_VAR(var) if (0) var = var
#endif

#ifndef dVAR
#  define dVAR		dNOOP
#endif


/* This stuff is not part of the API! You have been warned. */
#ifndef PERL_VERSION_DECIMAL
#  define PERL_VERSION_DECIMAL(r,v,s) (r*1000000 + v*1000 + s)
#endif
#ifndef PERL_DECIMAL_VERSION
#  define PERL_DECIMAL_VERSION \
	  PERL_VERSION_DECIMAL(PERL_REVISION,PERL_VERSION,PERL_SUBVERSION)
#endif
#ifndef PERL_VERSION_GE
#  define PERL_VERSION_GE(r,v,s) \
	  (PERL_DECIMAL_VERSION >= PERL_VERSION_DECIMAL(r,v,s))
#endif
#ifndef PERL_VERSION_LE
#  define PERL_VERSION_LE(r,v,s) \
	  (PERL_DECIMAL_VERSION <= PERL_VERSION_DECIMAL(r,v,s))
#endif

/* XS_INTERNAL is the explicit static-linkage variant of the default
 * XS macro.
 *
 * XS_EXTERNAL is the same as XS_INTERNAL except it does not include
 * "STATIC", ie. it exports XSUB symbols. You probably don't want that
 * for anything but the BOOT XSUB.
 *
 * See XSUB.h in core!
 */


/* TODO: This might be compatible further back than 5.10.0. */
#if PERL_VERSION_GE(5, 10, 0) && PERL_VERSION_LE(5, 15, 1)
#  undef XS_EXTERNAL
#  undef XS_INTERNAL
#  if defined(__CYGWIN__) && defined(USE_DYNAMIC_LOADING)
#    define XS_EXTERNAL(name) __declspec(dllexport) XSPROTO(name)
#    define XS_INTERNAL(name) STATIC XSPROTO(name)
#  endif
#  if defined(__SYMBIAN32__)
#    define XS_EXTERNAL(name) EXPORT_C XSPROTO(name)
#    define XS_INTERNAL(name) EXPORT_C STATIC XSPROTO(name)
#  endif
#  ifndef XS_EXTERNAL
#    if defined(HASATTRIBUTE_UNUSED) && !defined(__cplusplus)
#      define XS_EXTERNAL(name) void name(pTHX_ CV* cv __attribute__unused__)
#      define XS_INTERNAL(name) STATIC void name(pTHX_ CV* cv __attribute__unused__)
#    else
#      ifdef __cplusplus
#        define XS_EXTERNAL(name) extern "C" XSPROTO(name)
#        define XS_INTERNAL(name) static XSPROTO(name)
#      else
#        define XS_EXTERNAL(name) XSPROTO(name)
#        define XS_INTERNAL(name) STATIC XSPROTO(name)
#      endif
#    endif
#  endif
#endif

/* perl >= 5.10.0 && perl <= 5.15.1 */


/* The XS_EXTERNAL macro is used for functions that must not be static
 * like the boot XSUB of a module. If perl didn't have an XS_EXTERNAL
 * macro defined, the best we can do is assume XS is the same.
 * Dito for XS_INTERNAL.
 */
#ifndef XS_EXTERNAL
#  define XS_EXTERNAL(name) XS(name)
#endif
#ifndef XS_INTERNAL
#  define XS_INTERNAL(name) XS(name)
#endif

/* Now, finally, after all this mess, we want an ExtUtils::ParseXS
 * internal macro that we're free to redefine for varying linkage due
 * to the EXPORT_XSUB_SYMBOLS XS keyword. This is internal, use
 * XS_EXTERNAL(name) or XS_INTERNAL(name) in your code if you need to!
 */

#undef XS_EUPXS
#if defined(PERL_EUPXS_ALWAYS_EXPORT)
#  define XS_EUPXS(name) XS_EXTERNAL(name)
#else
   /* default to internal */
#  define XS_EUPXS(name) XS_INTERNAL(name)
#endif

#ifndef PERL_ARGS_ASSERT_CROAK_XS_USAGE
#define PERL_ARGS_ASSERT_CROAK_XS_USAGE assert(cv); assert(params)

/* prototype to pass -Wmissing-prototypes */
STATIC void
S_croak_xs_usage(const CV *const cv, const char *const params);

STATIC void
S_croak_xs_usage(const CV *const cv, const char *const params)
{
    const GV *const gv = CvGV(cv);

    PERL_ARGS_ASSERT_CROAK_XS_USAGE;

    if (gv) {
        const char *const gvname = GvNAME(gv);
        const HV *const stash = GvSTASH(gv);
        const char *const hvname = stash ? HvNAME(stash) : NULL;

        if (hvname)
	    Perl_croak_nocontext("Usage: %s::%s(%s)", hvname, gvname, params);
        else
	    Perl_croak_nocontext("Usage: %s(%s)", gvname, params);
    } else {
        /* Pants. I don't think that it should be possible to get here. */
	Perl_croak_nocontext("Usage: CODE(0x%" UVxf ")(%s)", PTR2UV(cv), params);
    }
}
#undef  PERL_ARGS_ASSERT_CROAK_XS_USAGE

#define croak_xs_usage        S_croak_xs_usage

#endif

/* NOTE: the prototype of newXSproto() is different in versions of perls,
 * so we define a portable version of newXSproto()
 */
#ifdef newXS_flags
#define newXSproto_portable(name, c_impl, file, proto) newXS_flags(name, c_impl, file, proto, 0)
#else
#define newXSproto_portable(name, c_impl, file, proto) (PL_Sv=(SV*)newXS(name, c_impl, file), sv_setpv(PL_Sv, proto), (CV*)PL_Sv)
#endif /* !defined(newXS_flags) */

#if PERL_VERSION_LE(5, 21, 5)
#  define newXS_deffile(a,b) Perl_newXS(aTHX_ a,b,file)
#else
#  define newXS_deffile(a,b) Perl_newXS_deffile(aTHX_ a,b)
#endif

#line 571 "Peek.c"

XS_EUPXS(XS_Devel__Peek_mstat); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Devel__Peek_mstat)
{
    dVAR; dXSARGS;
    if (items < 0 || items > 1)
       croak_xs_usage(cv,  "str=\"Devel::Peek::mstat: \"");
    {
	const char *	str;

	if (items < 1)
	    str = "Devel::Peek::mstat: ";
	else {
	    str = (const char *)SvPV_nolen(ST(0))
;
	}

	mstat(str);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Devel__Peek_fill_mstats); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Devel__Peek_fill_mstats)
{
    dVAR; dXSARGS;
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "sv, level= 0");
    {
	SV *	sv = ST(0)
;
	int	level;

	if (items < 2)
	    level = 0;
	else {
	    level = (int)SvIV(ST(1))
;
	}

	fill_mstats(sv, level);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Devel__Peek_mstats_fillhash); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Devel__Peek_mstats_fillhash)
{
    dVAR; dXSARGS;
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "sv, level= 0");
    {
	SV *	sv = ST(0)
;
	int	level;

	if (items < 2)
	    level = 0;
	else {
	    level = (int)SvIV(ST(1))
;
	}

	mstats_fillhash(sv, level);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Devel__Peek_mstats2hash); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Devel__Peek_mstats2hash)
{
    dVAR; dXSARGS;
    if (items < 2 || items > 3)
       croak_xs_usage(cv,  "sv, rv, level= 0");
    {
	SV *	sv = ST(0)
;
	SV *	rv = ST(1)
;
	int	level;

	if (items < 3)
	    level = 0;
	else {
	    level = (int)SvIV(ST(2))
;
	}

	mstats2hash(sv, rv, level);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Devel__Peek_Dump); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Devel__Peek_Dump)
{
    dVAR; dXSARGS;
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "sv, lim=4");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	SV *	sv = ST(0)
;
	I32	lim;

	if (items < 2)
	    lim = 4;
	else {
	    lim = (I32)SvIV(ST(1))
;
	}
#line 439 "Peek.xs"
{
    S_do_dump(aTHX_ sv, lim);
}
#line 692 "Peek.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Devel__Peek_DumpArray); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Devel__Peek_DumpArray)
{
    dVAR; dXSARGS;
    if (items < 1)
       croak_xs_usage(cv,  "lim, ...");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	I32	lim = (I32)SvIV(ST(0))
;
#line 455 "Peek.xs"
{
    long i;
    SV *pv_lim_sv = perl_get_sv("Devel::Peek::pv_limit", 0);
    const STRLEN pv_lim = pv_lim_sv ? SvIV(pv_lim_sv) : 0;
    SV *dumpop = perl_get_sv("Devel::Peek::dump_ops", 0);
    const U16 save_dumpindent = PL_dumpindent;
    PL_dumpindent = 2;

    for (i=1; i<items; i++) {
	PerlIO_printf(Perl_debug_log, "Elt No. %ld  0x%" UVxf "\n", i - 1, PTR2UV(ST(i)));
	do_sv_dump(0, Perl_debug_log, ST(i), 0, lim,
		   (bool)(dumpop && SvTRUE(dumpop)), pv_lim);
    }
    PL_dumpindent = save_dumpindent;
}
#line 726 "Peek.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Devel__Peek_DumpProg); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Devel__Peek_DumpProg)
{
    dVAR; dXSARGS;
    if (items != 0)
       croak_xs_usage(cv,  "");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
#line 474 "Peek.xs"
{
    warn("dumpindent is %d", (int)PL_dumpindent);
    if (PL_main_root)
	op_dump(PL_main_root);
}
#line 748 "Peek.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Devel__Peek_SvREFCNT); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Devel__Peek_SvREFCNT)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "sv");
    {
	SV *	sv = ST(0)
;
	U32	RETVAL;
	dXSTARG;
#line 485 "Peek.xs"
    SvGETMAGIC(sv);
    if (!SvROK(sv))
        croak_xs_usage(cv, "SCALAR");
    RETVAL = SvREFCNT(SvRV(sv)) - 1; /* -1 because our ref doesn't count */
#line 771 "Peek.c"
	XSprePUSH; PUSHu((UV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Devel__Peek_DeadCode); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Devel__Peek_DeadCode)
{
    dVAR; dXSARGS;
    if (items != 0)
       croak_xs_usage(cv,  "");
    {
	SV *	RETVAL;
#line 495 "Peek.xs"
    RETVAL = DeadCode(aTHX);
#line 788 "Peek.c"
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Devel__Peek_CvGV); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Devel__Peek_CvGV)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "cv");
    {
	SV *	cv = ST(0)
;
	SV *	RETVAL;

	RETVAL = _CvGV(cv);
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Devel__Peek_runops_debug); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Devel__Peek_runops_debug)
{
    dVAR; dXSARGS;
    if (items < 0 || items > 1)
       croak_xs_usage(cv,  "flag= -1");
    {
	bool	RETVAL;
	int	flag;

	if (items < 1)
	    flag = -1;
	else {
	    flag = (int)SvIV(ST(0))
;
	}

	RETVAL = _runops_debug(flag);
	ST(0) = boolSV(RETVAL);
    }
    XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif
XS_EXTERNAL(boot_Devel__Peek); /* prototype to pass -Wmissing-prototypes */
XS_EXTERNAL(boot_Devel__Peek)
{
#if PERL_VERSION_LE(5, 21, 5)
    dVAR; dXSARGS;
#else
    dVAR; dXSBOOTARGSXSAPIVERCHK;
#endif
#if (PERL_REVISION == 5 && PERL_VERSION < 9)
    char* file = __FILE__;
#else
    const char* file = __FILE__;
#endif

    PERL_UNUSED_VAR(file);

    PERL_UNUSED_VAR(cv); /* -W */
    PERL_UNUSED_VAR(items); /* -W */
#if PERL_VERSION_LE(5, 21, 5)
    XS_VERSION_BOOTCHECK;
#  ifdef XS_APIVERSION_BOOTCHECK
    XS_APIVERSION_BOOTCHECK;
#  endif
#endif

        newXS_deffile("Devel::Peek::mstat", XS_Devel__Peek_mstat);
        newXS_deffile("Devel::Peek::fill_mstats", XS_Devel__Peek_fill_mstats);
        (void)newXSproto_portable("Devel::Peek::mstats_fillhash", XS_Devel__Peek_mstats_fillhash, file, "\\%;$");
        (void)newXSproto_portable("Devel::Peek::mstats2hash", XS_Devel__Peek_mstats2hash, file, "$\\%;$");
        newXS_deffile("Devel::Peek::Dump", XS_Devel__Peek_Dump);
        newXS_deffile("Devel::Peek::DumpArray", XS_Devel__Peek_DumpArray);
        newXS_deffile("Devel::Peek::DumpProg", XS_Devel__Peek_DumpProg);
        (void)newXSproto_portable("Devel::Peek::SvREFCNT", XS_Devel__Peek_SvREFCNT, file, "\\[$@%&*]");
        newXS_deffile("Devel::Peek::DeadCode", XS_Devel__Peek_DeadCode);
        newXS_deffile("Devel::Peek::CvGV", XS_Devel__Peek_CvGV);
        newXS_deffile("Devel::Peek::runops_debug", XS_Devel__Peek_runops_debug);

    /* Initialisation Section */

#line 444 "Peek.xs"
{
    CV * const cv = get_cvn_flags("Devel::Peek::Dump", 17, 0);
    assert(cv);
    cv_set_call_checker(cv, S_ck_dump, (SV *)cv);
    Perl_custom_op_register(aTHX_ S_pp_dump, &my_xop);
}

#line 888 "Peek.c"

    /* End of Initialisation Section */

#if PERL_VERSION_LE(5, 21, 5)
#  if PERL_VERSION_GE(5, 9, 0)
    if (PL_unitcheckav)
        call_list(PL_scopestack_ix, PL_unitcheckav);
#  endif
    XSRETURN_YES;
#else
    Perl_xs_boot_epilog(aTHX_ ax);
#endif
}


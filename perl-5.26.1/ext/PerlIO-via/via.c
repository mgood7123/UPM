/*
 * This file was generated automatically by ExtUtils::ParseXS version 3.34 from the
 * contents of via.xs. Do not edit this file, edit via.xs instead.
 *
 *    ANY CHANGES MADE HERE WILL BE LOST!
 *
 */

#line 1 "via.xs"
#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef PERLIO_LAYERS

#include "perliol.h"

typedef struct
{
 struct _PerlIO base;       /* Base "class" info */
 HV *		stash;
 SV *		obj;
 SV *		var;
 SSize_t	cnt;
 IO *		io;
 SV *		fh;
 CV *PUSHED;
 CV *POPPED;
 CV *OPEN;
 CV *FDOPEN;
 CV *SYSOPEN;
 CV *GETARG;
 CV *FILENO;
 CV *READ;
 CV *WRITE;
 CV *FILL;
 CV *CLOSE;
 CV *SEEK;
 CV *TELL;
 CV *UNREAD;
 CV *FLUSH;
 CV *SETLINEBUF;
 CV *CLEARERR;
 CV *mERROR;
 CV *mEOF;
 CV *BINMODE;
 CV *UTF8;
} PerlIOVia;

#define MYMethod(x) #x,&s->x

static CV *
PerlIOVia_fetchmethod(pTHX_ PerlIOVia * s, const char *method, CV ** save)
{
    GV *gv = gv_fetchmeth(s->stash, method, strlen(method), 0);
#if 0
    Perl_warn(aTHX_ "Lookup %s::%s => %p", HvNAME_get(s->stash), method, gv);
#endif
    if (gv) {
	return *save = GvCV(gv);
    }
    else {
	return *save = (CV *) - 1;
    }
}

/*
 * Try and call method, possibly via cached lookup.
 * If method does not exist return Nullsv (caller may fallback to another approach
 * If method does exist call it with flags passing variable number of args
 * Last arg is a "filehandle" to layer below (if present)
 * Returns scalar returned by method (if any) otherwise sv_undef
 */

static SV *
PerlIOVia_method(pTHX_ PerlIO * f, const char *method, CV ** save, int flags,
		 ...)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    SV *result = Nullsv;
    CV *cv =
	(*save) ? *save : PerlIOVia_fetchmethod(aTHX_ s, method, save);
    if (cv != (CV *) - 1) {
	IV count;
	dSP;
	SV *arg;
        va_list ap;

        va_start(ap, flags);
	PUSHSTACKi(PERLSI_MAGIC);
	ENTER;
	PUSHMARK(sp);
	XPUSHs(s->obj);
	while ((arg = va_arg(ap, SV *))) {
	    XPUSHs(arg);
	}
        va_end(ap);
	if (*PerlIONext(f)) {
	    if (!s->fh) {
		GV *gv;
		char *package = HvNAME_get(s->stash);

                if (!package)
                    return Nullsv; /* can this ever happen? */
		gv = newGVgen(package);
		GvIOp(gv) = newIO();
		s->fh = newRV((SV *) gv);
		s->io = GvIOp(gv);
		if (gv) {
		    /* shamelessly stolen from IO::File's new_tmpfile() */
		    (void) hv_delete(GvSTASH(gv), GvNAME(gv), GvNAMELEN(gv), G_DISCARD);
		}
	    }
	    IoIFP(s->io) = PerlIONext(f);
	    IoOFP(s->io) = PerlIONext(f);
	    XPUSHs(s->fh);
	}
	else {
	    PerlIO_debug("No next\n");
	    /* FIXME: How should this work for OPEN etc? */
	}
	PUTBACK;
	count = call_sv((SV *) cv, flags);
	if (count) {
	    SPAGAIN;
	    result = POPs;
	    PUTBACK;
	}
	else {
	    result = &PL_sv_undef;
	}
	LEAVE;
	POPSTACK;
    }
    return result;
}

static IV
PerlIOVia_pushed(pTHX_ PerlIO * f, const char *mode, SV * arg,
		 PerlIO_funcs * tab)
{
    IV code = PerlIOBase_pushed(aTHX_ f, mode, Nullsv, tab);
    if (code == 0) {
	PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
	if (!arg) {
	    if (ckWARN(WARN_LAYER))
		Perl_warner(aTHX_ packWARN(WARN_LAYER),
			    "No package specified");
	    errno = EINVAL;
	    code = -1;
	}
	else {
	    STRLEN pkglen = 0;
	    const char *pkg = SvPV(arg, pkglen);
	    s->obj =
		newSVpvn(Perl_form(aTHX_ "PerlIO::via::%s", pkg),
			 pkglen + 13);
	    s->stash = gv_stashpvn(SvPVX_const(s->obj), pkglen + 13, 0);
	    if (!s->stash) {
		SvREFCNT_dec(s->obj);
		s->obj = SvREFCNT_inc(arg);
		s->stash = gv_stashpvn(pkg, pkglen, 0);
	    }
	    if (s->stash) {
		char lmode[8];
		SV *modesv;
		SV *result;
		if (!mode) {
		    /* binmode() passes NULL - so find out what mode is */
		    mode = PerlIO_modestr(f,lmode);
		}
		modesv = newSVpvn_flags(mode, strlen(mode), SVs_TEMP);
		result = PerlIOVia_method(aTHX_ f, MYMethod(PUSHED), G_SCALAR,
				     modesv, Nullsv);
		if (result) {
		    if (sv_isobject(result)) {
			SvREFCNT_dec(s->obj);
			s->obj = SvREFCNT_inc(result);
		    }
		    else if (SvIV(result) != 0)
			return SvIV(result);
		}
		else {
		    goto push_failed;
		}
		modesv = (*PerlIONext(f) && (PerlIOBase(PerlIONext(f))->flags & PERLIO_F_UTF8))
                           ? &PL_sv_yes : &PL_sv_no;
		result = PerlIOVia_method(aTHX_ f, MYMethod(UTF8), G_SCALAR, modesv, Nullsv);
		if (result && SvTRUE(result)) {
		    PerlIOBase(f)->flags |= PERLIO_F_UTF8;
		}
		else {
		    PerlIOBase(f)->flags &= ~PERLIO_F_UTF8;
		}
		if (PerlIOVia_fetchmethod(aTHX_ s, MYMethod(FILL)) ==
		    (CV *) - 1)
		    PerlIOBase(f)->flags &= ~PERLIO_F_FASTGETS;
		else
		    PerlIOBase(f)->flags |= PERLIO_F_FASTGETS;
	    }
	    else {
		if (ckWARN(WARN_LAYER))
		    Perl_warner(aTHX_ packWARN(WARN_LAYER),
				"Cannot find package '%.*s'", (int) pkglen,
				pkg);
push_failed:
#ifdef ENOSYS
		errno = ENOSYS;
#else
#ifdef ENOENT
		errno = ENOENT;
#endif
#endif
		code = -1;
	    }
	}
    }
    return code;
}

static PerlIO *
PerlIOVia_open(pTHX_ PerlIO_funcs * self, PerlIO_list_t * layers,
	       IV n, const char *mode, int fd, int imode, int perm,
	       PerlIO * f, int narg, SV ** args)
{
    if (!f) {
	f = PerlIO_push(aTHX_ PerlIO_allocate(aTHX), self, mode,
			PerlIOArg);
    }
    else {
	/* Reopen */
	if (!PerlIO_push(aTHX_ f, self, mode, PerlIOArg))
	    return NULL;
    }
    if (f) {
	PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
	SV *result = Nullsv;
	if (fd >= 0) {
	    SV *fdsv = sv_2mortal(newSViv(fd));
	    result =
		PerlIOVia_method(aTHX_ f, MYMethod(FDOPEN), G_SCALAR, fdsv,
				 Nullsv);
	}
	else if (narg > 0) {
	    if (*mode == '#') {
		SV *imodesv = sv_2mortal(newSViv(imode));
		SV *permsv = sv_2mortal(newSViv(perm));
		result =
		    PerlIOVia_method(aTHX_ f, MYMethod(SYSOPEN), G_SCALAR,
				     *args, imodesv, permsv, Nullsv);
	    }
	    else {
		result =
		    PerlIOVia_method(aTHX_ f, MYMethod(OPEN), G_SCALAR,
				     *args, Nullsv);
	    }
	}
	if (result) {
	    if (sv_isobject(result))
		s->obj = SvREFCNT_inc(result);
	    else if (!SvTRUE(result)) {
		return NULL;
	    }
	}
	else {
	    /* Required open method not present */
	    PerlIO_funcs *tab = NULL;
	    IV m = n - 1;
	    while (m >= 0) {
		PerlIO_funcs *t =
		    PerlIO_layer_fetch(aTHX_ layers, m, NULL);
		if (t && t->Open) {
		    tab = t;
		    break;
		}
		m--;
	    }
	    if (tab) {
		if ((*tab->Open) (aTHX_ tab, layers, m, mode, fd, imode,
				  perm, PerlIONext(f), narg, args)) {
		    PerlIO_debug("Opened with %s => %p->%p\n", tab->name,
				 PerlIONext(f), *PerlIONext(f));
		    if (m + 1 < n) {
			/*
			 * More layers above the one that we used to open -
			 * apply them now
			 */
			if (PerlIO_apply_layera
			    (aTHX_ PerlIONext(f), mode, layers, m + 1,
			     n) != 0) {
			    /* If pushing layers fails close the file */
			    PerlIO_close(f);
			    f = NULL;
			}
		    }
		    /* FIXME - Call an OPENED method here ? */
		    return f;
		}
		else {
		    PerlIO_debug("Open fail %s => %p->%p\n", tab->name,
				 PerlIONext(f), *PerlIONext(f));
		    /* Sub-layer open failed */
		}
	    }
	    else {
	         PerlIO_debug("Nothing to open with");
		/* Nothing to do the open */
	    }
	    PerlIO_pop(aTHX_ f);
	    return NULL;
	}
    }
    return f;
}

static IV
PerlIOVia_popped(pTHX_ PerlIO * f)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    PerlIOVia_method(aTHX_ f, MYMethod(POPPED), G_VOID, Nullsv);
    if (s->var) {
	SvREFCNT_dec(s->var);
	s->var = Nullsv;
    }

    if (s->io) {
	IoIFP(s->io) = NULL;
	IoOFP(s->io) = NULL;
    }
    if (s->fh) {
	SvREFCNT_dec(s->fh);
	s->fh = Nullsv;
	s->io = NULL;
    }
    if (s->obj) {
	SvREFCNT_dec(s->obj);
	s->obj = Nullsv;
    }
    return 0;
}

static IV
PerlIOVia_close(pTHX_ PerlIO * f)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    IV code = PerlIOBase_close(aTHX_ f);
    SV *result =
	PerlIOVia_method(aTHX_ f, MYMethod(CLOSE), G_SCALAR, Nullsv);
    if (result && SvIV(result) != 0)
	code = SvIV(result);
    PerlIOBase(f)->flags &= ~(PERLIO_F_RDBUF | PERLIO_F_WRBUF);
    return code;
}

static IV
PerlIOVia_fileno(pTHX_ PerlIO * f)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    SV *result =
	PerlIOVia_method(aTHX_ f, MYMethod(FILENO), G_SCALAR, Nullsv);
    return (result) ? SvIV(result) : PerlIO_fileno(PerlIONext(f));
}

static IV
PerlIOVia_binmode(pTHX_ PerlIO * f)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    SV *result =
	PerlIOVia_method(aTHX_ f, MYMethod(BINMODE), G_SCALAR, Nullsv);
    if (!result || !SvOK(result)) {
	PerlIO_pop(aTHX_ f);
	return 0;
    }
    return SvIV(result);
}

static IV
PerlIOVia_seek(pTHX_ PerlIO * f, Off_t offset, int whence)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    SV *offsv = sv_2mortal(sizeof(Off_t) > sizeof(IV)
			   ? newSVnv((NV)offset) : newSViv((IV)offset));
    SV *whsv = sv_2mortal(newSViv(whence));
    SV *result =
	PerlIOVia_method(aTHX_ f, MYMethod(SEEK), G_SCALAR, offsv, whsv,
			 Nullsv);
#if Off_t_size == 8 && defined(CONDOP_SIZE) && CONDOP_SIZE < Off_t_size
    if (result)
	return (Off_t) SvIV(result);
    else
	return (Off_t) -1;
#else
    return (result) ? SvIV(result) : -1;
#endif
}

static Off_t
PerlIOVia_tell(pTHX_ PerlIO * f)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    SV *result =
	PerlIOVia_method(aTHX_ f, MYMethod(TELL), G_SCALAR, Nullsv);
    return (result)
	   ? (SvNOK(result) ? (Off_t)SvNV(result) : (Off_t)SvIV(result))
	   : (Off_t) - 1;
}

static SSize_t
PerlIOVia_unread(pTHX_ PerlIO * f, const void *vbuf, Size_t count)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    SV *buf = newSVpvn_flags((char *) vbuf, count, SVs_TEMP);
    SV *result =
	PerlIOVia_method(aTHX_ f, MYMethod(UNREAD), G_SCALAR, buf, Nullsv);
    if (result)
	return (SSize_t) SvIV(result);
    else {
	return PerlIOBase_unread(aTHX_ f, vbuf, count);
    }
}

static SSize_t
PerlIOVia_read(pTHX_ PerlIO * f, void *vbuf, Size_t count)
{
    SSize_t rd = 0;
    if (PerlIOBase(f)->flags & PERLIO_F_CANREAD) {
	if (PerlIOBase(f)->flags & PERLIO_F_FASTGETS) {
	    rd = PerlIOBase_read(aTHX_ f, vbuf, count);
	}
	else {
	    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
	    SV *buf = sv_2mortal(newSV(count));
	    SV *n = sv_2mortal(newSViv(count));
	    SV *result =
		PerlIOVia_method(aTHX_ f, MYMethod(READ), G_SCALAR, buf, n,
				 Nullsv);
	    if (result) {
		rd = (SSize_t) SvIV(result);
		Move(SvPVX(buf), vbuf, rd, char);
		return rd;
	    }
	}
    }
    return rd;
}

static SSize_t
PerlIOVia_write(pTHX_ PerlIO * f, const void *vbuf, Size_t count)
{
    if (PerlIOBase(f)->flags & PERLIO_F_CANWRITE) {
	PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
	SV *buf = newSVpvn((char *) vbuf, count);
	SV *result =
	    PerlIOVia_method(aTHX_ f, MYMethod(WRITE), G_SCALAR, buf,
			     Nullsv);
	SvREFCNT_dec(buf);
	if (result)
	    return (SSize_t) SvIV(result);
	return -1;
    }
    return 0;
}

static IV
PerlIOVia_fill(pTHX_ PerlIO * f)
{
    if (PerlIOBase(f)->flags & PERLIO_F_CANREAD) {
	PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
	SV *result =
	    PerlIOVia_method(aTHX_ f, MYMethod(FILL), G_SCALAR, Nullsv);
	if (s->var) {
	    SvREFCNT_dec(s->var);
	    s->var = Nullsv;
	}
	if (result && SvOK(result)) {
	    STRLEN len = 0;
	    const char *p = SvPV(result, len);
	    s->var = newSVpvn(p, len);
	    s->cnt = SvCUR(s->var);
	    return 0;
	}
	else
	    PerlIOBase(f)->flags |= PERLIO_F_EOF;
    }
    return -1;
}

static IV
PerlIOVia_flush(pTHX_ PerlIO * f)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    SV *result =
	PerlIOVia_method(aTHX_ f, MYMethod(FLUSH), G_SCALAR, Nullsv);
    if (s->var && s->cnt > 0) {
	SvREFCNT_dec(s->var);
	s->var = Nullsv;
    }
    return (result) ? SvIV(result) : 0;
}

static STDCHAR *
PerlIOVia_get_base(pTHX_ PerlIO * f)
{
    if (PerlIOBase(f)->flags & PERLIO_F_CANREAD) {
	PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
	if (s->var) {
	    return (STDCHAR *) SvPVX(s->var);
	}
    }
    return (STDCHAR *) NULL;
}

static STDCHAR *
PerlIOVia_get_ptr(pTHX_ PerlIO * f)
{
    if (PerlIOBase(f)->flags & PERLIO_F_CANREAD) {
	PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
	if (s->var) {
	    STDCHAR *p = (STDCHAR *) (SvEND(s->var) - s->cnt);
	    return p;
	}
    }
    return (STDCHAR *) NULL;
}

static SSize_t
PerlIOVia_get_cnt(pTHX_ PerlIO * f)
{
    if (PerlIOBase(f)->flags & PERLIO_F_CANREAD) {
	PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
	if (s->var) {
	    return s->cnt;
	}
    }
    return 0;
}

static Size_t
PerlIOVia_bufsiz(pTHX_ PerlIO * f)
{
    if (PerlIOBase(f)->flags & PERLIO_F_CANREAD) {
	PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
	if (s->var)
	    return SvCUR(s->var);
    }
    return 0;
}

static void
PerlIOVia_set_ptrcnt(pTHX_ PerlIO * f, STDCHAR * ptr, SSize_t cnt)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    PERL_UNUSED_ARG(ptr);
    s->cnt = cnt;
}

static void
PerlIOVia_setlinebuf(pTHX_ PerlIO * f)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    PerlIOVia_method(aTHX_ f, MYMethod(SETLINEBUF), G_VOID, Nullsv);
    PerlIOBase_setlinebuf(aTHX_ f);
}

static void
PerlIOVia_clearerr(pTHX_ PerlIO * f)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    PerlIOVia_method(aTHX_ f, MYMethod(CLEARERR), G_VOID, Nullsv);
    PerlIOBase_clearerr(aTHX_ f);
}

static IV
PerlIOVia_error(pTHX_ PerlIO * f)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    SV *result =
	PerlIOVia_method(aTHX_ f, "ERROR", &s->mERROR, G_SCALAR, Nullsv);
    return (result) ? SvIV(result) : PerlIOBase_error(aTHX_ f);
}

static IV
PerlIOVia_eof(pTHX_ PerlIO * f)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    SV *result =
	PerlIOVia_method(aTHX_ f, "EOF", &s->mEOF, G_SCALAR, Nullsv);
    return (result) ? SvIV(result) : PerlIOBase_eof(aTHX_ f);
}

static SV *
PerlIOVia_getarg(pTHX_ PerlIO * f, CLONE_PARAMS * param, int flags)
{
    PerlIOVia *s = PerlIOSelf(f, PerlIOVia);
    PERL_UNUSED_ARG(param);
    PERL_UNUSED_ARG(flags);
    return PerlIOVia_method(aTHX_ f, MYMethod(GETARG), G_SCALAR, Nullsv);
}

static PerlIO *
PerlIOVia_dup(pTHX_ PerlIO * f, PerlIO * o, CLONE_PARAMS * param,
	      int flags)
{
    if ((f = PerlIOBase_dup(aTHX_ f, o, param, flags))) {
	/* Most of the fields will lazily set themselves up as needed
	   stash and obj have been set up by the implied push
	 */
    }
    return f;
}



static PERLIO_FUNCS_DECL(PerlIO_object) = {
 sizeof(PerlIO_funcs),
 "via",
 sizeof(PerlIOVia),
 PERLIO_K_BUFFERED|PERLIO_K_DESTRUCT,
 PerlIOVia_pushed,
 PerlIOVia_popped,
 PerlIOVia_open, /* NULL, */
 PerlIOVia_binmode, /* NULL, */
 PerlIOVia_getarg,
 PerlIOVia_fileno,
 PerlIOVia_dup,
 PerlIOVia_read,
 PerlIOVia_unread,
 PerlIOVia_write,
 PerlIOVia_seek,
 PerlIOVia_tell,
 PerlIOVia_close,
 PerlIOVia_flush,
 PerlIOVia_fill,
 PerlIOVia_eof,
 PerlIOVia_error,
 PerlIOVia_clearerr,
 PerlIOVia_setlinebuf,
 PerlIOVia_get_base,
 PerlIOVia_bufsiz,
 PerlIOVia_get_ptr,
 PerlIOVia_get_cnt,
 PerlIOVia_set_ptrcnt,
};


#endif /* Layers available */

#line 649 "via.c"
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

#line 793 "via.c"
#ifdef __cplusplus
extern "C"
#endif
XS_EXTERNAL(boot_PerlIO__via); /* prototype to pass -Wmissing-prototypes */
XS_EXTERNAL(boot_PerlIO__via)
{
#if PERL_VERSION_LE(5, 21, 5)
    dVAR; dXSARGS;
#else
    dVAR; dXSBOOTARGSXSAPIVERCHK;
#endif

    PERL_UNUSED_VAR(cv); /* -W */
    PERL_UNUSED_VAR(items); /* -W */
#if PERL_VERSION_LE(5, 21, 5)
    XS_VERSION_BOOTCHECK;
#  ifdef XS_APIVERSION_BOOTCHECK
    XS_APIVERSION_BOOTCHECK;
#  endif
#endif


    /* Initialisation Section */

#line 643 "via.xs"
{
#ifdef PERLIO_LAYERS
 PerlIO_define_layer(aTHX_ PERLIO_FUNCS_CAST(&PerlIO_object));
#endif
}

#line 825 "via.c"

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


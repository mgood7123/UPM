/*
 * This file was generated automatically by ExtUtils::ParseXS version 3.34 from the
 * contents of SHA.xs. Do not edit this file, edit SHA.xs instead.
 *
 *    ANY CHANGES MADE HERE WILL BE LOST!
 *
 */

#line 1 "SHA.xs"
#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifdef SvPVbyte
	#if PERL_REVISION == 5 && PERL_VERSION < 8
		#undef SvPVbyte
		#define SvPVbyte(sv, lp) \
			(sv_utf8_downgrade((sv), 0), SvPV((sv), (lp)))
	#endif
#else
	#define SvPVbyte SvPV
#endif

#ifndef dTHX
	#define pTHX_
	#define aTHX_
#endif

#ifndef PerlIO
	#define PerlIO				FILE
	#define PerlIO_read(f, buf, count)	fread(buf, 1, count, f)
#endif

#ifndef sv_derived_from
	#include "src/sdf.c"
#endif

#ifndef Newx
	#define Newx(ptr, num, type)	New(0, ptr, num, type)
	#define Newxz(ptr, num, type)	Newz(0, ptr, num, type)
#endif

#include "src/sha.c"

static const int ix2alg[] =
	{1,1,1,224,224,224,256,256,256,384,384,384,512,512,512,
	512224,512224,512224,512256,512256,512256};

#ifndef INT2PTR
#define INT2PTR(p, i) (p) (i)
#endif

#define MAX_WRITE_SIZE 16384
#define IO_BUFFER_SIZE 4096

static SHA *getSHA(pTHX_ SV *self)
{
	if (!sv_isobject(self) || !sv_derived_from(self, "Digest::SHA"))
		return(NULL);
	return INT2PTR(SHA *, SvIV(SvRV(self)));
}

#line 65 "SHA.c"
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

#line 209 "SHA.c"

XS_EUPXS(XS_Digest__SHA_shainit); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA_shainit)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "s, alg");
    {
	SHA *	s = getSHA(aTHX_ ST(0))
;
	int	alg = (int)SvIV(ST(1))
;
	int	RETVAL;
	dXSTARG;

	RETVAL = shainit(s, alg);
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Digest__SHA_sharewind); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA_sharewind)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "s");
    {
	SHA *	s = getSHA(aTHX_ ST(0))
;

	sharewind(s);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Digest__SHA_shawrite); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA_shawrite)
{
    dVAR; dXSARGS;
    if (items != 3)
       croak_xs_usage(cv,  "bitstr, bitcnt, s");
    {
	unsigned char *	bitstr = (unsigned char *)SvPV_nolen(ST(0))
;
	unsigned long	bitcnt = (unsigned long)SvUV(ST(1))
;
	SHA *	s = getSHA(aTHX_ ST(2))
;
	unsigned long	RETVAL;
	dXSTARG;

	RETVAL = shawrite(bitstr, bitcnt, s);
	XSprePUSH; PUSHu((UV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Digest__SHA_newSHA); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA_newSHA)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "classname, alg");
    {
	char *	classname = (char *)SvPV_nolen(ST(0))
;
	int	alg = (int)SvIV(ST(1))
;
#line 79 "SHA.xs"
	SHA *state;
#line 284 "SHA.c"
	SV *	RETVAL;
#line 81 "SHA.xs"
	Newxz(state, 1, SHA);
	if (!shainit(state, alg)) {
		Safefree(state);
		XSRETURN_UNDEF;
	}
	RETVAL = newSV(0);
	sv_setref_pv(RETVAL, classname, (void *) state);
	SvREADONLY_on(SvRV(RETVAL));
#line 295 "SHA.c"
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Digest__SHA_clone); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA_clone)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "self");
    {
	SV *	self = ST(0)
;
#line 96 "SHA.xs"
	SHA *state;
	SHA *clone;
#line 315 "SHA.c"
	SV *	RETVAL;
#line 99 "SHA.xs"
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	Newx(clone, 1, SHA);
	RETVAL = newSV(0);
	sv_setref_pv(RETVAL, sv_reftype(SvRV(self), 1), (void *) clone);
	SvREADONLY_on(SvRV(RETVAL));
	Copy(state, clone, 1, SHA);
#line 325 "SHA.c"
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Digest__SHA_DESTROY); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA_DESTROY)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "s");
    {
	SHA *	s = getSHA(aTHX_ ST(0))
;
#line 113 "SHA.xs"
	Safefree(s);
#line 344 "SHA.c"
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Digest__SHA_sha1); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA_sha1)
{
    dVAR; dXSARGS;
    dXSI32;
    PERL_UNUSED_VAR(cv); /* -W */
    {
#line 140 "SHA.xs"
	int i;
	UCHR *data;
	STRLEN len;
	SHA sha;
	char *result;
#line 363 "SHA.c"
	SV *	RETVAL;
#line 146 "SHA.xs"
	if (!shainit(&sha, ix2alg[ix]))
		XSRETURN_UNDEF;
	for (i = 0; i < items; i++) {
		data = (UCHR *) (SvPVbyte(ST(i), len));
		while (len > MAX_WRITE_SIZE) {
			shawrite(data, MAX_WRITE_SIZE << 3, &sha);
			data += MAX_WRITE_SIZE;
			len  -= MAX_WRITE_SIZE;
		}
		shawrite(data, len << 3, &sha);
	}
	shafinish(&sha);
	len = 0;
	if (ix % 3 == 0) {
		result = (char *) shadigest(&sha);
		len = sha.digestlen;
	}
	else if (ix % 3 == 1)
		result = shahex(&sha);
	else
		result = shabase64(&sha);
	RETVAL = newSVpv(result, len);
#line 388 "SHA.c"
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Digest__SHA_hmac_sha1); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA_hmac_sha1)
{
    dVAR; dXSARGS;
    dXSI32;
    PERL_UNUSED_VAR(cv); /* -W */
    {
#line 196 "SHA.xs"
	int i;
	UCHR *key = (UCHR *) "";
	UCHR *data;
	STRLEN len = 0;
	HMAC hmac;
	char *result;
#line 410 "SHA.c"
	SV *	RETVAL;
#line 203 "SHA.xs"
	if (items > 0) {
		key = (UCHR *) (SvPVbyte(ST(items-1), len));
	}
	if (hmacinit(&hmac, ix2alg[ix], key, len) == NULL)
		XSRETURN_UNDEF;
	for (i = 0; i < items - 1; i++) {
		data = (UCHR *) (SvPVbyte(ST(i), len));
		while (len > MAX_WRITE_SIZE) {
			hmacwrite(data, MAX_WRITE_SIZE << 3, &hmac);
			data += MAX_WRITE_SIZE;
			len  -= MAX_WRITE_SIZE;
		}
		hmacwrite(data, len << 3, &hmac);
	}
	hmacfinish(&hmac);
	len = 0;
	if (ix % 3 == 0) {
		result = (char *) hmacdigest(&hmac);
		len = hmac.digestlen;
	}
	else if (ix % 3 == 1)
		result = hmachex(&hmac);
	else
		result = hmacbase64(&hmac);
	RETVAL = newSVpv(result, len);
#line 438 "SHA.c"
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Digest__SHA_hashsize); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA_hashsize)
{
    dVAR; dXSARGS;
    dXSI32;
    if (items != 1)
       croak_xs_usage(cv,  "self");
    {
	SV *	self = ST(0)
;
#line 238 "SHA.xs"
	SHA *state;
#line 458 "SHA.c"
	int	RETVAL;
	dXSTARG;
#line 240 "SHA.xs"
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	RETVAL = ix ? state->alg : (int) (state->digestlen << 3);
#line 465 "SHA.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Digest__SHA_add); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA_add)
{
    dVAR; dXSARGS;
    if (items < 1)
       croak_xs_usage(cv,  "self, ...");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	SV *	self = ST(0)
;
#line 250 "SHA.xs"
	int i;
	UCHR *data;
	STRLEN len;
	SHA *state;
#line 488 "SHA.c"
#line 255 "SHA.xs"
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	for (i = 1; i < items; i++) {
		data = (UCHR *) (SvPVbyte(ST(i), len));
		while (len > MAX_WRITE_SIZE) {
			shawrite(data, MAX_WRITE_SIZE << 3, state);
			data += MAX_WRITE_SIZE;
			len  -= MAX_WRITE_SIZE;
		}
		shawrite(data, len << 3, state);
	}
	XSRETURN(1);
#line 502 "SHA.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Digest__SHA_digest); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA_digest)
{
    dVAR; dXSARGS;
    dXSI32;
    if (items != 1)
       croak_xs_usage(cv,  "self");
    {
	SV *	self = ST(0)
;
#line 276 "SHA.xs"
	STRLEN len;
	SHA *state;
	char *result;
#line 523 "SHA.c"
	SV *	RETVAL;
#line 280 "SHA.xs"
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	shafinish(state);
	len = 0;
	if (ix == 0) {
		result = (char *) shadigest(state);
		len = state->digestlen;
	}
	else if (ix == 1)
		result = shahex(state);
	else
		result = shabase64(state);
	RETVAL = newSVpv(result, len);
	sharewind(state);
#line 540 "SHA.c"
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Digest__SHA__getstate); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA__getstate)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "self");
    {
	SV *	self = ST(0)
;
#line 301 "SHA.xs"
	SHA *state;
	UCHR buf[256];
	UCHR *ptr = buf;
#line 561 "SHA.c"
	SV *	RETVAL;
#line 305 "SHA.xs"
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	Copy(digcpy(state), ptr, state->alg <= SHA256 ? 32 : 64, UCHR);
	ptr += state->alg <= SHA256 ? 32 : 64;
	Copy(state->block, ptr, state->alg <= SHA256 ? 64 : 128, UCHR);
	ptr += state->alg <= SHA256 ? 64 : 128;
	ptr = w32mem(ptr, state->blockcnt);
	ptr = w32mem(ptr, state->lenhh);
	ptr = w32mem(ptr, state->lenhl);
	ptr = w32mem(ptr, state->lenlh);
	ptr = w32mem(ptr, state->lenll);
	RETVAL = newSVpv((char *) buf, (STRLEN) (ptr - buf));
#line 576 "SHA.c"
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Digest__SHA__putstate); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA__putstate)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "self, packed_state");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	SV *	self = ST(0)
;
	SV *	packed_state = ST(1)
;
#line 325 "SHA.xs"
	UINT bc;
	STRLEN len;
	SHA *state;
	UCHR *data;
#line 602 "SHA.c"
#line 330 "SHA.xs"
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	data = (UCHR *) SvPV(packed_state, len);
	if (len != (state->alg <= SHA256 ? 116U : 212U))
		XSRETURN_UNDEF;
	data = statecpy(state, data);
	Copy(data, state->block, state->blocksize >> 3, UCHR);
	data += (state->blocksize >> 3);
	bc = memw32(data), data += 4;
	if (bc >= (state->alg <= SHA256 ? 512U : 1024U))
		XSRETURN_UNDEF;
	state->blockcnt = bc;
	state->lenhh = memw32(data), data += 4;
	state->lenhl = memw32(data), data += 4;
	state->lenlh = memw32(data), data += 4;
	state->lenll = memw32(data);
	XSRETURN(1);
#line 621 "SHA.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Digest__SHA__addfilebin); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA__addfilebin)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "self, f");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	SV *	self = ST(0)
;
	PerlIO *	f = IoIFP(sv_2io(ST(1)))
;
#line 353 "SHA.xs"
	SHA *state;
	int n;
	UCHR in[IO_BUFFER_SIZE];
#line 645 "SHA.c"
#line 357 "SHA.xs"
	if (!f || (state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	while ((n = PerlIO_read(f, in, sizeof(in))) > 0)
		shawrite(in, (ULNG) n << 3, state);
	XSRETURN(1);
#line 652 "SHA.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Digest__SHA__addfileuniv); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Digest__SHA__addfileuniv)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "self, f");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	SV *	self = ST(0)
;
	PerlIO *	f = IoIFP(sv_2io(ST(1)))
;
#line 368 "SHA.xs"
	UCHR c;
	int n;
	int cr = 0;
	UCHR *src, *dst;
	UCHR in[IO_BUFFER_SIZE+1];
	SHA *state;
#line 679 "SHA.c"
#line 375 "SHA.xs"
	if (!f || (state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	while ((n = PerlIO_read(f, in+1, IO_BUFFER_SIZE)) > 0) {
		for (dst = in, src = in + 1; n; n--) {
			c = *src++;
			if (!cr) {
				if (c == '\015')
					cr = 1;
				else
					*dst++ = c;
			}
			else {
				if (c == '\015')
					*dst++ = '\012';
				else if (c == '\012') {
					*dst++ = '\012';
					cr = 0;
				}
				else {
					*dst++ = '\012';
					*dst++ = c;
					cr = 0;
				}
			}
		}
		shawrite(in, (ULNG) (dst - in) << 3, state);
	}
	if (cr) {
		in[0] = '\012';
		shawrite(in, 1 << 3, state);
	}
	XSRETURN(1);
#line 713 "SHA.c"
	PUTBACK;
	return;
    }
}

#ifdef __cplusplus
extern "C"
#endif
XS_EXTERNAL(boot_Digest__SHA); /* prototype to pass -Wmissing-prototypes */
XS_EXTERNAL(boot_Digest__SHA)
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

        (void)newXSproto_portable("Digest::SHA::shainit", XS_Digest__SHA_shainit, file, "$$");
        (void)newXSproto_portable("Digest::SHA::sharewind", XS_Digest__SHA_sharewind, file, "$");
        (void)newXSproto_portable("Digest::SHA::shawrite", XS_Digest__SHA_shawrite, file, "$$$");
        (void)newXSproto_portable("Digest::SHA::newSHA", XS_Digest__SHA_newSHA, file, "$$");
        (void)newXSproto_portable("Digest::SHA::clone", XS_Digest__SHA_clone, file, "$");
        (void)newXSproto_portable("Digest::SHA::DESTROY", XS_Digest__SHA_DESTROY, file, "$");
        cv = newXSproto_portable("Digest::SHA::sha1", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 0;
        cv = newXSproto_portable("Digest::SHA::sha1_base64", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 2;
        cv = newXSproto_portable("Digest::SHA::sha1_hex", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 1;
        cv = newXSproto_portable("Digest::SHA::sha224", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 3;
        cv = newXSproto_portable("Digest::SHA::sha224_base64", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 5;
        cv = newXSproto_portable("Digest::SHA::sha224_hex", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 4;
        cv = newXSproto_portable("Digest::SHA::sha256", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 6;
        cv = newXSproto_portable("Digest::SHA::sha256_base64", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 8;
        cv = newXSproto_portable("Digest::SHA::sha256_hex", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 7;
        cv = newXSproto_portable("Digest::SHA::sha384", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 9;
        cv = newXSproto_portable("Digest::SHA::sha384_base64", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 11;
        cv = newXSproto_portable("Digest::SHA::sha384_hex", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 10;
        cv = newXSproto_portable("Digest::SHA::sha512", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 12;
        cv = newXSproto_portable("Digest::SHA::sha512224", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 15;
        cv = newXSproto_portable("Digest::SHA::sha512224_base64", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 17;
        cv = newXSproto_portable("Digest::SHA::sha512224_hex", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 16;
        cv = newXSproto_portable("Digest::SHA::sha512256", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 18;
        cv = newXSproto_portable("Digest::SHA::sha512256_base64", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 20;
        cv = newXSproto_portable("Digest::SHA::sha512256_hex", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 19;
        cv = newXSproto_portable("Digest::SHA::sha512_base64", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 14;
        cv = newXSproto_portable("Digest::SHA::sha512_hex", XS_Digest__SHA_sha1, file, ";@");
        XSANY.any_i32 = 13;
        cv = newXSproto_portable("Digest::SHA::hmac_sha1", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 0;
        cv = newXSproto_portable("Digest::SHA::hmac_sha1_base64", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 2;
        cv = newXSproto_portable("Digest::SHA::hmac_sha1_hex", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 1;
        cv = newXSproto_portable("Digest::SHA::hmac_sha224", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 3;
        cv = newXSproto_portable("Digest::SHA::hmac_sha224_base64", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 5;
        cv = newXSproto_portable("Digest::SHA::hmac_sha224_hex", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 4;
        cv = newXSproto_portable("Digest::SHA::hmac_sha256", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 6;
        cv = newXSproto_portable("Digest::SHA::hmac_sha256_base64", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 8;
        cv = newXSproto_portable("Digest::SHA::hmac_sha256_hex", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 7;
        cv = newXSproto_portable("Digest::SHA::hmac_sha384", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 9;
        cv = newXSproto_portable("Digest::SHA::hmac_sha384_base64", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 11;
        cv = newXSproto_portable("Digest::SHA::hmac_sha384_hex", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 10;
        cv = newXSproto_portable("Digest::SHA::hmac_sha512", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 12;
        cv = newXSproto_portable("Digest::SHA::hmac_sha512224", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 15;
        cv = newXSproto_portable("Digest::SHA::hmac_sha512224_base64", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 17;
        cv = newXSproto_portable("Digest::SHA::hmac_sha512224_hex", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 16;
        cv = newXSproto_portable("Digest::SHA::hmac_sha512256", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 18;
        cv = newXSproto_portable("Digest::SHA::hmac_sha512256_base64", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 20;
        cv = newXSproto_portable("Digest::SHA::hmac_sha512256_hex", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 19;
        cv = newXSproto_portable("Digest::SHA::hmac_sha512_base64", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 14;
        cv = newXSproto_portable("Digest::SHA::hmac_sha512_hex", XS_Digest__SHA_hmac_sha1, file, ";@");
        XSANY.any_i32 = 13;
        cv = newXSproto_portable("Digest::SHA::algorithm", XS_Digest__SHA_hashsize, file, "$");
        XSANY.any_i32 = 1;
        cv = newXSproto_portable("Digest::SHA::hashsize", XS_Digest__SHA_hashsize, file, "$");
        XSANY.any_i32 = 0;
        (void)newXSproto_portable("Digest::SHA::add", XS_Digest__SHA_add, file, "$;@");
        cv = newXSproto_portable("Digest::SHA::b64digest", XS_Digest__SHA_digest, file, "$");
        XSANY.any_i32 = 2;
        cv = newXSproto_portable("Digest::SHA::digest", XS_Digest__SHA_digest, file, "$");
        XSANY.any_i32 = 0;
        cv = newXSproto_portable("Digest::SHA::hexdigest", XS_Digest__SHA_digest, file, "$");
        XSANY.any_i32 = 1;
        (void)newXSproto_portable("Digest::SHA::_getstate", XS_Digest__SHA__getstate, file, "$");
        (void)newXSproto_portable("Digest::SHA::_putstate", XS_Digest__SHA__putstate, file, "$$");
        (void)newXSproto_portable("Digest::SHA::_addfilebin", XS_Digest__SHA__addfilebin, file, "$$");
        (void)newXSproto_portable("Digest::SHA::_addfileuniv", XS_Digest__SHA__addfileuniv, file, "$$");
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


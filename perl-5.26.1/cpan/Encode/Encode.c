/*
 * This file was generated automatically by ExtUtils::ParseXS version 3.34 from the
 * contents of Encode.xs. Do not edit this file, edit Encode.xs instead.
 *
 *    ANY CHANGES MADE HERE WILL BE LOST!
 *
 */

#line 1 "Encode.xs"
/*
 $Id: Encode.xs,v 2.39 2016/11/29 23:29:23 dankogai Exp dankogai $
 */

#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "encode.h"
#include "def_t.h"

# define PERLIO_MODNAME  "PerlIO::encoding"
# define PERLIO_FILENAME "PerlIO/encoding.pm"

/* set 1 or more to profile.  t/encoding.t dumps core because of
   Perl_warner and PerlIO don't work well */
#define ENCODE_XS_PROFILE 0

/* set 0 to disable floating point to calculate buffer size for
   encode_method().  1 is recommended. 2 restores NI-S original */
#define ENCODE_XS_USEFP   1

#define UNIMPLEMENTED(x,y) static y x (SV *sv, char *encoding) {	\
			Perl_croak_nocontext("panic_unimplemented");	\
                        PERL_UNUSED_VAR(sv); \
                        PERL_UNUSED_VAR(encoding); \
             return (y)0; /* fool picky compilers */ \
                         }
/**/

UNIMPLEMENTED(_encoded_utf8_to_bytes, I32)
UNIMPLEMENTED(_encoded_bytes_to_utf8, I32)

#ifndef SvIV_nomg
#define SvIV_nomg SvIV
#endif

#ifdef UTF8_DISALLOW_ILLEGAL_INTERCHANGE
#   define UTF8_ALLOW_STRICT UTF8_DISALLOW_ILLEGAL_INTERCHANGE
#else
#   define UTF8_ALLOW_STRICT 0
#endif

#define UTF8_ALLOW_NONSTRICT (UTF8_ALLOW_ANY &                    \
                              ~(UTF8_ALLOW_CONTINUATION |         \
                                UTF8_ALLOW_NON_CONTINUATION |     \
                                UTF8_ALLOW_LONG))

static void
Encode_XSEncoding(pTHX_ encode_t * enc)
{
    dSP;
    HV *stash = gv_stashpv("Encode::XS", TRUE);
    SV *iv    = newSViv(PTR2IV(enc));
    SV *sv    = sv_bless(newRV_noinc(iv),stash);
    int i = 0;
    /* with the SvLEN() == 0 hack, PVX won't be freed. We cast away name's
    constness, in the hope that perl won't mess with it. */
    assert(SvTYPE(iv) >= SVt_PV); assert(SvLEN(iv) == 0);
    SvFLAGS(iv) |= SVp_POK;
    SvPVX(iv) = (char*) enc->name[0];
    PUSHMARK(sp);
    XPUSHs(sv);
    while (enc->name[i]) {
    const char *name = enc->name[i++];
    XPUSHs(sv_2mortal(newSVpvn(name, strlen(name))));
    }
    PUTBACK;
    call_pv("Encode::define_encoding", G_DISCARD);
    SvREFCNT_dec(sv);
}

static void
call_failure(SV * routine, U8 * done, U8 * dest, U8 * orig)
{
    /* Exists for breakpointing */
    PERL_UNUSED_VAR(routine);
    PERL_UNUSED_VAR(done);
    PERL_UNUSED_VAR(dest);
    PERL_UNUSED_VAR(orig);
}

static void
utf8_safe_downgrade(pTHX_ SV ** src, U8 ** s, STRLEN * slen, bool modify)
{
    if (!modify) {
        SV *tmp = sv_2mortal(newSVpvn((char *)*s, *slen));
        SvUTF8_on(tmp);
        if (SvTAINTED(*src))
            SvTAINTED_on(tmp);
        *src = tmp;
        *s = (U8 *)SvPVX(*src);
    }
    if (*slen) {
        if (!utf8_to_bytes(*s, slen))
            croak("Wide character");
        SvCUR_set(*src, *slen);
    }
    SvUTF8_off(*src);
}

static void
utf8_safe_upgrade(pTHX_ SV ** src, U8 ** s, STRLEN * slen, bool modify)
{
    if (!modify) {
        SV *tmp = sv_2mortal(newSVpvn((char *)*s, *slen));
        if (SvTAINTED(*src))
            SvTAINTED_on(tmp);
        *src = tmp;
    }
    sv_utf8_upgrade_nomg(*src);
    *s = (U8 *)SvPV_nomg(*src, *slen);
}

#define ERR_ENCODE_NOMAP "\"\\x{%04" UVxf "}\" does not map to %s"
#define ERR_DECODE_NOMAP "%s \"\\x%02" UVXf "\" does not map to Unicode"

static SV *
do_fallback_cb(pTHX_ UV ch, SV *fallback_cb)
{
    dSP;
    int argc;
    SV *retval = newSVpv("",0);
    ENTER;
    SAVETMPS;
    PUSHMARK(sp);
    XPUSHs(sv_2mortal(newSVnv((UV)ch)));
    PUTBACK;
    argc = call_sv(fallback_cb, G_SCALAR);
    SPAGAIN;
    if (argc != 1){
	croak("fallback sub must return scalar!");
    }
    sv_catsv(retval, POPs);
    PUTBACK;
    FREETMPS;
    LEAVE;
    return retval;
}

static SV *
encode_method(pTHX_ const encode_t * enc, const encpage_t * dir, SV * src, U8 * s, STRLEN slen,
	      int check, STRLEN * offset, SV * term, int * retcode, 
	      SV *fallback_cb)
{
    STRLEN tlen  = slen;
    STRLEN ddone = 0;
    STRLEN sdone = 0;
    /* We allocate slen+1.
       PerlIO dumps core if this value is smaller than this. */
    SV *dst = newSV(slen+1);
    U8 *d = (U8 *)SvPVX(dst);
    STRLEN dlen = SvLEN(dst)-1;
    int code = 0;
    STRLEN trmlen = 0;
    U8 *trm = term ? (U8*) SvPV(term, trmlen) : NULL;

    if (SvTAINTED(src)) SvTAINTED_on(dst); /* propagate taintedness */

    if (offset) {
      s += *offset;
      if (slen > *offset){ /* safeguard against slen overflow */
      slen -= *offset;
      }else{
      slen = 0;
      }
      tlen = slen;
    }

    if (slen == 0){
    SvCUR_set(dst, 0);
    SvPOK_only(dst);
    goto ENCODE_END;
    }

    while( (code = do_encode(dir, s, &slen, d, dlen, &dlen, !check,
                 trm, trmlen)) ) 
    {
    SvCUR_set(dst, dlen+ddone);
    SvPOK_only(dst);
    
    if (code == ENCODE_FALLBACK || code == ENCODE_PARTIAL ||
        code == ENCODE_FOUND_TERM) {
        break;
    }
    switch (code) {
    case ENCODE_NOSPACE:
    {	
        STRLEN more = 0; /* make sure you initialize! */
        STRLEN sleft;
        sdone += slen;
        ddone += dlen;
        sleft = tlen - sdone;
#if ENCODE_XS_PROFILE >= 2
        Perl_warn(aTHX_
              "more=%d, sdone=%d, sleft=%d, SvLEN(dst)=%d\n",
              more, sdone, sleft, SvLEN(dst));
#endif
        if (sdone != 0) { /* has src ever been processed ? */
#if   ENCODE_XS_USEFP == 2
        more = (1.0*tlen*SvLEN(dst)+sdone-1)/sdone
            - SvLEN(dst);
#elif ENCODE_XS_USEFP
        more = (STRLEN)((1.0*SvLEN(dst)+1)/sdone * sleft);
#else
        /* safe until SvLEN(dst) == MAX_INT/16 */
        more = (16*SvLEN(dst)+1)/sdone/16 * sleft;
#endif
        }
        more += UTF8_MAXLEN; /* insurance policy */
        d = (U8 *) SvGROW(dst, SvLEN(dst) + more);
        /* dst need to grow need MORE bytes! */
        if (ddone >= SvLEN(dst)) {
        Perl_croak(aTHX_ "Destination couldn't be grown.");
        }
        dlen = SvLEN(dst)-ddone-1;
        d   += ddone;
        s   += slen;
        slen = tlen-sdone;
        continue;
    }
    case ENCODE_NOREP:
        /* encoding */	
        if (dir == enc->f_utf8) {
        STRLEN clen;
        UV ch =
            utf8n_to_uvuni(s+slen, (tlen-sdone-slen),
                   &clen, UTF8_ALLOW_ANY|UTF8_CHECK_ONLY);
        /* if non-representable multibyte prefix at end of current buffer - break*/
        if (clen > tlen - sdone - slen) break;
        if (check & ENCODE_DIE_ON_ERR) {
            Perl_croak(aTHX_ ERR_ENCODE_NOMAP,
                   (UV)ch, enc->name[0]);
            return &PL_sv_undef; /* never reaches but be safe */
        }
        if (check & ENCODE_WARN_ON_ERR){
            Perl_warner(aTHX_ packWARN(WARN_UTF8),
                ERR_ENCODE_NOMAP, (UV)ch, enc->name[0]);
        }
        if (check & ENCODE_RETURN_ON_ERR){
            goto ENCODE_SET_SRC;
        }
        if (check & (ENCODE_PERLQQ|ENCODE_HTMLCREF|ENCODE_XMLCREF)){
            SV* subchar = 
            (fallback_cb != &PL_sv_undef)
		? do_fallback_cb(aTHX_ ch, fallback_cb)
		: newSVpvf(check & ENCODE_PERLQQ ? "\\x{%04" UVxf "}" :
                 check & ENCODE_HTMLCREF ? "&#%" UVuf ";" :
                 "&#x%" UVxf ";", (UV)ch);
	    SvUTF8_off(subchar); /* make sure no decoded string gets in */
            sdone += slen + clen;
            ddone += dlen + SvCUR(subchar);
            sv_catsv(dst, subchar);
            SvREFCNT_dec(subchar);
        } else {
            /* fallback char */
            sdone += slen + clen;
            ddone += dlen + enc->replen;
            sv_catpvn(dst, (char*)enc->rep, enc->replen);
        }
        }
        /* decoding */
        else {
        if (check & ENCODE_DIE_ON_ERR){
            Perl_croak(aTHX_ ERR_DECODE_NOMAP,
                              enc->name[0], (UV)s[slen]);
            return &PL_sv_undef; /* never reaches but be safe */
        }
        if (check & ENCODE_WARN_ON_ERR){
            Perl_warner(
            aTHX_ packWARN(WARN_UTF8),
            ERR_DECODE_NOMAP,
               	        enc->name[0], (UV)s[slen]);
        }
        if (check & ENCODE_RETURN_ON_ERR){
            goto ENCODE_SET_SRC;
        }
        if (check &
            (ENCODE_PERLQQ|ENCODE_HTMLCREF|ENCODE_XMLCREF)){
            SV* subchar = 
            (fallback_cb != &PL_sv_undef)
		? do_fallback_cb(aTHX_ (UV)s[slen], fallback_cb) 
		: newSVpvf("\\x%02" UVXf, (UV)s[slen]);
            sdone += slen + 1;
            ddone += dlen + SvCUR(subchar);
            sv_catsv(dst, subchar);
            SvREFCNT_dec(subchar);
        } else {
            sdone += slen + 1;
            ddone += dlen + strlen(FBCHAR_UTF8);
            sv_catpv(dst, FBCHAR_UTF8);
        }
        }
        /* settle variables when fallback */
        d    = (U8 *)SvEND(dst);
            dlen = SvLEN(dst) - ddone - 1;
        s    = (U8*)SvPVX(src) + sdone;
        slen = tlen - sdone;
        break;

    default:
        Perl_croak(aTHX_ "Unexpected code %d converting %s %s",
               code, (dir == enc->f_utf8) ? "to" : "from",
               enc->name[0]);
        return &PL_sv_undef;
    }
    }
 ENCODE_SET_SRC:
    if (check && !(check & ENCODE_LEAVE_SRC)){
    sdone = SvCUR(src) - (slen+sdone);
    if (sdone) {
        sv_setpvn(src, (char*)s+slen, sdone);
    }
    SvCUR_set(src, sdone);
    SvSETMAGIC(src);
    }
    /* warn("check = 0x%X, code = 0x%d\n", check, code); */

    SvCUR_set(dst, dlen+ddone);
    SvPOK_only(dst);

#if ENCODE_XS_PROFILE
    if (SvCUR(dst) > SvCUR(src)){
    Perl_warn(aTHX_
          "SvLEN(dst)=%d, SvCUR(dst)=%d. %d bytes unused(%f %%)\n",
          SvLEN(dst), SvCUR(dst), SvLEN(dst) - SvCUR(dst),
          (SvLEN(dst) - SvCUR(dst))*1.0/SvLEN(dst)*100.0);
    }
#endif

    if (offset) 
      *offset += sdone + slen;

 ENCODE_END:
    *SvEND(dst) = '\0';
    if (retcode) *retcode = code;
    return dst;
}

static bool
strict_utf8(pTHX_ SV* sv)
{
    HV* hv;
    SV** svp;
    sv = SvRV(sv);
    if (!sv || SvTYPE(sv) != SVt_PVHV)
        return 0;
    hv = (HV*)sv;
    svp = hv_fetch(hv, "strict_utf8", 11, 0);
    if (!svp)
        return 0;
    return SvTRUE(*svp);
}

/*
 * https://github.com/dankogai/p5-encode/pull/56#issuecomment-231959126
 */
#ifndef UNICODE_IS_NONCHAR
#define UNICODE_IS_NONCHAR(c) ((c >= 0xFDD0 && c <= 0xFDEF) || (c & 0xFFFE) == 0xFFFE)
#endif

#ifndef UNICODE_IS_SUPER
#define UNICODE_IS_SUPER(c) (c > PERL_UNICODE_MAX)
#endif

#define UNICODE_IS_STRICT(c) (!UNICODE_IS_SURROGATE(c) && !UNICODE_IS_NONCHAR(c) && !UNICODE_IS_SUPER(c))

#ifndef UTF_ACCUMULATION_OVERFLOW_MASK
#ifndef CHARBITS
#define CHARBITS CHAR_BIT
#endif
#define UTF_ACCUMULATION_OVERFLOW_MASK (((UV) UTF_CONTINUATION_MASK) << ((sizeof(UV) * CHARBITS) - UTF_ACCUMULATION_SHIFT))
#endif

/*
 * Convert non strict utf8 sequence of len >= 2 to unicode codepoint
 */
static UV
convert_utf8_multi_seq(U8* s, STRLEN len, STRLEN *rlen)
{
    UV uv;
    U8 *ptr = s;
    bool overflowed = 0;

    uv = NATIVE_TO_UTF(*s) & UTF_START_MASK(len);

    len--;
    s++;

    while (len--) {
        if (!UTF8_IS_CONTINUATION(*s)) {
            *rlen = s-ptr;
            return 0;
        }
        if (uv & UTF_ACCUMULATION_OVERFLOW_MASK)
            overflowed = 1;
        uv = UTF8_ACCUMULATE(uv, *s);
        s++;
    }

    *rlen = s-ptr;

    if (overflowed || *rlen > (STRLEN)UNISKIP(uv)) {
        *rlen = 1;
        return 0;
    }

    return uv;
}

static U8*
process_utf8(pTHX_ SV* dst, U8* s, U8* e, SV *check_sv,
             bool encode, bool strict, bool stop_at_partial)
{
    UV uv;
    STRLEN ulen;
    SV *fallback_cb;
    int check;
    U8 *d;
    STRLEN dlen;

    if (SvROK(check_sv)) {
	/* croak("UTF-8 decoder doesn't support callback CHECK"); */
	fallback_cb = check_sv;
	check = ENCODE_PERLQQ|ENCODE_LEAVE_SRC; /* same as perlqq */
    }
    else {
	fallback_cb = &PL_sv_undef;
	check = SvIV_nomg(check_sv);
    }

    SvPOK_only(dst);
    SvCUR_set(dst,0);

    dlen = (s && e && s < e) ? e-s+1 : 1;
    d = (U8 *) SvGROW(dst, dlen);

    while (s < e) {
        if (UTF8_IS_INVARIANT(*s)) {
            *d++ = *s++;
            continue;
        }

        ulen = 1;
        if (UTF8_IS_START(*s)) {
            U8 skip = UTF8SKIP(s);
            if ((s + skip) > e) {
                if (stop_at_partial || (check & ENCODE_STOP_AT_PARTIAL)) {
                    const U8 *p = s + 1;
                    for (; p < e; p++) {
                        if (!UTF8_IS_CONTINUATION(*p)) {
                            ulen = p-s;
                            goto malformed_byte;
                        }
                    }
                    break;
                }

                ulen = e-s;
                goto malformed_byte;
            }

            uv = convert_utf8_multi_seq(s, skip, &ulen);
            if (uv == 0)
                goto malformed_byte;
            else if (strict && !UNICODE_IS_STRICT(uv))
                goto malformed;


             /* Whole char is good */
             memcpy(d, s, skip);
             d += skip;
             s += skip;
             continue;
        }

        /* If we get here there is something wrong with alleged UTF-8 */
    malformed_byte:
        uv = (UV)*s;
        if (ulen == 0)
            ulen = 1;

    malformed:
        if (check & ENCODE_DIE_ON_ERR){
            if (encode)
                Perl_croak(aTHX_ ERR_ENCODE_NOMAP, uv, "utf8");
            else
                Perl_croak(aTHX_ ERR_DECODE_NOMAP, "utf8", uv);
        }
        if (check & ENCODE_WARN_ON_ERR){
            if (encode)
                Perl_warner(aTHX_ packWARN(WARN_UTF8),
                            ERR_ENCODE_NOMAP, uv, "utf8");
            else
                Perl_warner(aTHX_ packWARN(WARN_UTF8),
                            ERR_DECODE_NOMAP, "utf8", uv);
        }
        if (check & ENCODE_RETURN_ON_ERR) {
                break;
        }
        if (check & (ENCODE_PERLQQ|ENCODE_HTMLCREF|ENCODE_XMLCREF)){
	    SV* subchar =
		(fallback_cb != &PL_sv_undef)
		? do_fallback_cb(aTHX_ uv, fallback_cb)
		: newSVpvf(check & ENCODE_PERLQQ 
			   ? (ulen == 1 ? "\\x%02" UVXf : "\\x{%04" UVXf "}")
			   :  check & ENCODE_HTMLCREF ? "&#%" UVuf ";" 
			   : "&#x%" UVxf ";", uv);
	    if (encode){
		SvUTF8_off(subchar); /* make sure no decoded string gets in */
	    }
            dlen += SvCUR(subchar) - ulen;
            SvCUR_set(dst, d-(U8 *)SvPVX(dst));
            *SvEND(dst) = '\0';
            sv_catsv(dst, subchar);
            SvREFCNT_dec(subchar);
            d = (U8 *) SvGROW(dst, dlen) + SvCUR(dst);
        } else {
            STRLEN fbcharlen = strlen(FBCHAR_UTF8);
            dlen += fbcharlen - ulen;
            if (SvLEN(dst) < dlen) {
                SvCUR_set(dst, d-(U8 *)SvPVX(dst));
                d = (U8 *) sv_grow(dst, dlen) + SvCUR(dst);
            }
            memcpy(d, FBCHAR_UTF8, fbcharlen);
            d += fbcharlen;
        }
        s += ulen;
    }
    SvCUR_set(dst, d-(U8 *)SvPVX(dst));
    *SvEND(dst) = '\0';

    return s;
}


#line 547 "Encode.c"
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

#line 691 "Encode.c"

XS_EUPXS(XS_Encode__utf8_decode_xs); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__utf8_decode_xs)
{
    dVAR; dXSARGS;
    if (items < 2 || items > 3)
       croak_xs_usage(cv,  "obj, src, check_sv = &PL_sv_no");
    {
	SV *	obj = ST(0)
;
	SV *	src = ST(1)
;
	SV *	check_sv;
#line 547 "Encode.xs"
    STRLEN slen;
    U8 *s;
    U8 *e;
    SV *dst;
    bool renewed = 0;
    int check;
    bool modify;
#line 713 "Encode.c"

	if (items < 3)
	    check_sv = &PL_sv_no;
	else {
	    check_sv = ST(2)
;
	}
#line 555 "Encode.xs"
    SvGETMAGIC(src);
    SvGETMAGIC(check_sv);
    check = SvROK(check_sv) ? ENCODE_PERLQQ|ENCODE_LEAVE_SRC : SvIV_nomg(check_sv);
    modify = (check && !(check & ENCODE_LEAVE_SRC));
#line 726 "Encode.c"
#line 560 "Encode.xs"
{
    dSP;
    if (!SvOK(src))
        XSRETURN_UNDEF;
    s = modify ? (U8 *)SvPV_force_nomg(src, slen) : (U8 *)SvPV_nomg(src, slen);
    if (SvUTF8(src))
        utf8_safe_downgrade(aTHX_ &src, &s, &slen, modify);
    e = s+slen;

    /* 
     * PerlIO check -- we assume the object is of PerlIO if renewed
     */
    ENTER; SAVETMPS;
    PUSHMARK(sp);
    XPUSHs(obj);
    PUTBACK;
    if (call_method("renewed",G_SCALAR) == 1) {
    SPAGAIN;
    renewed = (bool)POPi;
    PUTBACK; 
#if 0
    fprintf(stderr, "renewed == %d\n", renewed);
#endif
    }
    FREETMPS; LEAVE;
    /* end PerlIO check */

    dst = sv_2mortal(newSV(slen>0?slen:1)); /* newSV() abhors 0 -- inaba */
    s = process_utf8(aTHX_ dst, s, e, check_sv, 0, strict_utf8(aTHX_ obj), renewed);

    /* Clear out translated part of source unless asked not to */
    if (modify) {
    slen = e-s;
    if (slen) {
        sv_setpvn(src, (char*)s, slen);
    }
    SvCUR_set(src, slen);
    SvSETMAGIC(src);
    }
    SvUTF8_on(dst);
    if (SvTAINTED(src)) SvTAINTED_on(dst); /* propagate taintedness */
    ST(0) = dst;
    XSRETURN(1);
}
#line 772 "Encode.c"
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__utf8_encode_xs); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__utf8_encode_xs)
{
    dVAR; dXSARGS;
    if (items < 2 || items > 3)
       croak_xs_usage(cv,  "obj, src, check_sv = &PL_sv_no");
    {
	SV *	obj = ST(0)
;
	SV *	src = ST(1)
;
	SV *	check_sv;
#line 611 "Encode.xs"
    STRLEN slen;
    U8 *s;
    U8 *e;
    SV *dst;
    int check;
    bool modify;
#line 797 "Encode.c"

	if (items < 3)
	    check_sv = &PL_sv_no;
	else {
	    check_sv = ST(2)
;
	}
#line 618 "Encode.xs"
    SvGETMAGIC(src);
    SvGETMAGIC(check_sv);
    check = SvROK(check_sv) ? ENCODE_PERLQQ|ENCODE_LEAVE_SRC : SvIV_nomg(check_sv);
    modify = (check && !(check & ENCODE_LEAVE_SRC));
#line 810 "Encode.c"
#line 623 "Encode.xs"
{
    if (!SvOK(src))
        XSRETURN_UNDEF;
    s = modify ? (U8 *)SvPV_force_nomg(src, slen) : (U8 *)SvPV_nomg(src, slen);
    e = s+slen;
    dst = sv_2mortal(newSV(slen>0?slen:1)); /* newSV() abhors 0 -- inaba */
    if (SvUTF8(src)) {
    /* Already encoded */
    if (strict_utf8(aTHX_ obj)) {
        s = process_utf8(aTHX_ dst, s, e, check_sv, 1, 1, 0);
    }
        else {
            /* trust it and just copy the octets */
    	    sv_setpvn(dst,(char *)s,(e-s));
        s = e;
        }
    }
    else {
        /* Native bytes - can always encode */
        U8 *d = (U8 *) SvGROW(dst, 2*slen+1); /* +1 or assertion will botch */
        while (s < e) {
#ifdef append_utf8_from_native_byte
            append_utf8_from_native_byte(*s, &d);
            s++;
#else
            UV uv = NATIVE_TO_UNI((UV) *s);
            s++; /* Above expansion of NATIVE_TO_UNI() is safer this way. */
            if (UNI_IS_INVARIANT(uv))
                *d++ = (U8)UTF_TO_NATIVE(uv);
            else {
                *d++ = (U8)UTF8_EIGHT_BIT_HI(uv);
                *d++ = (U8)UTF8_EIGHT_BIT_LO(uv);
            }
#endif
        }
        SvCUR_set(dst, d- (U8 *)SvPVX(dst));
        *SvEND(dst) = '\0';
    }

    /* Clear out translated part of source unless asked not to */
    if (modify) {
    slen = e-s;
    if (slen) {
        sv_setpvn(src, (char*)s, slen);
    }
    SvCUR_set(src, slen);
    SvSETMAGIC(src);
    }
    SvPOK_only(dst);
    SvUTF8_off(dst);
    if (SvTAINTED(src)) SvTAINTED_on(dst); /* propagate taintedness */
    ST(0) = dst;
    XSRETURN(1);
}
#line 866 "Encode.c"
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__XS_renew); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__XS_renew)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "obj");
    {
	SV *	obj = ST(0)
;
#line 686 "Encode.xs"
{
    PERL_UNUSED_VAR(obj);
    XSRETURN(1);
}
#line 886 "Encode.c"
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Encode__XS_renewed); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__XS_renewed)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "obj");
    {
	SV *	obj = ST(0)
;
	int	RETVAL;
	dXSTARG;
#line 695 "Encode.xs"
    RETVAL = 0;
    PERL_UNUSED_VAR(obj);
#line 906 "Encode.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__XS_name); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__XS_name)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "obj");
    {
	SV *	obj = ST(0)
;
#line 704 "Encode.xs"
{
    encode_t *enc = INT2PTR(encode_t *, SvIV(SvRV(obj)));
    ST(0) = sv_2mortal(newSVpvn(enc->name[0],strlen(enc->name[0])));
    XSRETURN(1);
}
#line 928 "Encode.c"
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__XS_cat_decode); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__XS_cat_decode)
{
    dVAR; dXSARGS;
    if (items < 5 || items > 6)
       croak_xs_usage(cv,  "obj, dst, src, off, term, check_sv = &PL_sv_no");
    {
	SV *	obj = ST(0)
;
	SV *	dst = ST(1)
;
	SV *	src = ST(2)
;
	SV *	off = ST(3)
;
	SV *	term = ST(4)
;
	SV *	check_sv;
#line 719 "Encode.xs"
    int check;
    SV *fallback_cb;
    bool modify;
    encode_t *enc;
    STRLEN offset;
    int code = 0;
    U8 *s;
    STRLEN slen;
    SV *tmp;
#line 962 "Encode.c"

	if (items < 6)
	    check_sv = &PL_sv_no;
	else {
	    check_sv = ST(5)
;
	}
#line 729 "Encode.xs"
    SvGETMAGIC(src);
    SvGETMAGIC(check_sv);
    check = SvROK(check_sv) ? ENCODE_PERLQQ|ENCODE_LEAVE_SRC : SvIV_nomg(check_sv);
    fallback_cb = SvROK(check_sv) ? check_sv : &PL_sv_undef;
    modify = (check && !(check & ENCODE_LEAVE_SRC));
    enc = INT2PTR(encode_t *, SvIV(SvRV(obj)));
    offset = (STRLEN)SvIV(off);
#line 978 "Encode.c"
#line 737 "Encode.xs"
{
    if (!SvOK(src))
        XSRETURN_NO;
    s = modify ? (U8 *)SvPV_force_nomg(src, slen) : (U8 *)SvPV_nomg(src, slen);
    if (SvUTF8(src))
        utf8_safe_downgrade(aTHX_ &src, &s, &slen, modify);
    tmp = encode_method(aTHX_ enc, enc->t_utf8, src, s, slen, check,
                &offset, term, &code, fallback_cb);
    sv_catsv(dst, tmp);
    SvREFCNT_dec(tmp);
    SvIV_set(off, (IV)offset);
    if (code == ENCODE_FOUND_TERM) {
    ST(0) = &PL_sv_yes;
    }else{
    ST(0) = &PL_sv_no;
    }
    XSRETURN(1);
}
#line 998 "Encode.c"
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__XS_decode); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__XS_decode)
{
    dVAR; dXSARGS;
    if (items < 2 || items > 3)
       croak_xs_usage(cv,  "obj, src, check_sv = &PL_sv_no");
    {
	SV *	obj = ST(0)
;
	SV *	src = ST(1)
;
	SV *	check_sv;
#line 762 "Encode.xs"
    int check;
    SV *fallback_cb;
    bool modify;
    encode_t *enc;
    U8 *s;
    STRLEN slen;
#line 1023 "Encode.c"
	SV *	RETVAL;

	if (items < 3)
	    check_sv = &PL_sv_no;
	else {
	    check_sv = ST(2)
;
	}
#line 769 "Encode.xs"
    SvGETMAGIC(src);
    SvGETMAGIC(check_sv);
    check = SvROK(check_sv) ? ENCODE_PERLQQ|ENCODE_LEAVE_SRC : SvIV_nomg(check_sv);
    fallback_cb = SvROK(check_sv) ? check_sv : &PL_sv_undef;
    modify = (check && !(check & ENCODE_LEAVE_SRC));
    enc = INT2PTR(encode_t *, SvIV(SvRV(obj)));
#line 1039 "Encode.c"
#line 776 "Encode.xs"
{
    if (!SvOK(src))
        XSRETURN_UNDEF;
    s = modify ? (U8 *)SvPV_force_nomg(src, slen) : (U8 *)SvPV_nomg(src, slen);
    if (SvUTF8(src))
        utf8_safe_downgrade(aTHX_ &src, &s, &slen, modify);
    RETVAL = encode_method(aTHX_ enc, enc->t_utf8, src, s, slen, check,
              NULL, Nullsv, NULL, fallback_cb);
    SvUTF8_on(RETVAL);
}
#line 1051 "Encode.c"
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__XS_encode); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__XS_encode)
{
    dVAR; dXSARGS;
    if (items < 2 || items > 3)
       croak_xs_usage(cv,  "obj, src, check_sv = &PL_sv_no");
    {
	SV *	obj = ST(0)
;
	SV *	src = ST(1)
;
	SV *	check_sv;
#line 795 "Encode.xs"
    int check;
    SV *fallback_cb;
    bool modify;
    encode_t *enc;
    U8 *s;
    STRLEN slen;
#line 1078 "Encode.c"
	SV *	RETVAL;

	if (items < 3)
	    check_sv = &PL_sv_no;
	else {
	    check_sv = ST(2)
;
	}
#line 802 "Encode.xs"
    SvGETMAGIC(src);
    SvGETMAGIC(check_sv);
    check = SvROK(check_sv) ? ENCODE_PERLQQ|ENCODE_LEAVE_SRC : SvIV_nomg(check_sv);
    fallback_cb = SvROK(check_sv) ? check_sv : &PL_sv_undef;
    modify = (check && !(check & ENCODE_LEAVE_SRC));
    enc = INT2PTR(encode_t *, SvIV(SvRV(obj)));
#line 1094 "Encode.c"
#line 809 "Encode.xs"
{
    if (!SvOK(src))
        XSRETURN_UNDEF;
    s = modify ? (U8 *)SvPV_force_nomg(src, slen) : (U8 *)SvPV_nomg(src, slen);
    if (!SvUTF8(src))
        utf8_safe_upgrade(aTHX_ &src, &s, &slen, modify);
    RETVAL = encode_method(aTHX_ enc, enc->f_utf8, src, s, slen, check,
              NULL, Nullsv, NULL, fallback_cb);
}
#line 1105 "Encode.c"
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__XS_needs_lines); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__XS_needs_lines)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "obj");
    {
	SV *	obj = ST(0)
;
#line 825 "Encode.xs"
{
    /* encode_t *enc = INT2PTR(encode_t *, SvIV(SvRV(obj))); */
    PERL_UNUSED_VAR(obj);
    ST(0) = &PL_sv_no;
    XSRETURN(1);
}
#line 1129 "Encode.c"
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__XS_perlio_ok); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__XS_perlio_ok)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "obj");
    {
	SV *	obj = ST(0)
;
#line 836 "Encode.xs"
    SV *sv;
#line 1146 "Encode.c"
#line 838 "Encode.xs"
{
    /* encode_t *enc = INT2PTR(encode_t *, SvIV(SvRV(obj))); */
    /* require_pv(PERLIO_FILENAME); */

    PERL_UNUSED_VAR(obj);
    eval_pv("require PerlIO::encoding", 0);
    SPAGAIN;

    sv = get_sv("@", 0);
    if (SvTRUE(sv)) {
    ST(0) = &PL_sv_no;
    }else{
    ST(0) = &PL_sv_yes;
    }
    XSRETURN(1);
}
#line 1164 "Encode.c"
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__XS_mime_name); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__XS_mime_name)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "obj");
    {
	SV *	obj = ST(0)
;
#line 859 "Encode.xs"
    SV *sv;
#line 1181 "Encode.c"
#line 861 "Encode.xs"
{
    encode_t *enc = INT2PTR(encode_t *, SvIV(SvRV(obj)));
    SV *retval;
    eval_pv("require Encode::MIME::Name", 0);
    SPAGAIN;

    sv = get_sv("@", 0);
    if (SvTRUE(sv)) {
	ST(0) = &PL_sv_undef;
    }else{
	ENTER;
	SAVETMPS;
	PUSHMARK(sp);
	XPUSHs(sv_2mortal(newSVpvn(enc->name[0], strlen(enc->name[0]))));
	PUTBACK;
	call_pv("Encode::MIME::Name::get_mime_name", G_SCALAR);
	SPAGAIN;
	retval = newSVsv(POPs);
	PUTBACK;
	FREETMPS;
	LEAVE;
	/* enc->name[0] */
	ST(0) = retval;
    }
    XSRETURN(1);
}
#line 1209 "Encode.c"
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__bytes_to_utf8); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__bytes_to_utf8)
{
    dVAR; dXSARGS;
    if (items < 1)
       croak_xs_usage(cv,  "sv, ...");
    {
	SV *	sv = ST(0)
;
	I32	RETVAL;
	dXSTARG;
#line 896 "Encode.xs"
{
    SV * encoding = items == 2 ? ST(1) : Nullsv;

    if (encoding)
    RETVAL = _encoded_bytes_to_utf8(sv, SvPV_nolen(encoding));
    else {
    STRLEN len;
    U8*    s = (U8*)SvPV(sv, len);
    U8*    converted;

    converted = bytes_to_utf8(s, &len); /* This allocs */
    sv_setpvn(sv, (char *)converted, len);
    SvUTF8_on(sv); /* XXX Should we? */
    Safefree(converted);                /* ... so free it */
    RETVAL = len;
    }
}
#line 1244 "Encode.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__utf8_to_bytes); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__utf8_to_bytes)
{
    dVAR; dXSARGS;
    if (items < 1)
       croak_xs_usage(cv,  "sv, ...");
    {
	SV *	sv = ST(0)
;
	I32	RETVAL;
	dXSTARG;
#line 920 "Encode.xs"
{
    SV * to    = items > 1 ? ST(1) : Nullsv;
    SV * check = items > 2 ? ST(2) : Nullsv;

    if (to) {
    RETVAL = _encoded_utf8_to_bytes(sv, SvPV_nolen(to));
    } else {
    STRLEN len;
    U8 *s = (U8*)SvPV(sv, len);

    RETVAL = 0;
    if (SvTRUE(check)) {
        /* Must do things the slow way */
        U8 *dest;
            /* We need a copy to pass to check() */
        U8 *src  = s;
        U8 *send = s + len;
        U8 *d0;

        New(83, dest, len, U8); /* I think */
        d0 = dest;

        while (s < send) {
                if (*s < 0x80){
            *dest++ = *s++;
                } else {
            STRLEN ulen;
            UV uv = *s++;

            /* Have to do it all ourselves because of error routine,
               aargh. */
            if (!(uv & 0x40)){ goto failure; }
            if      (!(uv & 0x20)) { ulen = 2;  uv &= 0x1f; }
            else if (!(uv & 0x10)) { ulen = 3;  uv &= 0x0f; }
            else if (!(uv & 0x08)) { ulen = 4;  uv &= 0x07; }
            else if (!(uv & 0x04)) { ulen = 5;  uv &= 0x03; }
            else if (!(uv & 0x02)) { ulen = 6;  uv &= 0x01; }
            else if (!(uv & 0x01)) { ulen = 7;  uv = 0; }
            else                   { ulen = 13; uv = 0; }

            /* Note change to utf8.c variable naming, for variety */
            while (ulen--) {
            if ((*s & 0xc0) != 0x80){
                goto failure;
            } else {
                uv = (uv << 6) | (*s++ & 0x3f);
            }
          }
          if (uv > 256) {
          failure:
              call_failure(check, s, dest, src);
              /* Now what happens? */
          }
          *dest++ = (U8)uv;
        }
        }
        RETVAL = dest - d0;
        sv_usepvn(sv, (char *)dest, RETVAL);
        SvUTF8_off(sv);
    } else {
        RETVAL = (utf8_to_bytes(s, &len) ? len : 0);
    }
    }
}
#line 1327 "Encode.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode_is_utf8); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode_is_utf8)
{
    dVAR; dXSARGS;
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "sv, check = 0");
    {
	SV *	sv = ST(0)
;
	int	check;
#line 992 "Encode.xs"
    char *str;
    STRLEN len;
#line 1347 "Encode.c"
	bool	RETVAL;

	if (items < 2)
	    check = 0;
	else {
	    check = (int)SvIV(ST(1))
;
	}
#line 995 "Encode.xs"
{
    SvGETMAGIC(sv); /* SvGETMAGIC() can modify SvOK flag */
    str = SvOK(sv) ? SvPV_nomg(sv, len) : NULL; /* SvPV() can modify SvUTF8 flag */
    RETVAL = SvUTF8(sv) ? TRUE : FALSE;
    if (RETVAL && check && (!str || !is_utf8_string((U8 *)str, len)))
        RETVAL = FALSE;
}
#line 1364 "Encode.c"
	ST(0) = boolSV(RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__utf8_on); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__utf8_on)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "sv");
    {
	SV *	sv = ST(0)
;
	SV *	RETVAL;
#line 1009 "Encode.xs"
{
    SvGETMAGIC(sv);
    if (!SvTAINTED(sv) && SvPOKp(sv)) {
        if (SvTHINKFIRST(sv)) sv_force_normal(sv);
        RETVAL = newSViv(SvUTF8(sv));
        SvUTF8_on(sv);
        SvSETMAGIC(sv);
    } else {
        RETVAL = &PL_sv_undef;
    }
}
#line 1393 "Encode.c"
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Encode__utf8_off); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Encode__utf8_off)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "sv");
    {
	SV *	sv = ST(0)
;
	SV *	RETVAL;
#line 1027 "Encode.xs"
{
    SvGETMAGIC(sv);
    if (!SvTAINTED(sv) && SvPOKp(sv)) {
        if (SvTHINKFIRST(sv)) sv_force_normal(sv);
        RETVAL = newSViv(SvUTF8(sv));
        SvUTF8_off(sv);
        SvSETMAGIC(sv);
    } else {
        RETVAL = &PL_sv_undef;
    }
}
#line 1423 "Encode.c"
	RETVAL = sv_2mortal(RETVAL);
	ST(0) = RETVAL;
    }
    XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif
XS_EXTERNAL(boot_Encode); /* prototype to pass -Wmissing-prototypes */
XS_EXTERNAL(boot_Encode)
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

        newXS_deffile("Encode::utf8::decode_xs", XS_Encode__utf8_decode_xs);
        newXS_deffile("Encode::utf8::encode_xs", XS_Encode__utf8_encode_xs);
        (void)newXSproto_portable("Encode::XS::renew", XS_Encode__XS_renew, file, "$");
        (void)newXSproto_portable("Encode::XS::renewed", XS_Encode__XS_renewed, file, "$");
        (void)newXSproto_portable("Encode::XS::name", XS_Encode__XS_name, file, "$");
        (void)newXSproto_portable("Encode::XS::cat_decode", XS_Encode__XS_cat_decode, file, "$$$$$;$");
        (void)newXSproto_portable("Encode::XS::decode", XS_Encode__XS_decode, file, "$$;$");
        (void)newXSproto_portable("Encode::XS::encode", XS_Encode__XS_encode, file, "$$;$");
        (void)newXSproto_portable("Encode::XS::needs_lines", XS_Encode__XS_needs_lines, file, "$");
        (void)newXSproto_portable("Encode::XS::perlio_ok", XS_Encode__XS_perlio_ok, file, "$");
        (void)newXSproto_portable("Encode::XS::mime_name", XS_Encode__XS_mime_name, file, "$");
        (void)newXSproto_portable("Encode::_bytes_to_utf8", XS_Encode__bytes_to_utf8, file, "$;@");
        (void)newXSproto_portable("Encode::_utf8_to_bytes", XS_Encode__utf8_to_bytes, file, "$;@");
        (void)newXSproto_portable("Encode::is_utf8", XS_Encode_is_utf8, file, "$;$");
        (void)newXSproto_portable("Encode::_utf8_on", XS_Encode__utf8_on, file, "$");
        (void)newXSproto_portable("Encode::_utf8_off", XS_Encode__utf8_off, file, "$");

    /* Initialisation Section */

#line 1042 "Encode.xs"
{
    HV *stash = gv_stashpvn("Encode", strlen("Encode"), GV_ADD);
    newCONSTSUB(stash, "DIE_ON_ERR", newSViv(ENCODE_DIE_ON_ERR));
    newCONSTSUB(stash, "WARN_ON_ERR", newSViv(ENCODE_WARN_ON_ERR));
    newCONSTSUB(stash, "RETURN_ON_ERR", newSViv(ENCODE_RETURN_ON_ERR));
    newCONSTSUB(stash, "LEAVE_SRC", newSViv(ENCODE_LEAVE_SRC));
    newCONSTSUB(stash, "PERLQQ", newSViv(ENCODE_PERLQQ));
    newCONSTSUB(stash, "HTMLCREF", newSViv(ENCODE_HTMLCREF));
    newCONSTSUB(stash, "XMLCREF", newSViv(ENCODE_XMLCREF));
    newCONSTSUB(stash, "STOP_AT_PARTIAL", newSViv(ENCODE_STOP_AT_PARTIAL));
    newCONSTSUB(stash, "FB_DEFAULT", newSViv(ENCODE_FB_DEFAULT));
    newCONSTSUB(stash, "FB_CROAK", newSViv(ENCODE_FB_CROAK));
    newCONSTSUB(stash, "FB_QUIET", newSViv(ENCODE_FB_QUIET));
    newCONSTSUB(stash, "FB_WARN", newSViv(ENCODE_FB_WARN));
    newCONSTSUB(stash, "FB_PERLQQ", newSViv(ENCODE_FB_PERLQQ));
    newCONSTSUB(stash, "FB_HTMLCREF", newSViv(ENCODE_FB_HTMLCREF));
    newCONSTSUB(stash, "FB_XMLCREF", newSViv(ENCODE_FB_XMLCREF));
}
{
#include "def_t.exh"
}

#line 1500 "Encode.c"

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


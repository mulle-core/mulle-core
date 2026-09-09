/* String to double conversion, the inverse of mulle-dtostr.
 * Copyright (c) 2026 - Nat! - Mulle kybernetiK
 * Distributed under the MIT license (see LICENSE).
 *
 * The parser is locale independent. It never looks at `localeconv`, the
 * accepted characters are given by the caller in a `mulle__strtod_syntax`.
 */

#ifndef MULLE__STRTOD_H
#define MULLE__STRTOD_H

#include "_mulle-dtostr.h"


/* Pass as `len` for a NUL terminated string
 */
#define MULLE__STRTOD_NUL_TERMINATED   ((size_t) -1)

/* Number of decimal digits kept in the significand. Everything beyond is
 * dropped and flagged in `truncated`.
 */
#define MULLE__STRTOD_MAX_DIGITS       19

/* The decimal exponent is clamped to this range. Both ends are far outside
 * of the double range, so the converter can decide overflow or underflow
 * without looking at the digits again.
 */
#define MULLE__STRTOD_EXPONENT_LIMIT   9999


/* What may appear in the text. Everything not allowed simply terminates the
 * conversion, it is not an error. "+1" with `allow_plus` clear is therefore
 * not a number at all, whereas "1e5" with `allow_exponent_plus` clear is the
 * number 1 followed by the text "e5".
 */
enum
{
   mulle_strtod_allow_leading_whitespace_e = 0x0001,  //  " 1.0"
   mulle_strtod_allow_plus_e               = 0x0002,  //  "+1.0"
   mulle_strtod_allow_minus_e              = 0x0004,  //  "-1.0"
   mulle_strtod_allow_exponent_plus_e      = 0x0008,  //  "1e+5"
   mulle_strtod_allow_exponent_minus_e     = 0x0010,  //  "1e-5"
   mulle_strtod_allow_inf_e                = 0x0020,  //  "inf", "infinity"
   mulle_strtod_allow_nan_e                = 0x0040,  //  "nan", "nan(_1a)"
   mulle_strtod_allow_empty_integer_e      = 0x0080,  //  ".5"
   mulle_strtod_allow_empty_fraction_e     = 0x0100,  //  "5."
   mulle_strtod_case_sensitive_e           = 0x0200,  //  "INF", "1E5" fail
   mulle_strtod_allow_hex_e                = 0x0400   //  reserved, unused
};


/* The hand made "locale". `exponent` is a NUL padded set of single
 * characters, "e" or "ed" (Fortran) or "" for no exponent at all (CSS).
 * `grouping` is a single character or 0 for none. `group_size` is 3 for most
 * conventions, or 0 to accept a separator anywhere in the integer part.
 *
 * Multi byte separators (U+00A0 for French and SI) are not supported.
 */
struct mulle__strtod_syntax
{
   char       decimal_point;   //  '.' or ','
   char       grouping;        //  ',' '.' '\'' '_' ' ' or 0 for none
   uint8_t    group_size;      //  3, or 0 for "don't check positions"
   char       exponent[ 4];    //  "e" "ed" or "" for none
   uint32_t   flags;
};


/* Exactly what mulle_dtostr emits and nothing else
 */
static inline struct mulle__strtod_syntax   mulle__strtod_syntax_make_dtostr( void)
{
   struct mulle__strtod_syntax   syntax = { 0 };

   syntax.decimal_point = '.';
   syntax.exponent[ 0]  = 'e';
   syntax.flags         = mulle_strtod_allow_minus_e
                        | mulle_strtod_allow_exponent_plus_e
                        | mulle_strtod_allow_exponent_minus_e
                        | mulle_strtod_allow_inf_e
                        | mulle_strtod_allow_nan_e;
   return( syntax);
}


/* C locale, as `strtod` does it
 */
static inline struct mulle__strtod_syntax   mulle__strtod_syntax_make_c( void)
{
   struct mulle__strtod_syntax   syntax;

   syntax        = mulle__strtod_syntax_make_dtostr();
   syntax.flags |= mulle_strtod_allow_leading_whitespace_e
                 | mulle_strtod_allow_plus_e
                 | mulle_strtod_allow_empty_integer_e
                 | mulle_strtod_allow_empty_fraction_e;
   return( syntax);
}


/* JSON numbers: no "+1", no "inf", no "nan", no ".5", no "5."
 */
static inline struct mulle__strtod_syntax   mulle__strtod_syntax_make_json( void)
{
   struct mulle__strtod_syntax   syntax = { 0 };

   syntax.decimal_point = '.';
   syntax.exponent[ 0]  = 'e';
   syntax.flags         = mulle_strtod_allow_minus_e
                        | mulle_strtod_allow_exponent_plus_e
                        | mulle_strtod_allow_exponent_minus_e;
   return( syntax);
}


/* CSS 2.1 numbers: no exponent, ".5" is fine, "5." is not.
 * Note that CSS Syntax Level 3 does allow an exponent, so add "e" to
 * `exponent` if that is the CSS you mean.
 */
static inline struct mulle__strtod_syntax   mulle__strtod_syntax_make_css( void)
{
   struct mulle__strtod_syntax   syntax = { 0 };

   syntax.decimal_point = '.';
   syntax.flags         = mulle_strtod_allow_plus_e
                        | mulle_strtod_allow_minus_e
                        | mulle_strtod_allow_empty_integer_e;
   return( syntax);
}


/* 1,234,567.89
 */
static inline struct mulle__strtod_syntax   mulle__strtod_syntax_make_en( void)
{
   struct mulle__strtod_syntax   syntax;

   syntax            = mulle__strtod_syntax_make_c();
   syntax.grouping   = ',';
   syntax.group_size = 3;
   return( syntax);
}


/* 1.234.567,89
 */
static inline struct mulle__strtod_syntax   mulle__strtod_syntax_make_de( void)
{
   struct mulle__strtod_syntax   syntax;

   syntax               = mulle__strtod_syntax_make_c();
   syntax.decimal_point = ',';
   syntax.grouping      = '.';
   syntax.group_size    = 3;
   return( syntax);
}


/* 1'234'567.89
 */
static inline struct mulle__strtod_syntax   mulle__strtod_syntax_make_ch( void)
{
   struct mulle__strtod_syntax   syntax;

   syntax            = mulle__strtod_syntax_make_c();
   syntax.grouping   = '\'';
   syntax.group_size = 3;
   return( syntax);
}


/* Catches combinations that can not work, like a grouping character that is
 * also the decimal point. Returns 1 if the syntax is usable.
 */
MULLE__DTOSTR_GLOBAL
int   mulle__strtod_syntax_is_valid( const struct mulle__strtod_syntax *syntax);


/*
 * Reads the longest prefix of `s` that is a number in `syntax` and returns it
 * as a decimal, the same intermediate form that `mulle_dtostr_decompose`
 * produces. Pass `MULLE__STRTOD_NUL_TERMINATED` as `len` for a C string,
 * otherwise `len` bounds the text and no NUL is needed. `syntax` may be NULL,
 * which means `mulle__strtod_syntax_make_c`.
 *
 * `endptr` may be NULL. It is set to the first character not consumed, which
 * is `s` itself if nothing was consumed. Nothing consumed is reported as
 * `special` being `mulle_dtostr_invalid_e`.
 *
 * The exponent is clamped to +/- MULLE__STRTOD_EXPONENT_LIMIT, the
 * significand holds at most MULLE__STRTOD_MAX_DIGITS digits and `truncated`
 * says whether a non zero digit was dropped.
 */
MULLE__DTOSTR_GLOBAL
struct mulle_dtostr_decimal
   mulle_strtod_parse( const char *s,
                       size_t len,
                       const struct mulle__strtod_syntax *syntax,
                       char **endptr);


/* Status codes returned by mulle_strtod_scan.
 */
enum
{
   mulle_strtod_ok_e            = 0,
   mulle_strtod_no_conversion_e = 1,
   mulle_strtod_overflow_e      = 2,
   mulle_strtod_underflow_e     = 3
};


/*
 * Compose a double from a canonical decimal (the output of `decompose` or the
 * parser). This is the bisection converter: it uses `mulle_dtostr_decompose`
 * on candidate doubles until one roundtrips exactly. For decimals that are not
 * the shortest representation of any double, the closest double is returned
 * (possibly 1 ulp off).
 *
 * `mulle_dtostr_inf_e`, `mulle_dtostr_nan_e`, `mulle_dtostr_zero_e`, and
 * `mulle_dtostr_invalid_e` are handled directly without searching.
 */
MULLE__DTOSTR_GLOBAL
double   mulle_strtod_compose( struct mulle_dtostr_decimal decimal);


/*
 * Combined parse + convert in one call. Returns a status code and sets
 * `*p_value` on success. `endptr`, `syntax`, `len` behave as in
 * `mulle_strtod_parse`. `errno` is never touched.
 *
 * KNOWN LIMITATION: the parser keeps at most 19 significant digits. Input
 * produced by `mulle_dtostr` (at most 17 digits) roundtrips bit exactly.
 * Arbitrary text with more than 19 significant digits may land 1 ulp away
 * from the correctly rounded double in about 0.1% of cases, specifically
 * when the true value falls very close to the midpoint between two adjacent
 * doubles and the distinguishing information is in the dropped digits.
 */
MULLE__DTOSTR_GLOBAL
int   mulle_strtod_scan( const char *s,
                         size_t len,
                         const struct mulle__strtod_syntax *syntax,
                         double *p_value,
                         char **endptr);


/*
 * Drop-in for libc `strtod`, using the C locale syntax.
 */
static inline double   mulle_strtod( const char *s, char **endptr)
{
   double   value;

   mulle_strtod_scan( s,
                      MULLE__STRTOD_NUL_TERMINATED,
                      (void *) 0,
                      &value,
                      endptr);
   return( value);
}


/*
 * Like mulle_strtod but with an explicit length bound, no NUL needed.
 */
static inline double   mulle_strtod_len( const char *s, size_t len, char **endptr)
{
   double   value;

   mulle_strtod_scan( s,
                      len,
                      (void *) 0,
                      &value,
                      endptr);
   return( value);
}


#endif  /* MULLE__STRTOD_H */

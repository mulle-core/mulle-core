/* Implementation of the Schubfach algorithm:
 * https://github.com/vitaut/zmij
 * Copyright (c) 2025 - present, Victor Zverovich
 * Copyright (c) 2025 - C conversion and additional code by Nat!
 * Distributed under the MIT license (see LICENSE).
 */

#ifndef MULLE__DTOSTR_H
#define MULLE__DTOSTR_H

#define MULLE__DTOSTR_VERSION   ((0 << 24) | (2 << 8) | 0)

#include <mulle-c11/mulle-c11.h>
#include <stdint.h>
#include <stddef.h>


#if defined( MULLE__DTOSTR_BUILD) || defined( MULLE__CORE_BUILD)
# define MULLE__DTOSTR_GLOBAL    MULLE_C_GLOBAL
#else
# if defined( MULLE_DTOSTR_INCLUDE_DYNAMIC) || (defined( MULLE_INCLUDE_DYNAMIC) && ! defined( MULLE_DTOSTR_INCLUDE_STATIC))
#  define MULLE__DTOSTR_GLOBAL   MULLE_C_EXTERN_GLOBAL
# else
#  define MULLE__DTOSTR_GLOBAL   extern
# endif
#endif


#define MULLE__DTOSTR_BUFFER_SIZE 25

/* Values for the `special` field of struct mulle_dtostr_decimal.
 * `mulle_dtostr_invalid_e` is never produced by mulle_dtostr_decompose, only
 * by the parser in mulle-strtod.h.
 */
enum
{
   mulle_dtostr_normal_e  = 0,
   mulle_dtostr_inf_e     = 1,
   mulle_dtostr_nan_e     = 2,
   mulle_dtostr_zero_e    = 3,
   mulle_dtostr_invalid_e = 4
};


/* Intermediate decimal representation of a double value.
 * Fits in 128 bits (two uint64_t) for efficient return by value.
 */
struct mulle_dtostr_decimal
{
  uint64_t   significand;  /* decimal significand/mantissa */
  int16_t    exponent;     /* decimal exponent */
  uint8_t    sign;         /* 0 = positive, 1 = negative */
  uint8_t    special;      /* mulle_dtostr_normal_e and friends */
  uint8_t    digits;       /* significand digit count, mulle_strtod_parse only */
  uint8_t    truncated;    /* a non zero digit was dropped, ditto */
  uint16_t   _padding;     /* reserved for alignment */
};

/* Decomposes a double into decimal representation.
 * Returns the intermediate form suitable for custom formatting.
 */

MULLE__DTOSTR_GLOBAL
struct mulle_dtostr_decimal   mulle_dtostr_decompose( double value);

/*
 * Writes the shortest correctly rounded decimal representation of `value` to
 * `buffer`. `buffer` should point to a buffer of size MULLE__DTOSTR_BUFFER_SIZE
 * or larger. Returns the length of the generated string (excluding null terminator).
 *
 * `mulle_strtod` reads the result back as the identical bit pattern, which is
 * checked over a wide sample in test_strtod_roundtrip. libc `strtod` does so
 * too on the platforms tested, but only `mulle_strtod` is guaranteed to.
 */
MULLE__DTOSTR_GLOBAL
size_t   mulle_dtostr( double value, char *buffer);


#endif  /* MULLE__DTOSTR_H */

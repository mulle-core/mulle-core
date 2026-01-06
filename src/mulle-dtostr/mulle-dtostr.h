/* Implementation of the Schubfach algorithm:
 * https://github.com/vitaut/zmij
 * Copyright (c) 2025 - present, Victor Zverovich
 * Copyright (c) 2025 - C conversion and additional code by Nat!
 * Distributed under the MIT license (see LICENSE).
 */

#ifndef MULLE__DTOSTR_H
#define MULLE__DTOSTR_H

#define MULLE__DTOSTR_VERSION   ((0 << 24) | (1 << 8) | 0)

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MULLE__DTOSTR_BUFFER_SIZE 25

/* Intermediate decimal representation of a double value.
 * Fits in 128 bits (two uint64_t) for efficient return by value.
 */
struct mulle_dtostr_decimal
{
  uint64_t   significand;  /* decimal significand/mantissa */
  int16_t    exponent;     /* decimal exponent */
  uint8_t    sign;         /* 0 = positive, 1 = negative */
  uint8_t    special;      /* 0 = normal, 1 = inf, 2 = nan, 3 = zero */
  uint32_t   _padding;     /* reserved for alignment */
};

/* Decomposes a double into decimal representation.
 * Returns the intermediate form suitable for custom formatting.
 */
struct mulle_dtostr_decimal   mulle_dtostr_decompose( double value);

/*
 * Writes the shortest correctly rounded decimal representation of `value` to
 * `buffer`. `buffer` should point to a buffer of size MULLE__DTOSTR_BUFFER_SIZE
 * or larger. Returns the length of the generated string (excluding null terminator).
 * It is claimed that you get the exact same double value back
 * when using strtod (at least on linux, else we'd need to implement our own.)
 */
size_t   mulle_dtostr( double value, char *buffer);

#ifdef __cplusplus
}
#endif

#endif  /* MULLE__DTOSTR_H */

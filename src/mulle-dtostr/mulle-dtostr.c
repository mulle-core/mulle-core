/* A double-to-string conversion algorithm based on Schubfach (zmij variant).
 * Copyright (c) 2025 - present, Victor Zverovich
 * Copyright (c) 2025 - C conversion by Nat!
 * Distributed under the MIT license (see LICENSE).
 * 
 * This implementation ports the zmij algorithm (an optimized Schubfach variant)
 * from C++ to C while maintaining the original mulle-dtostr API.
 */

#include "mulle-dtostr.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct uint128 {
  uint64_t hi;
  uint64_t lo;
};

#ifdef __SIZEOF_INT128__
typedef unsigned __int128 uint128_t;
#else
typedef struct uint128 uint128_t;
#endif

static inline uint128_t umul128(uint64_t a, uint64_t b) {
#ifdef __SIZEOF_INT128__
    return (unsigned __int128)a * b;
#else
    uint64_t a_lo = (uint32_t)a, a_hi = a >> 32;
    uint64_t b_lo = (uint32_t)b, b_hi = b >> 32;
    uint64_t p0 = a_lo * b_lo;
    uint64_t p1 = a_lo * b_hi;
    uint64_t p2 = a_hi * b_lo;
    uint64_t p3 = a_hi * b_hi;
    uint64_t cy = (p0 >> 32) + (uint32_t)p1 + (uint32_t)p2;
    struct uint128 result = {p3 + (p1 >> 32) + (p2 >> 32) + (cy >> 32), p0 + (cy << 32)};
    return result;
#endif
}

static uint64_t umul192_upper64_modified(uint64_t pow10_hi, uint64_t pow10_lo, uint64_t scaled_sig) {
#ifdef __SIZEOF_INT128__
    unsigned __int128 x = umul128(pow10_lo, scaled_sig);
    uint64_t x_hi = (uint64_t)(x >> 64);
    unsigned __int128 y = umul128(pow10_hi, scaled_sig);
    uint64_t z = ((uint64_t)y >> 1) + x_hi;
    uint64_t result = (uint64_t)(y >> 64) + (z >> 63);
    const uint64_t mask = ((uint64_t)1 << 63) - 1;
    return result | (((z & mask) + mask) >> 63);
#else
    struct uint128 x = umul128(pow10_lo, scaled_sig);
    uint64_t x_hi = x.hi;
    struct uint128 y = umul128(pow10_hi, scaled_sig);
    uint64_t z = (y.lo >> 1) + x_hi;
    uint64_t result = y.hi + (z >> 63);
    const uint64_t mask = ((uint64_t)1 << 63) - 1;
    return result | (((z & mask) + mask) >> 63);
#endif
}

/* Power table */
static const struct uint128 pow10_significands[] = {
#include "pow10_table_data.inc"
};

struct significand_result {
   char  *start;
   int   length;
};

/* Forward declarations */
static size_t write_mulle(char* buffer, uint64_t dec_sig, int dec_exp);
static struct significand_result write_significand(char* temp_buffer, uint64_t value);
static void write2digits(char* buffer, uint32_t value);


/* Helper for fast 2-digit writing */

static const char digits2_data[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

static const char* digits2(size_t value) {
  return &digits2_data[value * 2];
}

static void write2digits(char* buffer, uint32_t value) {
  memcpy(buffer, digits2(value), 2);
}

static struct significand_result write_significand(char* temp_buffer, uint64_t value) {
   struct significand_result result;
   char                      *p;
   char                      *end;
   uint64_t                  n;
   
   /* Handle zero */
   if( value == 0)
   {
      temp_buffer[0] = '0';
      result.start = temp_buffer;
      result.length = 1;
      return( result);
   }
   
   /* Write digits backwards from end of buffer */
   end = temp_buffer + 31;
   p = end;
   n = value;
   
   /* Extract digits (writes backwards, so first digit written is last digit of number) */
   while( n > 0)
   {
      *--p = '0' + (n % 10);
      n /= 10;
   }
   
   /* Now p points to first significant digit, end points past last digit written */
   /* Strip trailing zeros from the NUMBER (which appear at end-1, end-2, etc.) */
   while( end > p + 1 && *(end - 1) == '0')
      end--;
   
   result.start = p;
   result.length = end - p;
   
   return( result);
}

static size_t write_mulle(char* buffer, uint64_t dec_sig, int dec_exp) {
  char                      temp[32];
  struct significand_result sig;
  int                       abs_exp;
  int                       actual_exp;
  int                       i;
  int                       int_digits;
  
  /* Write significand to temp buffer (no leading zeros, trailing zeros stripped) */
  sig = write_significand( temp, dec_sig);
  
  /* Calculate actual exponent: decompose gives exp such that value = sig * 10^exp
   * where sig is a 16 or 17-digit integer. The first significant digit written
   * by write_significand represents the highest order digit of dec_sig */
  actual_exp = dec_sig >= 10000000000000000ULL 
               ? dec_exp + 16  /* 17-digit: first digit has weight 10^16 */
               : dec_exp + 15; /* 16-digit: first digit has weight 10^15 */
  
  /* Choose shortest format */
  if( actual_exp >= -4 && actual_exp <= 6)
  {
    /* Use fixed-point notation */
    if( actual_exp >= 0)
    {
      /* Integer or decimal >= 1: 12.2, 122, etc. */
      if( actual_exp < sig.length - 1)
      {
        /* Need decimal point: 12.2 */
        int_digits = actual_exp + 1;
        memcpy( buffer, sig.start, int_digits);
        buffer[ int_digits] = '.';
        memcpy( buffer + int_digits + 1, sig.start + int_digits, sig.length - int_digits);
        buffer[ sig.length + 1] = '\0';
        return( sig.length + 1);
      }
      /* Pure integer: 122 */
      memcpy( buffer, sig.start, sig.length);
      for( i = 0; i < actual_exp - sig.length + 1; i++)
        buffer[ sig.length + i] = '0';
      buffer[ sig.length + actual_exp - sig.length + 1] = '\0';
      return( sig.length + actual_exp - sig.length + 1);
    }
    /* Decimal < 1: 0.122 */
    buffer[0] = '0';
    buffer[1] = '.';
    for( i = 0; i < -actual_exp - 1; i++)
      buffer[ 2 + i] = '0';
    memcpy( buffer + 2 + (-actual_exp - 1), sig.start, sig.length);
    buffer[ 2 + (-actual_exp - 1) + sig.length] = '\0';
    return( 2 + (-actual_exp - 1) + sig.length);
  }
  
  /* Use scientific notation */
  buffer[0] = sig.start[0];
  if( sig.length > 1)
  {
    buffer[1] = '.';
    memcpy( buffer + 2, sig.start + 1, sig.length - 1);
    buffer[ sig.length + 1] = 'e';
    buffer[ sig.length + 2] = actual_exp >= 0 ? '+' : '-';
    abs_exp = actual_exp >= 0 ? actual_exp : -actual_exp;
    if( abs_exp >= 100)
    {
      buffer[ sig.length + 3] = '0' + (abs_exp / 100);
      write2digits( buffer + sig.length + 4, abs_exp % 100);
      buffer[ sig.length + 6] = '\0';
      return( sig.length + 6);
    }
    write2digits( buffer + sig.length + 3, abs_exp);
    buffer[ sig.length + 5] = '\0';
    return( sig.length + 5);
  }
  buffer[1] = '.';
  buffer[2] = 'e';
  buffer[3] = actual_exp >= 0 ? '+' : '-';
  abs_exp = actual_exp >= 0 ? actual_exp : -actual_exp;
  if( abs_exp >= 100)
  {
    buffer[4] = '0' + (abs_exp / 100);
    write2digits( buffer + 5, abs_exp % 100);
    buffer[7] = '\0';
    return( 7);
  }
  write2digits( buffer + 4, abs_exp);
  buffer[6] = '\0';
  return( 6);
}

struct mulle_dtostr_decimal   mulle_dtostr_decompose(double value) {
    struct mulle_dtostr_decimal result = {0};
    
    uint64_t bits = 0;
    memcpy(&bits, &value, sizeof(value));
    
    result.sign = (bits >> 63) & 1;
    
    const int num_sig_bits = 52;
    const int exp_mask = 0x7ff;
    int bin_exp = (int)(bits >> num_sig_bits) & exp_mask;
    uint64_t bin_sig = bits & (((uint64_t)1 << num_sig_bits) - 1);
    
    /* Handle special cases */
    if (bin_exp == 0x7ff) {
        result.special = bin_sig == 0 ? 1 : 2; /* inf : nan */
        return result;
    }
    
    if (bin_exp == 0 && bin_sig == 0) {
        result.special = 3; /* zero */
        return result;
    }
    
    /* Run the same zmij algorithm to get decimal representation */
    const uint64_t implicit_bit = (uint64_t)1 << num_sig_bits;
    int regular = bin_sig != 0;
    
    if (((bin_exp + 1) & exp_mask) <= 1) {
        if (bin_exp != 0) {
            result.special = bin_sig == 0 ? 1 : 2;
            return result;
        }
        bin_sig ^= implicit_bit;
        bin_exp = 1;
        regular = 1;
    }
    bin_sig ^= implicit_bit;
    bin_exp -= num_sig_bits + 1023;
    
    /* Use zmij algorithm to get decimal significand and exponent */
    uint64_t bin_sig_shifted = bin_sig << 2;
    uint64_t lower = bin_sig_shifted - (regular + 1);
    uint64_t upper = bin_sig_shifted + 2;
    
    const int log10_3_over_4_sig = -131008;
    const int log10_2_sig = 315653;
    const int log10_2_exp = 20;
    
    int dec_exp = (bin_exp * log10_2_sig + (!regular) * log10_3_over_4_sig) >> log10_2_exp;
    
    const int dec_exp_min = -292;
    struct uint128 pow10_entry = pow10_significands[-dec_exp - dec_exp_min];
    uint64_t pow10_hi = pow10_entry.hi;
    uint64_t pow10_lo = pow10_entry.lo;
    
    const int log2_pow10_sig = 217707;
    const int log2_pow10_exp = 16;
    int pow10_bin_exp = -dec_exp * log2_pow10_sig >> log2_pow10_exp;
    int shift = bin_exp + pow10_bin_exp + 2;
    
    uint64_t bin_sig_lsb = bin_sig & 1;
    lower = umul192_upper64_modified(pow10_hi, pow10_lo, lower << shift) + bin_sig_lsb;
    upper = umul192_upper64_modified(pow10_hi, pow10_lo, upper << shift) - bin_sig_lsb;
    
    uint64_t shorter = 10 * ((upper >> 2) / 10);
    if ((shorter << 2) >= lower) {
        result.significand = shorter;
        result.exponent = dec_exp;
        result.special = 0;
        return result;
    }
    
    uint64_t scaled_sig = umul192_upper64_modified(pow10_hi, pow10_lo, bin_sig_shifted << shift);
    uint64_t dec_sig_under = scaled_sig >> 2;
    uint64_t dec_sig_over = dec_sig_under + 1;
    
    int64_t cmp = (int64_t)(scaled_sig - ((dec_sig_under + dec_sig_over) << 1));
    int under_closer = cmp < 0 || (cmp == 0 && (dec_sig_under & 1) == 0);
    int under_in = (dec_sig_under << 2) >= lower;
    
    result.significand = (under_closer && under_in) ? dec_sig_under : dec_sig_over;
    result.exponent = dec_exp;
    result.special = 0;
    return result;
}

size_t   mulle_dtostr(double value, char* buffer) {
  struct mulle_dtostr_decimal dec;
  char                      *start;
  size_t                    len;
  
  dec   = mulle_dtostr_decompose( value);
  start = buffer;
  
  /* Handle sign */
  if( dec.sign)
  {
    *buffer = '-';
    buffer++;
  }
  
  /* Handle special cases */
  switch( dec.special)
  {
  case 1:  /* inf */
    memcpy( buffer, "inf", 4);
    return( buffer - start + 3);
    
  case 2:  /* nan */
    memcpy( buffer, "nan", 4);
    return( buffer - start + 3);
    
  case 3:  /* zero */
    memcpy( buffer, "0", 2);
    return( buffer - start + 1);
  }
  
  /* Format normal values */
  len = write_mulle( buffer, dec.significand, dec.exponent);
  return( buffer - start + len);
}


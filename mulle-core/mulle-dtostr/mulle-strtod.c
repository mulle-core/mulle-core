/* String to double conversion, the inverse of mulle-dtostr.
 * Copyright (c) 2026 - Nat! - Mulle kybernetiK
 * Distributed under the MIT license (see LICENSE).
 *
 * This file holds the lexer only. It turns text into a
 * `struct mulle_dtostr_decimal` and touches no floating point at all.
 */

#include "_mulle-strtod.h"

#include <assert.h>
#include <string.h>


/* Infinity and NaN are built from their bit patterns rather than from the
 * HUGE_VAL and NAN macros. NAN is not guaranteed to exist, computing a NaN
 * arithmetically raises an invalid operation, and neither is needed when the
 * bits are known. This also keeps the file free of math.h and libm.
 */
#define DOUBLE_SIGN_BITS   0x8000000000000000ull
#define DOUBLE_INF_BITS    0x7FF0000000000000ull
#define DOUBLE_NAN_BITS    0x7FF8000000000000ull

/* The largest positive finite double as a raw bit pattern
 */
#define DOUBLE_MAX_BITS    0x7FEFFFFFFFFFFFFFull


/* Running state of a parse. `dropped` counts digits that did not fit into the
 * significand, `frac_count` counts every digit after the decimal point,
 * leading zeroes included. The decimal exponent falls out of the two as
 * `dropped - frac_count`, see mulle_strtod_parse.
 */
struct parse_state
{
   struct mulle__strtod_syntax   *syntax;
   char                          *p;
   char                          *end;
   uint64_t                      significand;
   int                           digits;
   int                           dropped;
   int                           frac_count;
   int                           int_count;
   int                           nonzero;
   int                           truncated;
};


static int   is_digit( char c)
{
   return( c >= '0' && c <= '9');
}


static int   is_space( char c)
{
   switch( c)
   {
   case ' '  :
   case '\t' :
   case '\n' :
   case '\v' :
   case '\f' :
   case '\r' :
      return( 1);
   }
   return( 0);
}


static char   to_lower( char c)
{
   return( (c >= 'A' && c <= 'Z') ? (char) (c + ('a' - 'A')) : c);
}


static int   is_alpha( char c)
{
   char   lower;

   lower = to_lower( c);
   return( lower >= 'a' && lower <= 'z');
}


/* `word` must be given in lowercase. Returns the number of characters
 * matched, or 0 for no match.
 */
static size_t   match_word( char *p, char *end, char *word, int case_sensitive)
{
   size_t   i;
   size_t   len;

   len = strlen( word);
   if( (size_t) (end - p) < len)
      return( 0);

   for( i = 0; i < len; i++)
   {
      if( case_sensitive)
      {
         if( p[ i] != word[ i])
            return( 0);
      }
      else
         if( to_lower( p[ i]) != word[ i])
            return( 0);
   }
   return( len);
}


static int   is_exponent_char( struct mulle__strtod_syntax *syntax, char c)
{
   char   d;
   int    case_sensitive;
   int    i;

   case_sensitive = (syntax->flags & mulle_strtod_case_sensitive_e) != 0;
   if( ! case_sensitive)
      c = to_lower( c);

   for( i = 0; i < (int) sizeof( syntax->exponent); i++)
   {
      d = syntax->exponent[ i];
      if( ! d)
         break;
      if( ! case_sensitive)
         d = to_lower( d);
      if( d == c)
         return( 1);
   }
   return( 0);
}


static void   add_digit( struct parse_state *state, char c, int fraction)
{
   if( fraction)
      state->frac_count++;
   else
      state->int_count++;

   /* A leading zero carries no value. In the fraction it still shifts the
    * exponent, but that is `frac_count` above and not our business here.
    */
   if( c == '0' && ! state->nonzero)
      return;

   state->nonzero = 1;

   if( state->digits < MULLE__STRTOD_MAX_DIGITS)
   {
      state->significand = state->significand * 10 + (uint64_t) (c - '0');
      state->digits++;
      return;
   }

   state->dropped++;
   if( c != '0')
      state->truncated = 1;
}


/* "inf"/"infinity" and "nan"/"nan(n-char-sequence)". Returns one of the
 * mulle_dtostr_..._e specials, or mulle_dtostr_normal_e for no match.
 */
static int   parse_special( struct parse_state *state)
{
   size_t     n;
   int        case_sensitive;
   uint32_t   flags;
   char       *p;

   flags          = state->syntax->flags;
   case_sensitive = (flags & mulle_strtod_case_sensitive_e) != 0;

   if( flags & mulle_strtod_allow_inf_e)
   {
      n = match_word( state->p, state->end, "infinity", case_sensitive);
      if( ! n)
         n = match_word( state->p, state->end, "inf", case_sensitive);
      if( n)
      {
         state->p = &state->p[ n];
         return( mulle_dtostr_inf_e);
      }
   }

   if( flags & mulle_strtod_allow_nan_e)
   {
      n = match_word( state->p, state->end, "nan", case_sensitive);
      if( n)
      {
         state->p = &state->p[ n];
         if( state->p < state->end && *state->p == '(')
         {
            p = &state->p[ 1];
            while( p < state->end && (is_digit( *p) || is_alpha( *p) || *p == '_'))
               p++;
            if( p < state->end && *p == ')')
               state->p = &p[ 1];
         }
         return( mulle_dtostr_nan_e);
      }
   }

   return( mulle_dtostr_normal_e);
}


/* Digits before the decimal point, with optional grouping. A separator is
 * only accepted between digits and, if `group_size` is set, only on a group
 * boundary. If the last group turns out to be short, the number ends at the
 * last valid group boundary instead, so "1,234,56" is 1234 with the rest
 * left over.
 */
static void   parse_integer( struct parse_state *state)
{
   struct parse_state   memo;
   char                 grouping;
   int                  group_size;
   int                  groups;
   int                  run;

   grouping   = state->syntax->grouping;
   group_size = (int) state->syntax->group_size;
   groups     = 0;
   run        = 0;
   memo       = *state;

   for(;;)
   {
      if( state->p < state->end && is_digit( *state->p))
      {
         add_digit( state, *state->p, 0);
         state->p++;
         run++;
         continue;
      }

      if( ! grouping || state->p >= state->end || *state->p != grouping)
         break;
      if( ! run)
         break;
      if( &state->p[ 1] >= state->end || ! is_digit( state->p[ 1]))
         break;
      if( group_size && (groups ? run != group_size : run > group_size))
         break;

      memo = *state;    /* the number is valid up to this separator */
      state->p++;
      groups++;
      run = 0;
   }

   if( groups && group_size && run != group_size)
      *state = memo;
}


static void   parse_fraction( struct parse_state *state)
{
   while( state->p < state->end && is_digit( *state->p))
   {
      add_digit( state, *state->p, 1);
      state->p++;
   }
}


/* `state->p` only moves if the exponent is syntactically complete, so "1e"
 * is the number 1 with the 'e' left over.
 */
static int   parse_exponent( struct parse_state *state)
{
   char       *p;
   int        negative;
   int        value;
   uint32_t   flags;

   if( ! state->syntax->exponent[ 0])
      return( 0);
   if( state->p >= state->end || ! is_exponent_char( state->syntax, *state->p))
      return( 0);

   flags    = state->syntax->flags;
   negative = 0;
   p        = &state->p[ 1];

   if( p < state->end)
   {
      if( *p == '+' && (flags & mulle_strtod_allow_exponent_plus_e))
         p++;
      else
         if( *p == '-' && (flags & mulle_strtod_allow_exponent_minus_e))
         {
            negative = 1;
            p++;
         }
   }

   if( p >= state->end || ! is_digit( *p))
      return( 0);

   value = 0;
   while( p < state->end && is_digit( *p))
   {
      if( value <= MULLE__STRTOD_EXPONENT_LIMIT)
         value = value * 10 + (*p - '0');
      p++;
   }

   state->p = p;
   return( negative ? -value : value);
}


int   mulle__strtod_syntax_is_valid( struct mulle__strtod_syntax *syntax)
{
   char   c;
   int    i;

   if( ! syntax)
      return( 0);
   if( ! syntax->decimal_point || is_digit( syntax->decimal_point))
      return( 0);
   if( syntax->decimal_point == '+' || syntax->decimal_point == '-')
      return( 0);
   if( syntax->grouping)
   {
      if( is_digit( syntax->grouping) || syntax->grouping == syntax->decimal_point)
         return( 0);
      if( syntax->grouping == '+' || syntax->grouping == '-')
         return( 0);
   }
   else
      if( syntax->group_size)
         return( 0);

   for( i = 0; i < (int) sizeof( syntax->exponent); i++)
   {
      c = syntax->exponent[ i];
      if( ! c)
         break;
      if( is_digit( c) || c == '+' || c == '-')
         return( 0);
      if( c == syntax->decimal_point || c == syntax->grouping)
         return( 0);
   }
   return( 1);
}


struct mulle_dtostr_decimal
   mulle_strtod_parse( char *s,
                       size_t len,
                       struct mulle__strtod_syntax *syntax,
                       char **endptr)
{
   struct mulle_dtostr_decimal   decimal = { 0 };
   struct mulle__strtod_syntax   fallback;
   struct parse_state            state   = { 0 };
   char                          *memo;
   int                           exponent;
   int                           special;
   uint32_t                      flags;

   if( ! syntax)
   {
      fallback = mulle__strtod_syntax_make_c();
      syntax   = &fallback;
   }
   if( len == MULLE__STRTOD_NUL_TERMINATED)
      len = s ? strlen( s) : 0;

   state.syntax = syntax;
   state.p      = s;
   state.end    = &s[ len];
   flags        = syntax->flags;

   if( flags & mulle_strtod_allow_leading_whitespace_e)
      while( state.p < state.end && is_space( *state.p))
         state.p++;

   if( state.p < state.end)
   {
      if( *state.p == '-' && (flags & mulle_strtod_allow_minus_e))
      {
         decimal.sign = 1;
         state.p++;
      }
      else
         if( *state.p == '+' && (flags & mulle_strtod_allow_plus_e))
            state.p++;
   }

   special = parse_special( &state);
   if( special != mulle_dtostr_normal_e)
   {
      decimal.special = (uint8_t) special;
      goto done;
   }

   parse_integer( &state);

   if( state.p < state.end &&
       syntax->decimal_point &&
       *state.p == syntax->decimal_point)
   {
      if( state.int_count || (flags & mulle_strtod_allow_empty_integer_e))
      {
         memo = state.p;
         state.p++;
         parse_fraction( &state);
         if( ! state.frac_count && ! (flags & mulle_strtod_allow_empty_fraction_e))
            state.p = memo;
      }
   }

   if( ! state.int_count && ! state.frac_count)
   {
      decimal.sign    = 0;
      decimal.special = mulle_dtostr_invalid_e;
      state.p         = s;
      goto done;
   }

   exponent = parse_exponent( &state) + state.dropped - state.frac_count;

   if( exponent > MULLE__STRTOD_EXPONENT_LIMIT)
      exponent = MULLE__STRTOD_EXPONENT_LIMIT;
   else
      if( exponent < -MULLE__STRTOD_EXPONENT_LIMIT)
         exponent = -MULLE__STRTOD_EXPONENT_LIMIT;

   if( ! state.digits)
   {
      decimal.special = mulle_dtostr_zero_e;
      goto done;
   }

   decimal.significand = state.significand;
   decimal.exponent    = (int16_t) exponent;
   decimal.digits      = (uint8_t) state.digits;
   decimal.truncated   = (uint8_t) state.truncated;
   decimal.special     = mulle_dtostr_normal_e;

done:
   if( endptr)
      *endptr = state.p;
   return( decimal);
}


/* --------------------------------------------------------------------------
 * Layer 2: decimal -> double
 *
 * A bisection over the bit pattern of positive doubles, using
 * `mulle_dtostr_decompose` as the comparison. The bit pattern of positive
 * doubles is monotone in value and shortest decimals are monotone in value,
 * so the search is well defined. When the input is the shortest
 * representation of a double, that double is found and the roundtrip is
 * guaranteed by construction.
 *
 * Otherwise the search brackets two adjacent doubles and the choice between
 * them is made by comparing the input against their exact midpoint. That
 * comparison is done in exact integer arithmetic, see `struct bigint` below,
 * as no amount of floating point would be trustworthy there.
 * --------------------------------------------------------------------------
 */

/* Number of digits kept by the parser is MULLE__STRTOD_MAX_DIGITS, so a
 * significand is at most 64 bits. In range decimal exponents run from about
 * -343 to 309, which calls for 5^343 (797 bits), and the alignment shift
 * between the decimal and the binary side reaches about 1385 bits. 128 limbs
 * of 32 bits leave ample room.
 */
#define BIGINT_LIMBS   128

struct bigint
{
   uint32_t   limb[ BIGINT_LIMBS];   /* little endian */
   int        used;
};


/* 5^13 is the largest power of five that fits into a uint32_t
 */
#define BIGINT_POW5_MAX   13

static uint32_t   bigint_pow5[ BIGINT_POW5_MAX + 1] =
{
   1u, 5u, 25u, 125u, 625u, 3125u, 15625u, 78125u, 390625u, 1953125u,
   9765625u, 48828125u, 244140625u, 1220703125u
};


static void   bigint_set_uint64( struct bigint *b, uint64_t value)
{
   memset( b->limb, 0, sizeof( b->limb));
   b->limb[ 0] = (uint32_t) value;
   b->limb[ 1] = (uint32_t) (value >> 32);
   b->used     = b->limb[ 1] ? 2 : (b->limb[ 0] ? 1 : 0);
}


static void   bigint_mul_uint32( struct bigint *b, uint32_t factor)
{
   uint64_t   carry;
   uint64_t   product;
   int        i;

   if( factor == 1 || ! b->used)
      return;

   carry = 0;
   for( i = 0; i < b->used; i++)
   {
      product   = (uint64_t) b->limb[ i] * factor + carry;
      b->limb[ i] = (uint32_t) product;
      carry     = product >> 32;
   }
   while( carry)
   {
      assert( b->used < BIGINT_LIMBS);
      b->limb[ b->used++] = (uint32_t) carry;
      carry >>= 32;
   }
}


static void   bigint_mul_pow5( struct bigint *b, int n)
{
   while( n >= BIGINT_POW5_MAX)
   {
      bigint_mul_uint32( b, bigint_pow5[ BIGINT_POW5_MAX]);
      n -= BIGINT_POW5_MAX;
   }
   if( n > 0)
      bigint_mul_uint32( b, bigint_pow5[ n]);
}


static void   bigint_shift_left( struct bigint *b, int bits)
{
   uint32_t   carry;
   uint32_t   value;
   int        i;
   int        limbs;
   int        shift;

   if( ! b->used || ! bits)
      return;

   limbs = bits / 32;
   shift = bits % 32;

   if( limbs)
   {
      assert( b->used + limbs <= BIGINT_LIMBS);
      for( i = b->used - 1; i >= 0; i--)
         b->limb[ i + limbs] = b->limb[ i];
      for( i = 0; i < limbs; i++)
         b->limb[ i] = 0;
      b->used += limbs;
   }

   if( shift)
   {
      carry = 0;
      for( i = limbs; i < b->used; i++)
      {
         value       = b->limb[ i];
         b->limb[ i] = (value << shift) | carry;
         carry       = value >> (32 - shift);
      }
      if( carry)
      {
         assert( b->used < BIGINT_LIMBS);
         b->limb[ b->used++] = carry;
      }
   }
}


static int   bigint_compare( struct bigint *a, struct bigint *b)
{
   int   i;

   if( a->used != b->used)
      return( a->used < b->used ? -1 : 1);

   for( i = a->used - 1; i >= 0; i--)
      if( a->limb[ i] != b->limb[ i])
         return( a->limb[ i] < b->limb[ i] ? -1 : 1);
   return( 0);
}


static double   bits_to_double( uint64_t bits)
{
   double   d;

   memcpy( &d, &bits, sizeof( d));
   return( d);
}


static uint64_t   double_to_bits( double d)
{
   uint64_t   bits;

   memcpy( &bits, &d, sizeof( bits));
   return( bits);
}


static int   is_infinite( double d)
{
   return( (double_to_bits( d) & ~DOUBLE_SIGN_BITS) == DOUBLE_INF_BITS);
}


/*
 * Compares `sig` * 10^`exp` against the midpoint between the double with bit
 * pattern `bits` and its immediate successor. Returns -1, 0 or 1.
 *
 * The midpoint of a = mantissa * 2^binary_exp and its successor is exactly
 * (2 * mantissa + 1) * 2^(binary_exp - 1), which needs 54 bits and is
 * therefore not a double itself. Both sides are turned into integers by
 * spreading 10^exp into 5^exp * 2^exp and cancelling the common power of two,
 * so the comparison is exact.
 *
 * `bits` may be 0, which yields the midpoint between zero and the smallest
 * subnormal. It may be DOUBLE_MAX_BITS, which yields the overflow threshold.
 */
static int   compare_to_midpoint( uint64_t sig, int exp, uint64_t bits)
{
   struct bigint   left;
   struct bigint   right;
   uint64_t        mantissa;
   int             binary_exp;
   int             biased;
   int             shift;

   biased   = (int) ((bits >> 52) & 0x7FF);
   mantissa = bits & 0xFFFFFFFFFFFFFull;

   if( ! biased)
      binary_exp = -1074;                  /* zero and the subnormals */
   else
   {
      mantissa  |= (uint64_t) 1 << 52;
      binary_exp = biased - 1075;
   }

   mantissa    = 2 * mantissa + 1;
   binary_exp -= 1;

   bigint_set_uint64( &left, sig);
   bigint_set_uint64( &right, mantissa);

   if( exp >= 0)
      bigint_mul_pow5( &left, exp);
   else
      bigint_mul_pow5( &right, -exp);

   shift = exp - binary_exp;
   if( shift > 0)
      bigint_shift_left( &left, shift);
   else
      if( shift < 0)
         bigint_shift_left( &right, -shift);

   return( bigint_compare( &left, &right));
}


static int   digit_count( uint64_t value)
{
   int   n;

   n = 0;
   while( value)
   {
      n++;
      value /= 10;
   }
   return( n);
}


/* Powers of ten up to 10^22 are the ones that a double holds exactly
 */
#define EXACT_POW10_MAX   22

static double   exact_pow10[ EXACT_POW10_MAX + 1] =
{
   1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11,
   1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22
};

/* Below 2^53 a uint64_t converts to a double without loss
 */
#define EXACT_SIGNIFICAND_MAX   ((uint64_t) 1 << 53)


/* Trailing zeroes carry no information, 12300e0 and 123e2 are the same
 * number. `decompose` reports its significand padded out to 16 or 17 digits,
 * so both sides have to be stripped before they can be compared.
 */
static void   strip_trailing_zeros( uint64_t *p_sig, int *p_exp)
{
   uint64_t   sig;

   sig = *p_sig;
   if( ! sig)
      return;

   while( ! (sig % 10))
   {
      sig /= 10;
      (*p_exp)++;
   }
   *p_sig = sig;
}


/* Compares the decimal of a double against the decimal `sig` * 10^`exp`,
 * both of which must be non zero. Returns -1, 0 or 1.
 *
 * No wide arithmetic needed here. The number of integer digits orders the two
 * unless they have the same magnitude, and then padding the shorter
 * significand to the length of the longer one puts them on a common scale.
 * The pad can not overflow, as `decompose` yields at most 17 digits and the
 * parser at most 19, so the widest pad is 17 digits scaled by 100.
 */
static int   decimal_compare( struct mulle_dtostr_decimal a,
                              uint64_t sig_b,
                              int exp_b)
{
   uint64_t   sig_a;
   int        digits_a;
   int        digits_b;
   int        exp_a;
   int        order_a;
   int        order_b;

   sig_a = a.significand;
   exp_a = (int) a.exponent;
   strip_trailing_zeros( &sig_a, &exp_a);

   digits_a = digit_count( sig_a);
   digits_b = digit_count( sig_b);

   order_a = digits_a + exp_a;
   order_b = digits_b + exp_b;
   if( order_a != order_b)
      return( order_a < order_b ? -1 : 1);

   while( digits_a < digits_b)
   {
      sig_a *= 10;
      digits_a++;
   }
   while( digits_b < digits_a)
   {
      sig_b *= 10;
      digits_b++;
   }

   if( sig_a != sig_b)
      return( sig_a < sig_b ? -1 : 1);
   return( 0);
}


double   mulle_strtod_compose( struct mulle_dtostr_decimal decimal)
{
   struct mulle_dtostr_decimal   probe;
   double                        result;
   uint64_t                      chosen;
   uint64_t                      hi;
   uint64_t                      lo;
   uint64_t                      mid;
   uint64_t                      sig;
   int                           cmp;
   int                           exp;

   switch( decimal.special)
   {
   case mulle_dtostr_inf_e :
      result = bits_to_double( DOUBLE_INF_BITS);
      return( decimal.sign ? -result : result);

   case mulle_dtostr_nan_e :
      return( bits_to_double( decimal.sign ? (DOUBLE_NAN_BITS | DOUBLE_SIGN_BITS)
                                           : DOUBLE_NAN_BITS));

   case mulle_dtostr_zero_e :
   case mulle_dtostr_invalid_e :
      result = 0.0;
      return( decimal.sign ? -result : result);
   }

   sig = decimal.significand;
   exp = (int) decimal.exponent;
   strip_trailing_zeros( &sig, &exp);

   if( ! sig)
   {
      result = 0.0;
      return( decimal.sign ? -result : result);
   }

   /* Both operands are exact and a single IEEE operation is correctly
    * rounded, so this is the right answer without any searching. It catches
    * most of what turns up in practice. Neither overflow nor a subnormal is
    * reachable here, the magnitudes involved are far too tame.
    */
   if( ! decimal.truncated &&
       sig < EXACT_SIGNIFICAND_MAX &&
       exp >= -EXACT_POW10_MAX &&
       exp <= EXACT_POW10_MAX)
   {
      result = (double) sig;
      if( exp > 0)
         result *= exact_pow10[ exp];
      else
         if( exp < 0)
            result /= exact_pow10[ -exp];
      return( decimal.sign ? -result : result);
   }

   /* Cheap magnitude filter, so that the exact comparisons below never see an
    * exponent that would blow up the bigint. Both bounds are well clear of
    * the double range, the real decisions happen further down.
    */
   cmp = digit_count( sig) + exp;
   if( cmp > 320)
   {
      result = bits_to_double( DOUBLE_INF_BITS);
      return( decimal.sign ? -result : result);
   }
   if( cmp < -350)
   {
      result = 0.0;
      return( decimal.sign ? -result : result);
   }

   /* Underflow. A value at or below the midpoint between zero and the
    * smallest subnormal rounds to zero, ties included, as zero is the
    * candidate with the even significand. Truncated digits push the real
    * value above an apparent tie.
    */
   cmp = compare_to_midpoint( sig, exp, 0);
   if( cmp < 0 || (! cmp && ! decimal.truncated))
   {
      result = 0.0;
      return( decimal.sign ? -result : result);
   }

   /* Overflow. Above the midpoint between DBL_MAX and the first value that no
    * longer fits, and at it too, because there the even candidate is the one
    * that overflows.
    */
   cmp = compare_to_midpoint( sig, exp, DOUBLE_MAX_BITS);
   if( cmp >= 0)
   {
      result = bits_to_double( DOUBLE_INF_BITS);
      return( decimal.sign ? -result : result);
   }

   /* Bisect for the double whose shortest decimal is the input. The value is
    * known to be in range by now.
    */
   lo = 1;
   hi = DOUBLE_MAX_BITS;

   while( lo < hi)
   {
      mid   = lo + (hi - lo) / 2;
      probe = mulle_dtostr_decompose( bits_to_double( mid));
      cmp   = decimal_compare( probe, sig, exp);

      if( cmp < 0)
         lo = mid + 1;
      else
         if( cmp > 0)
            hi = mid;
         else
         {
            /* The input is this double's shortest representation, so this is
             * the double that `mulle_dtostr` prints as the input. Roundtrip.
             */
            result = bits_to_double( mid);
            return( decimal.sign ? -result : result);
         }
   }

   /* The input is nobody's shortest representation. It sits between the
    * decimals of `lo - 1` and `lo`, so one of the two is the answer and the
    * midpoint decides.
    */
   cmp = compare_to_midpoint( sig, exp, lo - 1);
   if( cmp > 0)
      chosen = lo;
   else
      if( cmp < 0)
         chosen = lo - 1;
      else
         if( decimal.truncated)
            chosen = lo;                            /* really above the tie */
         else
            chosen = ((lo - 1) & 1) ? lo : lo - 1;  /* ties to even */

   result = bits_to_double( chosen);
   return( decimal.sign ? -result : result);
}


int   mulle_strtod_scan( char *s,
                        size_t len,
                        struct mulle__strtod_syntax *syntax,
                        double *p_value,
                        char **endptr)
{
   struct mulle_dtostr_decimal   decimal;
   double                        value;

   decimal = mulle_strtod_parse( s, len, syntax, endptr);

   if( decimal.special == mulle_dtostr_invalid_e)
   {
      if( p_value)
         *p_value = 0.0;
      return( mulle_strtod_no_conversion_e);
   }

   value = mulle_strtod_compose( decimal);
   if( p_value)
      *p_value = value;

   /* A spelled out "inf" or "nan" is not a range error, it is what was asked
    * for. Only a finite decimal that could not be represented is.
    */
   if( decimal.special != mulle_dtostr_normal_e)
      return( mulle_strtod_ok_e);

   if( is_infinite( value))
      return( mulle_strtod_overflow_e);
   if( value == 0.0)
      return( mulle_strtod_underflow_e);

   return( mulle_strtod_ok_e);
}

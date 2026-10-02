//
// _mulle_utf_rover_longlong_decimal() detects overflow with
//
//    old   = value;
//    value = value * 10 + (c - '0');
//    if( value >= old)
//       continue;                 // assumed to be fine
//    state = has_overflown;
//
// A wraparound of an unsigned 64 bit accumulator very often lands above the
// previous value, so this heuristic misses most overflows. The correct test is
//
//    if( value > (ULLONG_MAX - d) / 10)
//       overflow
//
#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <string.h>
#include <limits.h>


static int   fails;


static void   check( char *s, int expected_rval)
{
   char        *p;
   long long   value;
   int         rval;

   p     = s;
   value = 0;
   rval  = _mulle_utf8_scan_longlong_decimal( &p, strlen( s), &value);

   if( rval == expected_rval)
   {
      printf( "PASS: \"%s\" rval=%d\n", s, rval);
      return;
   }
   printf( "FAIL: \"%s\" expected rval=%d, got rval=%d (value=%lld)\n",
           s, expected_rval, rval, value);
   ++fails;
}


int   main( void)
{
   // well inside range
   check( "0",                    mulle_utf_is_valid);
   check( "1848",                 mulle_utf_is_valid);
   check( "-1848",                mulle_utf_is_valid);
   check( "+1848",                mulle_utf_is_valid);
   check( "9223372036854775807",  mulle_utf_is_valid);                   // LLONG_MAX
   check( "-9223372036854775808", mulle_utf_is_valid);                   // LLONG_MIN

   // above LLONG_MAX but still an unsigned long long
   check( "9223372036854775808",  mulle_utf_is_too_large_for_signed);
   check( "18446744073709551615", mulle_utf_is_too_large_for_signed);    // ULLONG_MAX

   // these must all be reported as overflow
   check( "18446744073709551616",          mulle_utf_has_overflown);     // ULLONG_MAX + 1
   check( "18446744073709551617",          mulle_utf_has_overflown);
   check( "99999999999999999999",          mulle_utf_has_overflown);     // 20 nines
   check( "184467440737095516150",         mulle_utf_has_overflown);     // ULLONG_MAX * 10
   check( "12345678901234567890123456789", mulle_utf_has_overflown);
   check( "-9223372036854775809",          mulle_utf_has_overflown);     // LLONG_MIN - 1
   check( "-18446744073709551616",         mulle_utf_has_overflown);

   // not numbers at all
   check( "",     mulle_utf_is_invalid);
   check( "x",    mulle_utf_is_invalid);
   check( "-",    mulle_utf_is_invalid);
   check( "+",    mulle_utf_is_invalid);

   // trailing garbage
   check( "1848x", mulle_utf_is_valid | mulle_utf_has_trailing_garbage);

   if( ! fails)
      printf( "\nAll tests passed!\n");
   else
      printf( "\n%d test(s) failed!\n", fails);

   return( fails);
}

//
// mulle_char5_is_char5string() answers "can these len characters be packed
// into a char5 value". It stops at a NUL and reports success:
//
//    case 0  : return( 1);   // zero byte, ok fine!
//
// mulle_char5_encode() then asserts on that same NUL (`assert( char5 > 0)`),
// and with NDEBUG silently produces a value that decodes to a shorter string,
// because both mulle_char5_strlen() and mulle_char5_decode() stop as soon as
// the remaining bits are zero.
//
// Either is_char5string() must reject strings whose given length contains a
// NUL, or encode/decode must preserve them. This test asserts the weaker and
// more useful contract: whatever is_char5string() accepts must survive a
// round trip.
//
#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <string.h>


static int   fails;


static void   check( char *description, char *src, size_t len)
{
   char            out[ 32];
   mulle_char5_t   value;
   size_t          n;

   if( ! mulle_char5_is_char5string( src, len))
   {
      printf( "PASS: %s (rejected by is_char5string)\n", description);
      return;
   }

   memset( out, '?', sizeof( out));
   value = mulle_char5_encode( src, len);
   n     = mulle_char5_decode( value, out, sizeof( out));

   if( n == len && ! memcmp( out, src, len))
   {
      printf( "PASS: %s (round trip preserved %zu chars)\n", description, len);
      return;
   }

   printf( "FAIL: %s accepted but round trip gave %zu of %zu chars\n",
           description, n, len);
   ++fails;
}


int   main( void)
{
   char   plain[]   = { 'a', 'b', 'c' };
   char   trailing[]= { 'a', 'b', 0 };
   char   leading[] = { 0, 'a', 'b' };
   char   middle[]  = { 'a', 0, 'b' };
   char   twozero[] = { 'a', 0, 0 };
   char   allzero[] = { 0, 0, 0 };
   char   maxlen[]  = { 'a', 'b', 'c', 'd', 'e', 'f' };

   check( "\"abc\"",            plain,    3);
   check( "\"ab\\0\"",          trailing, 3);
   check( "\"\\0ab\"",          leading,  3);
   check( "\"a\\0b\"",          middle,   3);
   check( "\"a\\0\\0\"",        twozero,  3);
   check( "\"\\0\\0\\0\"",      allzero,  3);
   check( "\"abcdef\"",         maxlen,   6);
   check( "empty",              plain,    0);

   if( ! fails)
      printf( "\nAll tests passed!\n");
   else
      printf( "\n%d test(s) failed!\n", fails);

   return( fails);
}

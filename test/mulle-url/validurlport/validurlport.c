#include <mulle-url/mulle-url.h>

#include <stdio.h>


static int   expect_port( int32_t c, int expected)
{
   int   actual;

   actual = mulle_unicode_is_validurlport( c);
   printf( "  is_validurlport('%c') = %d  (expected %d)  %s\n",
           (char) c, actual, expected, actual == expected ? "PASS" : "FAIL");
   return( actual != expected);
}


static int   expect_port16( uint16_t c, int expected)
{
   int   actual;

   actual = mulle_unicode16_is_validurlport( c);
   printf( "  is_validurlport16('%c') = %d  (expected %d)  %s\n",
           (char) c, actual, expected, actual == expected ? "PASS" : "FAIL");
   return( actual != expected);
}


static int   expect_plane( unsigned int plane, int expected)
{
   int   actual;

   actual = mulle_unicode_is_validurlportplane( plane);
   printf( "  is_validurlportplane(%u) = %d  (expected %d)  %s\n",
           plane, actual, expected, actual == expected ? "PASS" : "FAIL");
   return( actual != expected);
}


int   main( void)
{
   int   failed;

   printf( "RFC 3986 port = *DIGIT, validated per code point\n");
   printf( "the empty port and the 16-bit range limit stay the caller's\n\n");

   printf( "digits, all of them:\n");
   failed  = expect_port( '0', 1);
   failed |= expect_port( '1', 1);
   failed |= expect_port( '2', 1);
   failed |= expect_port( '3', 1);
   failed |= expect_port( '4', 1);
   failed |= expect_port( '5', 1);
   failed |= expect_port( '6', 1);
   failed |= expect_port( '7', 1);
   failed |= expect_port( '8', 1);
   failed |= expect_port( '9', 1);

   printf( "\nnot digits:\n");
   failed |= expect_port( ':', 0);
   failed |= expect_port( '/', 0);
   failed |= expect_port( 'a', 0);
   failed |= expect_port( ' ', 0);
   failed |= expect_port( '+', 0);

   printf( "\nnon-ASCII digits are not ASCII digits:\n");
   if( mulle_unicode_is_validurlport( 0x0660))
   {
      printf( "  U+0660 accepted  FAIL\n");
      failed = 1;
   }
   else
      printf( "  U+0660 rejected  PASS\n");

   printf( "\nthe UTF-16 form agrees:\n");
   failed |= expect_port16( '5', 1);
   failed |= expect_port16( 'x', 0);

   printf( "\nonly plane 0 (BMP) carries digits:\n");
   failed |= expect_plane( 0, 1);
   failed |= expect_plane( 1, 0);
   failed |= expect_plane( 16, 0);

   printf( "\n%s\n", failed ? "FAILED" : "PASSED");
   return( failed ? 1 : 0);
}
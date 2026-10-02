#include <mulle-url/mulle-url.h>

#include <stdio.h>


static int   expect_path( int32_t c, int expected)
{
   int   actual;

   actual = mulle_unicode_is_validurlpath( c);
   printf( "  is_validurlpath('%c') = %d  (expected %d)  %s\n",
           (char) c, actual, expected, actual == expected ? "PASS" : "FAIL");
   return( actual != expected);
}


int   main( void)
{
   int   failed;

   printf( "RFC 3986 pchar in the path class\n");
   printf( "pchar = unreserved / pct-encoded / sub-delims / \":\" / \"@\"\n\n");

   printf( "sub-delims, ';' is one of them:\n");
   failed  = expect_path( ';', 1);
   failed |= expect_path( '!', 1);
   failed |= expect_path( '$', 1);
   failed |= expect_path( '&', 1);
   failed |= expect_path( '\'', 1);
   failed |= expect_path( '(', 1);
   failed |= expect_path( ')', 1);
   failed |= expect_path( '*', 1);
   failed |= expect_path( '+', 1);
   failed |= expect_path( ',', 1);
   failed |= expect_path( '=', 1);

   printf( "\nremaining pchar:\n");
   failed |= expect_path( ':', 1);
   failed |= expect_path( '@', 1);
   failed |= expect_path( '/', 1);

   printf( "\ndelimiters that must stay out:\n");
   failed |= expect_path( '?', 0);
   failed |= expect_path( '#', 0);
   failed |= expect_path( '[', 0);
   failed |= expect_path( ']', 0);

   printf( "\n%s\n", failed ? "FAILED" : "PASSED");
   return( failed ? 1 : 0);
}

#include <mulle-url/mulle-url.h>

#include <stdio.h>
#include <string.h>


static int   expect_triplet( const char *s, int expected)
{
   int   actual;

   actual = mulle_unicode_is_validurlpercentescape_triplet( s, strlen( s));
   printf( "  triplet(\"%s\") = %d  (expected %d)  %s\n",
           s, actual, expected, actual == expected ? "PASS" : "FAIL");
   return( actual != expected);
}


static int   expect_encoding( const char *s, int expected)
{
   int   actual;

   actual = mulle_unicode_is_validurlpercentencoding( s, strlen( s));
   printf( "  encoding(\"%s\") = %d  (expected %d)  %s\n",
           s, actual, expected, actual == expected ? "PASS" : "FAIL");
   return( actual != expected);
}


int   main( void)
{
   int   failed;

   printf( "RFC 3986 pct-encoded = \"%%\" HEXDIG HEXDIG\n");
   printf( "the triplet check looks at the slice start only,\n");
   printf( "the encoding check looks at every '%%' in the slice\n\n");

   printf( "a '%%' at the slice start, followed by two hex digits:\n");
   failed  = expect_triplet( "%20", 1);
   failed |= expect_triplet( "%2F", 1);
   failed |= expect_triplet( "%2f", 1);
   failed |= expect_triplet( "%00", 1);
   failed |= expect_triplet( "%FF", 1);

   printf( "\nnot a well-formed triplet:\n");
   failed |= expect_triplet( "%2", 0);
   failed |= expect_triplet( "%zz", 0);
   failed |= expect_triplet( "%2G", 0);
   failed |= expect_triplet( "x20", 0);
   failed |= expect_triplet( "", 0);
   failed |= expect_triplet( "%", 0);

   printf( "\nwhole slice: every '%%' must open a well-formed triplet:\n");
   failed |= expect_encoding( "a%20b", 1);
   failed |= expect_encoding( "%00", 1);
   failed |= expect_encoding( "%20%41", 1);
   failed |= expect_encoding( "", 1);
   failed |= expect_encoding( "abc", 1);

   printf( "\nmalformed percent-encoding:\n");
   failed |= expect_encoding( "a%b", 0);
   failed |= expect_encoding( "%2", 0);
   failed |= expect_encoding( "%zz", 0);
   failed |= expect_encoding( "a%2", 0);
   failed |= expect_encoding( "%%20", 0);

   printf( "\n%s\n", failed ? "FAILED" : "PASSED");
   return( failed ? 1 : 0);
}

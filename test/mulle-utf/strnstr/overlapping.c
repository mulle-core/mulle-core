//
// mulle_utf8_strnstr() is a naive scan that never backtracks. On a partial
// match it resets the needle pointer but keeps the already advanced haystack
// pointer, so every overlapping occurrence is missed:
//
//    if( *s++ != *p)
//    {
//       p = search;      // needle restarts, but s has already moved on
//       continue;
//    }
//
// Searching "ab" in "aab" consumes the first 'a' against 'a', then compares
// the second 'a' against 'b', fails, and resumes at 'b' - the match at
// offset 1 is never seen.
//
// mulle_utf8_strstr() and mulle_utf8_strnchr() (for multi byte characters)
// are implemented on top of it and inherit the defect.
//
#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <string.h>


static int   fails;


static void   check( char *haystack, char *needle)
{
   char   *expected;
   char   *actual;

   expected = strstr( haystack, needle);
   actual   = mulle_utf8_strnstr( haystack, strlen( haystack), needle);

   if( expected == actual)
   {
      printf( "PASS: strnstr( \"%s\", \"%s\") -> %s\n",
              haystack,
              needle,
              expected ? "found" : "not found");
      return;
   }

   printf( "FAIL: strnstr( \"%s\", \"%s\") expected offset %ld, got %ld\n",
           haystack,
           needle,
           expected ? (long) (expected - haystack) : -1L,
           actual   ? (long) (actual - haystack)   : -1L);
   ++fails;
}


int   main( void)
{
   // these already work
   check( "hello world", "world");
   check( "hello world", "hello");
   check( "hello world", "xyz");
   check( "abc", "abc");

   // overlapping prefixes
   check( "aab", "ab");
   check( "aaab", "aab");
   check( "mississippi", "issip");
   check( "mississippi", "ssi");
   check( "aaa", "aa");
   check( "abcabcd", "abcd");
   check( "VfL VfL Bochum", "VfL Bochum");

   // multi byte needle, same code path
   check( "a\xC3\xA9\xC3\xA9", "\xC3\xA9\xC3\xA9");

   if( ! fails)
      printf( "\nAll tests passed!\n");
   else
      printf( "\n%d test(s) failed!\n", fails);

   return( fails);
}

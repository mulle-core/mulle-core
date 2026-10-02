//
// _mulle_utf8_convert_to_iso1() can never convert a two byte sequence:
//
//    if( (_c & 0xFC) == 0xC0)      // _c is now known to be 0xC0..0xC3
//    {
//       if( src < sentinel)
//       {
//          _d = *src++;
//          if( (_c & 0xC0) == 0x80)    // <- must test _d, not _c
//
// After the outer test `_c & 0xC0` is always 0xC0, so the inner condition is
// dead code. Every non ASCII character falls through to the `unknown`
// handling: with unknown < 0 the function returns NULL, with unknown == 0 the
// character is dropped, otherwise it is replaced.
//
#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <string.h>


static int   fails;


static void   check( char *description,
                     char *utf8,
                     size_t len,
                     int unknown,
                     char *expected,
                     size_t expected_len)
{
   char     dst[ 32];
   char     *end;
   size_t   actual_len;
   size_t   i;

   memset( dst, 0, sizeof( dst));

   end = _mulle_utf8_convert_to_iso1( utf8, len, dst, unknown);
   if( ! end)
   {
      if( ! expected)
      {
         printf( "PASS: %s\n", description);
         return;
      }
      printf( "FAIL: %s (returned NULL)\n", description);
      ++fails;
      return;
   }

   if( ! expected)
   {
      printf( "FAIL: %s (expected NULL, got a result)\n", description);
      ++fails;
      return;
   }

   actual_len = (size_t) (end - dst);
   if( actual_len != expected_len || memcmp( dst, expected, expected_len))
   {
      printf( "FAIL: %s\n      expected", description);
      for( i = 0; i < expected_len; i++)
         printf( " %02X", (unsigned char) expected[ i]);
      printf( "\n      got     ");
      for( i = 0; i < actual_len; i++)
         printf( " %02X", (unsigned char) dst[ i]);
      printf( "\n");
      ++fails;
      return;
   }

   printf( "PASS: %s\n", description);
}


int   main( void)
{
   // ASCII passes through, that part works
   check( "ascii", "hi", 2, -1, "hi", 2);

   // U+00E9 LATIN SMALL LETTER E WITH ACUTE -> iso1 0xE9
   check( "U+00E9", "\xC3\xA9", 2, -1, "\xE9", 1);

   // U+00FF -> iso1 0xFF
   check( "U+00FF", "\xC3\xBF", 2, -1, "\xFF", 1);

   // U+0080 -> iso1 0x80, the lowest two byte sequence
   check( "U+0080", "\xC2\x80", 2, -1, "\x80", 1);

   // mixed
   check( "mixed", "a\xC3\xA9" "b", 4, -1, "a\xE9" "b", 3);

   // U+20AC EURO is not representable in iso1
   check( "U+20AC with unknown=-1 fails", "\xE2\x82\xAC", 3, -1, NULL, 0);
   check( "U+20AC with unknown=0 skips",  "a\xE2\x82\xAC" "b", 5, 0, "ab", 2);
   check( "U+20AC with unknown='?'",      "a\xE2\x82\xAC" "b", 5, '?', "a?b", 3);

   if( ! fails)
      printf( "\nAll tests passed!\n");
   else
      printf( "\n%d test(s) failed!\n", fails);

   return( fails);
}

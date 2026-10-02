//
// Round-trip property tests across all six conversion pairs at encoding
// boundaries: U+7F, U+80, U+7FF, U+800, U+D7FF, U+E000, U+FFFD, U+FFFF,
// U+10000, U+10FFFF, plus a mixed string exercising all byte widths.
//
#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static int   fails;


static void   test_utf32_to_utf8_to_utf32( char *label,
                                           mulle_utf32_t *input,
                                           size_t len)
{
   char            utf8buf[ 64];
   mulle_utf32_t   back[ 16];
   char            *utf8end;
   mulle_utf32_t   *backend;

   utf8end = _mulle_utf32_convert_to_utf8( input, len, utf8buf);
   backend = _mulle_utf8_convert_to_utf32( utf8buf, (size_t)(utf8end - utf8buf), back);

   if( (size_t)(backend - back) == len && ! memcmp( back, input, len * sizeof( mulle_utf32_t)))
   {
      printf( "PASS: %s (utf32->utf8->utf32)\n", label);
      return;
   }
   printf( "FAIL: %s (utf32->utf8->utf32, got %zu chars back, expected %zu)\n",
           label, (size_t)(backend - back), len);
   ++fails;
}


static void   test_utf32_to_utf16_to_utf32( char *label,
                                            mulle_utf32_t *input,
                                            size_t len)
{
   mulle_utf16_t   utf16buf[ 32];
   mulle_utf32_t   back[ 16];
   mulle_utf16_t   *utf16end;
   mulle_utf32_t   *backend;

   utf16end = _mulle_utf32_convert_to_utf16( input, len, utf16buf);
   backend  = _mulle_utf16_convert_to_utf32( utf16buf,
                                             (size_t)(utf16end - utf16buf),
                                             back);

   if( (size_t)(backend - back) == len && ! memcmp( back, input, len * sizeof( mulle_utf32_t)))
   {
      printf( "PASS: %s (utf32->utf16->utf32)\n", label);
      return;
   }
   printf( "FAIL: %s (utf32->utf16->utf32, got %zu chars back, expected %zu)\n",
           label, (size_t)(backend - back), len);
   ++fails;
}


static void   test_utf8_to_utf16_to_utf8( char *label,
                                          char *input,
                                          size_t len)
{
   mulle_utf16_t   utf16buf[ 32];
   char            back[ 64];
   mulle_utf16_t   *utf16end;
   char            *backend;

   utf16end = _mulle_utf8_convert_to_utf16( input, len, utf16buf);
   backend  = _mulle_utf16_convert_to_utf8( utf16buf,
                                            (size_t)(utf16end - utf16buf),
                                            back);

   if( (size_t)(backend - back) == len && ! memcmp( back, input, len))
   {
      printf( "PASS: %s (utf8->utf16->utf8)\n", label);
      return;
   }
   printf( "FAIL: %s (utf8->utf16->utf8, got %zu bytes back, expected %zu)\n",
           label, (size_t)(backend - back), len);
   ++fails;
}


static void   test_codepoint( char *label, mulle_utf32_t c)
{
   mulle_utf32_t   arr[ 1];

   arr[ 0] = c;
   test_utf32_to_utf8_to_utf32( label, arr, 1);
   test_utf32_to_utf16_to_utf32( label, arr, 1);
}


int   main( void)
{
   // Single boundary code points
   test_codepoint( "U+007F (max 1-byte UTF-8)", 0x7F);
   test_codepoint( "U+0080 (min 2-byte UTF-8)", 0x80);
   test_codepoint( "U+07FF (max 2-byte UTF-8)", 0x7FF);
   test_codepoint( "U+0800 (min 3-byte UTF-8)", 0x800);
   test_codepoint( "U+D7FF (last before surrogates)", 0xD7FF);
   test_codepoint( "U+E000 (first after surrogates)", 0xE000);
   test_codepoint( "U+FFFD (replacement character)", 0xFFFD);
   test_codepoint( "U+10000 (min 4-byte UTF-8, min surrogate pair)", 0x10000);
   test_codepoint( "U+10FFFF (max Unicode)", 0x10FFFF);

   // Mixed string exercising all byte widths:
   // 'A' (1 byte), U+00E9 (2 bytes), U+3042 (3 bytes), U+1F600 (4 bytes)
   {
      mulle_utf32_t mixed[] = { 0x41, 0xE9, 0x3042, 0x1F600 };

      test_utf32_to_utf8_to_utf32( "mixed 1/2/3/4-byte", mixed, 4);
      test_utf32_to_utf16_to_utf32( "mixed 1/2/3/4-byte", mixed, 4);
   }

   // UTF-8 round trip through UTF-16
   {
      // "Aé日😀" as UTF-8
      char mixed_utf8[] = { 0x41,
                            (char) 0xC3, (char) 0xA9,
                            (char) 0xE6, (char) 0x97, (char) 0xA5,
                            (char) 0xF0, (char) 0x9F, (char) 0x98, (char) 0x80 };

      test_utf8_to_utf16_to_utf8( "mixed utf8->utf16->utf8",
                                  mixed_utf8, sizeof( mixed_utf8));
   }

   if( ! fails)
      printf( "\nAll tests passed!\n");
   else
      printf( "\n%d test(s) failed!\n", fails);

   return( !!fails);
}

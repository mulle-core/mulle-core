//
// mulle_utf8data_range_of_utf32_range() is supposed to map a character range
// (in UTF-32 indices) onto the corresponding byte range in a UTF-8 buffer.
//
// The bug: it uses `(unsigned char) *s++ & 0x80` to skip "continuation bytes",
// but that skips ALL bytes with the high bit set, including lead bytes.
// Multi-byte characters contribute zero to the character count, so the range
// is effectively in ASCII-character positions, ignoring non-ASCII entirely.
//
#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <string.h>


static int   fails;


static void   expect_range( char *description,
                            char *utf8,
                            size_t utf8len,
                            struct mulle_range char_range,
                            char *expected,
                            size_t expected_len)
{
   struct mulle_utf8data   data;
   struct mulle_utf8data   result;

   data.characters = utf8;
   data.length     = utf8len;

   result = mulle_utf8data_range_of_utf32_range( data, char_range);

   if( result.characters
       && result.length == expected_len
       && ! memcmp( result.characters, expected, expected_len))
   {
      printf( "PASS: %s\n", description);
      return;
   }

   if( ! result.characters)
      printf( "FAIL: %s (returned NULL)\n", description);
   else
      printf( "FAIL: %s (got %zu bytes \"%.*s\", expected %zu bytes \"%.*s\")\n",
              description,
              result.length, (int) result.length, result.characters,
              expected_len, (int) expected_len, expected);
   ++fails;
}


int   main( void)
{
   // "aéb"  = 61 C3 A9 62  (4 bytes, 3 characters)
   char   aeb[] = { 0x61, 0xC3, 0xA9, 0x62 };

   // "café" = 63 61 66 C3 A9 (5 bytes, 4 characters)
   char   cafe[] = { 0x63, 0x61, 0x66, 0xC3, 0xA9 };

   // "日本語" = E6 97 A5 E6 9C AC E8 AA 9E (9 bytes, 3 characters)
   char   jpn[] = { 0xE6, 0x97, 0xA5,
                    0xE6, 0x9C, 0xAC,
                    0xE8, 0xAA, 0x9E };

   // "a😀b" = 61 F0 9F 98 80 62 (6 bytes, 3 characters)
   char   emoji[] = { 0x61, 0xF0, 0x9F, 0x98, 0x80, 0x62 };

   // pure ASCII control: "abcde" range(1,2) = "bc"
   expect_range( "ASCII range(1,2) of \"abcde\"",
                 "abcde", 5,
                 mulle_range_make( 1, 2),
                 "bc", 2);

   // "aéb": character 1 is 'é' (2 bytes)
   expect_range( "range(1,1) of \"aéb\" is the é",
                 aeb, 4,
                 mulle_range_make( 1, 1),
                 &aeb[1], 2);

   // "aéb": characters 0-1 are "aé" (3 bytes)
   expect_range( "range(0,2) of \"aéb\" is \"aé\"",
                 aeb, 4,
                 mulle_range_make( 0, 2),
                 &aeb[0], 3);

   // "aéb": character 2 is 'b'
   expect_range( "range(2,1) of \"aéb\" is \"b\"",
                 aeb, 4,
                 mulle_range_make( 2, 1),
                 &aeb[3], 1);

   // "café": characters 2-3 are "fé" (3 bytes)
   expect_range( "range(2,2) of \"café\" is \"fé\"",
                 cafe, 5,
                 mulle_range_make( 2, 2),
                 &cafe[2], 3);

   // "日本語": character 1 is '本' (3 bytes)
   expect_range( "range(1,1) of \"日本語\" is \"本\"",
                 jpn, 9,
                 mulle_range_make( 1, 1),
                 &jpn[3], 3);

   // "日本語": all 3 characters
   expect_range( "range(0,3) of \"日本語\" is all 9 bytes",
                 jpn, 9,
                 mulle_range_make( 0, 3),
                 jpn, 9);

   // "a😀b": character 1 is 😀 (4 bytes)
   expect_range( "range(1,1) of \"a😀b\" is the emoji",
                 emoji, 6,
                 mulle_range_make( 1, 1),
                 &emoji[1], 4);

   // "a😀b": character 2 is 'b'
   expect_range( "range(2,1) of \"a😀b\" is \"b\"",
                 emoji, 6,
                 mulle_range_make( 2, 1),
                 &emoji[5], 1);

   if( ! fails)
      printf( "\nAll tests passed!\n");
   else
      printf( "\n%d test(s) failed!\n", fails);

   return( !!fails);
}

//
// mulle_utf16_validate() reads one UTF16 unit past the end of the buffer when
// the last unit is a high surrogate:
//
//    for( ; src < sentinel; src++)
//    {
//       ...
//       if( src >= sentinel)     // always false, the loop already guarantees this
//          return( src);
//       d = *++src;              // reads sentinel[ 0]
//
// The check needs to be `src + 1 >= sentinel`.
//
// Two observable effects:
//   1. a 2 byte out of bounds read (visible under AddressSanitizer)
//   2. if the memory past the buffer happens to hold a low surrogate, the
//      truncated string is reported as VALID, and every conversion routine
//      afterwards trusts that verdict and reads out of bounds too.
//
// This test provokes (2) deterministically by placing a low surrogate right
// behind the string that is handed to mulle_utf16_validate().
//
#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


static int   fails;


static void   expect_invalid( char *description, mulle_utf16_t *s, size_t len)
{
   if( mulle_utf16_validate( s, len))
   {
      printf( "PASS: %s\n", description);
      return;
   }
   printf( "FAIL: %s (accepted as valid UTF16)\n", description);
   ++fails;
}


static void   expect_valid( char *description, mulle_utf16_t *s, size_t len)
{
   if( ! mulle_utf16_validate( s, len))
   {
      printf( "PASS: %s\n", description);
      return;
   }
   printf( "FAIL: %s (rejected as invalid UTF16)\n", description);
   ++fails;
}


int   main( void)
{
   //
   // "poison" holds a valid surrogate pair. We only ever pass the first two
   // units to validate(), so the trailing DE00 is out of bounds as far as the
   // call is concerned.
   //
   mulle_utf16_t   poison[] = { 'A', 0xD83D, 0xDE00 };
   mulle_utf16_t   lone[]   = { 'A', 0xD83D };
   mulle_utf16_t   pair[]   = { 'A', 0xD83D, 0xDE00, 'B' };
   mulle_utf16_t   orphan[] = { 'A', 0xDE00 };
   mulle_utf16_t   nopair[] = { 'A', 0xD83D, 'B' };

   expect_invalid( "high surrogate at end of buffer (adjacent low surrogate)",
                   poison, 2);
   expect_invalid( "high surrogate at end of buffer", lone, 2);
   expect_invalid( "lone low surrogate", orphan, 2);
   expect_invalid( "high surrogate not followed by low surrogate", nopair, 3);
   expect_valid(   "complete surrogate pair", pair, 4);

   if( ! fails)
      printf( "\nAll tests passed!\n");
   else
      printf( "\n%d test(s) failed!\n", fails);

   return( fails);
}

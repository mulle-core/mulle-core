#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>


static void   test_conversion( mulle_utf32_t text[ 5])
{
   struct mulle_utf_information   info32;
   struct mulle_utf_information   info16;
   struct mulle_utf_information   info8;
   char                   *utf8;
   mulle_utf16_t                  *utf16;

   // UTF32 <-> UTF8
   mulle_utf32_information( text, -1, &info32);
   utf8 = mulle_utf32_convert_to_utf8_string( info32.start, info32.utf32len, NULL);
   if( ! utf8)
   {
      printf( "32 to 8 conversion failed");
      return;
   }

   mulle_utf8_information( utf8, -1, &info8);
   if( ! memcmp( &info8, &info32, sizeof( struct mulle_utf_information)))
   {
      printf( "mulle-utf8_information failed");
      return;
   }

   utf16 = mulle_utf8_convert_to_utf16_string( info8.start, info8.utf8len, NULL);
   if( ! utf16)
   {
      printf( "8 to 16 conversion failed");
      return;
   }

   mulle_utf16_information( utf16, -1, &info16);
   if( ! memcmp( &info16, &info32, sizeof( struct mulle_utf_information)))
   {
      printf( "mulle-utf16_information failed");
      return;
   }

   mulle_free( utf8);
   mulle_free( utf16);
}




static void  test( mulle_utf32_t text[ 4])
{
   test_conversion( text);
}


// 3emos: UTF8   f0 9f 8c 91 f0 9f 9a 9a f0 9f 91 a0  (unbommed)
// 3emos: UTF16  ff fe 3c d8 11 df 3d d8  9a de 3d d8 60 dc (bommed)
// 3emos: UTF32  ff fe 00 00 11 f3 01 00  9a f6 01 00 60 f4 01 00

int  main()
{
   mulle_utf32_t   bom_3emos[ 5]  = { 0xfeff, 0x01f311, 0x01f69a, 0x1f460, 0 };
   mulle_utf32_t   bom_utf16[ 5]  = { 65279, 47177, 29938, 18497, 0 };
   mulle_utf32_t   bom_utf15[ 5]  = { 65279, 32313, 29938, 18497, 0 };
   mulle_utf32_t   ascii_trailing_zero[ 5]  = { 'V', 'f', 'L', 0, 0 };

   test( bom_3emos);
   test( bom_utf15);
   test( bom_utf16);
   test( ascii_trailing_zero);

   return( 0);
}


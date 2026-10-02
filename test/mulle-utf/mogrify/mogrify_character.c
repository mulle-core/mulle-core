#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static mulle_utf32_t   xtoupper( mulle_utf32_t c)
{
   return( toupper( c));
}

static struct mulle_utf_mogrification_info  mogrify =
{
   xtoupper
};



static void   test_8( mulle_utf16_t original[ 4])
{
   char                    *text;
   char                    buf[ 32] = { 0 };  // important
   struct mulle_utf8data   src;
   struct mulle_utf8data   dst;

   text = mulle_utf16_convert_to_utf8_string( original, 4, NULL);
   src  = mulle_utf8data_make( text, (size_t) -1);
   dst  = mulle_utf8data_make( buf, sizeof( buf));

   _mulle_utf8_character_mogrify( &dst, &src, &mogrify);

   printf( "%s -> %s\n", text, dst.characters);
   mulle_free( text);
}


static void   test_16( mulle_utf16_t original[ 4])
{
   char                     *s;
   char                     *t;
   mulle_utf32_t            buf[ 32] = { 0 };  // important
   struct mulle_utf16data   src;
   struct mulle_utf32data   dst;

   src  = mulle_utf16data_make( original, (size_t) -1);
   dst  = mulle_utf32data_make( buf, sizeof( buf));

   _mulle_utf16_character_mogrify( &dst, &src, &mogrify);

   s = mulle_utf16_convert_to_utf8_string( original, 4, NULL);
   t = mulle_utf32_convert_to_utf8_string( dst.characters, dst.length, NULL);
   printf( "%s -> %s\n", s, t);
   mulle_free( t);
   mulle_free( s);
}

static void   test_16_unsafe( mulle_utf16_t original[ 4])
{
   char                     *s;
   char                     *t;
   mulle_utf16_t            buf[ 32] = { 0 };  // important
   struct mulle_utf16data   src;
   struct mulle_utf16data   dst;

   src  = mulle_utf16data_make( original, (size_t) -1);
   dst  = mulle_utf16data_make( buf, sizeof( buf));

   _mulle_utf16_character_mogrify_unsafe( &dst, &src, &mogrify);

   s = mulle_utf16_convert_to_utf8_string( original, 4, NULL);
   t = mulle_utf16_convert_to_utf8_string( dst.characters, dst.length, NULL);
   printf( "%s -> %s\n", s, t);
   mulle_free( t);
   mulle_free( s);
}

static void   test_32( mulle_utf16_t original[ 4])
{
   mulle_utf32_t            *text;
   char                     *s;
   char                     *t;
   mulle_utf32_t            buf[ 32] = { 0 };  // important
   struct mulle_utf32data   src;
   struct mulle_utf32data   dst;

   text = mulle_utf16_convert_to_utf32_string( original, 4, NULL);

   src  = mulle_utf32data_make( text, (size_t) -1);
   dst  = mulle_utf32data_make( buf, sizeof( buf));

   _mulle_utf32_character_mogrify( &dst, &src, &mogrify);

   s = mulle_utf16_convert_to_utf8_string( original, 4, NULL);
   t = mulle_utf32_convert_to_utf8_string( dst.characters, dst.length, NULL);
   printf( "%s -> %s\n", s, t);
   mulle_free( t);
   mulle_free( s);

   mulle_free( text);
}

int  main()
{
   mulle_utf16_t   text_utf16[ 4]  = { 47177, 29938, 18497 };
   mulle_utf16_t   text_utf15[ 4]  = { 32313, 29938, 18497 };
   mulle_utf16_t   ascii_capitalized[ 4]  = { 'V', 'f', 'L', 0 };
   mulle_utf16_t   ascii_lowercase[ 4]  = { 'v', 'f', 'l', 0 };
   mulle_utf16_t   ascii_uppercase[ 4]  = { 'V', 'F', 'L', 0 };

   test_8( text_utf15);
   test_8( text_utf16);
   test_8( ascii_capitalized);
   test_8( ascii_lowercase);
   test_8( ascii_uppercase);

   test_16( text_utf15);
   test_16( text_utf16);
   test_16( ascii_capitalized);
   test_16( ascii_lowercase);
   test_16( ascii_uppercase);

   test_16_unsafe( text_utf15);
   test_16_unsafe( text_utf16);
   test_16_unsafe( ascii_capitalized);
   test_16_unsafe( ascii_lowercase);
   test_16_unsafe( ascii_uppercase);

   test_32( text_utf15);
   test_32( text_utf16);
   test_32( ascii_capitalized);
   test_32( ascii_lowercase);
   test_32( ascii_uppercase);

   return( 0);
}


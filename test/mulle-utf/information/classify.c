#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>


int   mulle_utf_information_is_equal( struct mulle_utf_information *a,
                                      struct mulle_utf_information *b)
{
   if( a->utf8len != b->utf8len)
      return( 0);
   if( a->utf16len != b->utf16len)
      return( 0);
   if( a->utf32len != b->utf32len)
      return( 0);
   if( a->is_ascii != b->is_ascii)
      return( 0);
   if( a->is_char5 != b->is_char5)
      return( 0);
   if( a->is_utf15 != b->is_utf15)
      return( 0);
   return( 1);
}


struct buffer
{
   union
   {
      mulle_utf32_t   _32[ 8];
      mulle_utf16_t   _16[ 16];
      char    _8[ 32];
   } text;
   size_t         n;
};


static void  buffer_add( struct buffer *p, void *bytes, size_t len)
{
   memcpy( &p->text._8[ p->n], bytes, len);
   p->n += len;
}



static void   test( mulle_utf32_t text[ 4], int is_ascii, int is_utf15, int is_char5)
{
   struct buffer                  buffer16;
   struct buffer                  buffer8;
   struct mulle_utf_information   info8;
   struct mulle_utf_information   info16;
   struct mulle_utf_information   info32;

   memset( &buffer16, 0, sizeof( buffer16));
   memset( &buffer8, 0, sizeof( buffer8));

   mulle_utf32_bufferconvert_to_utf8( text, 4, &buffer8, (void *) buffer_add);
   mulle_utf32_bufferconvert_to_utf16( text, 4, &buffer16, (void *) buffer_add);

   mulle_utf8_information( buffer8.text._8, buffer8.n, &info8);
   mulle_utf16_information( buffer16.text._16, buffer16.n, &info16);
   mulle_utf32_information( text, 4, &info32);

   // buffer.n is bytes!
   if( ! mulle_utf_information_is_equal( &info32, &info8))
   {
      printf( "failed with %S\n", (wchar_t *) text);
      return;
   }

   if( ! mulle_utf_information_is_equal( &info16, &info8))
   {
      printf( "failed with %S (16/8)\n", (wchar_t *) text);
      return;
   }

   if( is_ascii >= 0 && info8.is_ascii != is_ascii)
   {
      printf( "ASCII detection failed with %S\n", (wchar_t *) text);
      return;
   }

   if( is_utf15 >= 0 && info8.is_utf15 != is_utf15)
   {
      printf( "UTF15 detection failed with %S\n", (wchar_t *) text);
      return;
   }

   if( is_char5 >= 0 && info8.is_char5 != is_char5)
   {
      printf( "CHAR5 detection failed with %S\n", (wchar_t *) text);
      return;
   }

   printf( "passed\n");
}


int  main()
{
   test( (mulle_utf32_t [ 4]) { 's', 'e', 't', '_' }, 1, 1, 1);
   test( (mulle_utf32_t [ 4]) { 'V', 'f', 'L', 0 }, 1, 1, 0);
   test( (mulle_utf32_t [ 4]) { 29938, 0 }, 0, 1, 0);
   test( (mulle_utf32_t [ 4]) { 49938, 0 }, 0, 0, 0);
      // KILLER
   test( (mulle_utf32_t [ 4]) { 65279, 47177, 29938, 18497 }, 0, 0, 0);
   return( 0);
}


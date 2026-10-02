#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static mulle_utf32_t   random_char( mulle_utf32_t mask)
{
   mulle_utf32_t  c;

   do
   {
      c = rand() % mulle_utf32_max + 1;
      c &= mask;
      if( ! c)
         continue;
   }
   while( mulle_utf32_is_bomcharacter( c) || mulle_utf32_is_surrogatecharacter( c) || mulle_utf32_is_noncharacter( c) || mulle_utf32_is_privatecharacter( c));

   return( c);
}



static void   random_text( mulle_utf32_t *text, size_t n, mulle_utf32_t mask)
{
   mulle_utf32_t   *sentinel;

   sentinel = &text[ n];
   while( text < sentinel)
      *text++ = random_char( mask);
}


struct buffer
{
   union
   {
      mulle_utf32_t   _32[ 8];
      mulle_utf16_t   _16[ 16];
      char    _8[ 32];
   } text;
   size_t   n;
};


static void  buffer_add( struct buffer *p, void *bytes, size_t len)
{
   memmove( &p->text._8[ p->n], bytes, len);
   p->n += len;
}



static void   test( mulle_utf32_t text[ 4])
{
   struct buffer   buffer16;
   struct buffer   buffer8;
   mulle_utf32_t   *utf32;
   mulle_utf16_t   *utf16;
   char            *utf8;
   mulle_utf32_t   c;
   unsigned int    i;

   memset( &buffer16, 0, sizeof( buffer16));
   memset( &buffer8, 0, sizeof( buffer8));

   mulle_utf32_bufferconvert_to_utf8( text, 4, &buffer8, (void *) buffer_add);
   mulle_utf32_bufferconvert_to_utf16( text, 4, &buffer16, (void *) buffer_add);

   utf8  = buffer8.text._8;
   utf16 = buffer16.text._16;
   utf32 = text;

   for( i = 0; i < 4; i++)
   {
      c = _mulle_utf32_next_utf32character( &utf32);
      if( text[ i] != c)
      {
         printf( "32f: %u failed (%d)\n", i, (int) c);
         abort();
      }

      c = _mulle_utf16_next_utf32character( &utf16);
      if( text[ i] != c)
      {
         printf( "16f: %u failed (%d)\n", i, (int) c);
         abort();
      }

      c  = _mulle_utf8_next_utf32character( &utf8);
      if( text[ i] != c)
      {
         printf( "8f: %u failed (%d)\n", i, (int) c);
         abort();
      }
   }

   for( i = 4; i;)
   {
      --i;
      c = _mulle_utf32_previous_utf32character( &utf32);
      if( text[ i] != c)
      {
         printf( "32b: %u failed (%d)\n", i, (int) c);
         abort();
      }

      c = _mulle_utf16_previous_utf32character( &utf16);
      if( text[ i] != c)
      {
         printf( "16b: %u failed (%d)\n", i, (int) c);
         abort();
      }

      c  = _mulle_utf8_previous_utf32character( &utf8);
      if( text[ i] != c)
      {
         printf( "8b: %u failed (%d)\n", i, (int) c);
         abort();
      }
   }
}


static void   stress_test()
{
   mulle_utf32_t   text[ 4];
   unsigned int    i;
   mulle_utf32_t   mask;

   mask = 0;
   for( i = 0; i < 1000000; i++)
   {
      switch( rand() % 5)
      {
      case 0 : mask = 0x7F; break;
      case 1 : mask = 0xFF; break;
      case 2 : mask = 0x7FFF; break;
      case 3 : mask = 0xFFFF; break;
      case 4 : mask = 0x1FFFFF; break;
      }

      random_text( text, 4, mask);

      test( text);
   }
}



int  main()
{
//   mulle_utf32_t   killer[ 4] = { 65279, 47177, 29938, 18497 };
   mulle_utf32_t   killer[ 4] = { 63, 78, 37, 27 };

   test( killer);
   stress_test();
   return( 0);
}


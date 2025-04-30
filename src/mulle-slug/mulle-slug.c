#include "include-private.h"

#include "mulle-slug.h"

#include <ctype.h>


int   __MULLE_SLUG_ranlib__;


uint32_t   mulle_slug_get_version( void)
{
   return( MULLE__SLUG_VERSION);
}


static struct map_entry
{
   mulle_utf32_t   utf32;
   char            *ascii;
} map[] =
{
#include "map.inc"
};



void  mulle_buffer_add_slugified_utf8data( struct mulle_buffer *buffer,
                                           struct mulle_utf8data data)
{
   mulle_utf32_t      c;
   mulle_utf32_t      prev;
   char               *walk;
   char               *sentinel;
   struct map_entry   *p;
   int                first;
   int                last;
   int                middle;
   size_t             length;
   size_t             prevlen;

   if( ! data.length)
      return;

   prev     = 0;
   walk     = data.characters;
   sentinel = &data.characters[ data.length];
   prevlen  = mulle_buffer_get_length( buffer);

   while( walk < sentinel)
   {
      c = mulle_utf8_next_utf32character( &walk);
      if( c < 127)
      {
         switch( c)
         {
         case '\0' : goto stop;
         case ' '  :
         case '\f' :
         case '\n' :
         case '\r' :
         case '\t' :
         case '\v' :
         case '-'  : if( prev && prev != '-')
                     {
                        mulle_buffer_add_byte( buffer, '-');
                        prev = '-';
                     }
                     continue;

//            case '&'  : mulle_buffer_add_string( buffer, "and"); prev = c; continue;
         case '<'  : mulle_buffer_add_string( buffer, "less"); prev = c; continue;
         case '>'  : mulle_buffer_add_string( buffer, "greater"); prev = c; continue;
//            case '|'  : mulle_buffer_add_string( buffer, "or"); prev = c; continue;
         case '$'  : mulle_buffer_add_string( buffer, "dollar"); prev = c; continue;
         case '#'  : mulle_buffer_add_string( buffer, "hash"); prev = c; continue;
         }

         if( ! isprint( c))
            continue;

         if( ispunct( c))
         {
            if( prev && prev != '-')
            {
               mulle_buffer_add_byte( buffer, '-');
               prev = '-';
            }
            continue;
         }

         mulle_buffer_add_byte( buffer, c);
         prev = c;
         continue;
      }

      // binary search UTF32 code
      {
         first  = 0;
         last   = (int) ((sizeof( map) / sizeof( map[ 0])) - 1);
         middle = (first + last) / 2;

         while( first <= last)
         {
            p = &map[ middle];
            if( p->utf32 <= c)
            {
               if( p->utf32 == c)
               {
                  mulle_buffer_add_string( buffer, p->ascii);
                  prev = c;
                  break;
               }

               first = middle + 1;
            }
            else
               last = middle - 1;

            middle = (first + last) / 2;
         }
      }
   }

stop:
   length = mulle_buffer_get_length( buffer);
   while( length > prevlen + 1)
   {
      c = mulle_buffer_get_last_byte( buffer);
      if( c != '-')
         break;

      mulle_buffer_remove_last_byte( buffer);
      --length;
   }
}


void  mulle_buffer_slugify_utf8data( struct mulle_buffer *buffer,
                                     struct mulle_utf8data data)
{
   mulle_buffer_add_slugified_utf8data( buffer, data);
   mulle_buffer_make_string( buffer);
}



struct mulle_utf8data   mulle_utf8data_slugify( struct mulle_utf8data  data,
                                                struct mulle_allocator *allocator)
{
   struct mulle_utf8data   slug;

   //
   // tries to avoid output of -- and trailing or leading -
   // tries to avoid output of leading '#'
   //
   mulle_buffer_do_allocator( buffer, allocator)
   {
      mulle_buffer_slugify_utf8data( buffer, data);
      slug = mulle_data_as_utf8data( mulle_buffer_extract_data( buffer));
   }

   return( slug);
}


char   *mulle_utf8_slugify( char *s)
{
   struct mulle_utf8data   data;
   struct mulle_utf8data   slug;

   if( ! s)
      return( NULL);

   data = mulle_utf8data_make( (char *) s, -1);
   slug = mulle_utf8data_slugify( data, NULL);
   assert( slug.characters);
   assert( slug.length >= 1); // sic (the trailing 0)
   return( (char *) slug.characters);
}


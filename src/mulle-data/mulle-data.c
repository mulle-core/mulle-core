#include "mulle-data.h"


int   __MULLE_DATA_ranlib__;


uint32_t   mulle_data_get_version( void)
{
   return( MULLE__DATA_VERSION);
}


MULLE__DATA_GLOBAL
void   *mulle_data_search_data( struct mulle_data haystack,
                                struct mulle_data needle)
{
   char      *haystack_bytes;
   char      *needle_bytes;
   size_t    i;
   size_t    j;

   mulle_data_assert( haystack);
   mulle_data_assert( needle);

   if( ! haystack.bytes || ! needle.bytes || haystack.length < needle.length)
      return( NULL);


   haystack_bytes = (char *) haystack.bytes;
   needle_bytes   = (char *) needle.bytes;

   for( i = 0; i <= haystack.length - needle.length; i++)
   {
      j = 0;

      while( j < needle.length && haystack_bytes[ i + j] == needle_bytes[ j])
        j++;

      if( j == needle.length)
         return( &haystack_bytes[ i]);
   }

   return( NULL);
}

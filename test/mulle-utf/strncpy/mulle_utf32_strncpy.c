#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


int  main()
{
   mulle_utf32_t   text[]  = { 'A', 'B', 'C', 0 };
   mulle_utf32_t   dst[32];
   mulle_utf32_t   *s;

   if( mulle_utf32_strncpy( NULL, 0, NULL) != NULL)
     printf( "FAIL");

   s = mulle_utf32_strncpy( dst, 32, text);
   if( s != dst)
     printf( "FAIL");
   if( mulle_utf32_strlen( s) != 3)
     printf( "FAIL");
   if( mulle_utf32_strcmp( s, text))
     printf( "FAIL");

   return( 0);
}


#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


int  main()
{
   char   text[]  = { 'A', 'B', 'C', 0 };
   char   dst[ 8];
   char   *s;

   if( mulle_utf8_strncpy( NULL, 0, NULL) != NULL)
     printf( "FAIL");

   s = mulle_utf8_strncpy( dst, 8, text);
   if( s != dst)
     printf( "FAIL");
   if( mulle_utf8_strlen( s) != 3)
     printf( "FAIL");
   if( mulle_utf8_strcmp( s, text))
     printf( "FAIL");

   return( 0);
}


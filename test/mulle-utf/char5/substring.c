#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int  main()
{
   int         i;
   int         j;
   int         k;
   uint32_t    code;
   uint32_t    subcode;
   uint64_t    code64;
   uint64_t    subcode64;
   size_t      len;
   char        *text;
   char        buf[ 32];

   text = &mulle_char5_get_charset()[ 1];

   for( i = 0; i <= mulle_char5_maxlength32; i++)
   {
      code = mulle_char5_encode32( text, i);
      for( j = 0; j < i; j++)
         for( k = 0; k <= i - j; k++)
         {
            subcode = mulle_char5_substring32( code, j, k);
            len     = mulle_char5_decode32( subcode, buf, sizeof( buf));
            printf( "%.*s( %d, %d) -> %.*s\n",  i,  text, j, k, (int) len, buf);
         }
   }

   for( i = 0; i <= mulle_char5_maxlength64; i++)
   {
      code64 = mulle_char5_encode64( text, i);
      for( j = 0; j < i; j++)
         for( k = 0; k <= i - j; k++)
         {
            subcode64 = mulle_char5_substring64( code64, j, k);
            len       = mulle_char5_decode64( subcode64, buf, sizeof( buf));
            printf( "%.*s( %d, %d) -> %.*s\n",  i,  text, j, k, (int) len, buf);
         }
   }

   return( 0);
}


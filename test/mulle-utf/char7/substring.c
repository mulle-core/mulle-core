#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int  main()
{
   int         i;
   int         j;
   int         k;
   uint64_t    code;
   uint64_t    subcode;
   uint64_t    code64;
   uint64_t    subcode64;
   size_t      len;
   // chars just need to be ASCII
   char        text[ ] =  {  '.', '0', '1', '2', 'A', 'C', 'E', 'I',
      'L', 'M', 'P', 'R', 'S', 'T', '_', 'a',
      'b', 'c', 'd', 'e', 'g', 'i', 'l', 'm',
      'n', 'o', 'p', 'r', 's', 't', 'u', 0 };
   char          buf[ 32];


   for( i = 0; i <= mulle_char7_maxlength32; i++)
   {
      code = mulle_char7_encode32( text, i);
      for( j = 0; j < i; j++)
         for( k = 0; k <= i - j; k++)
         {
            subcode = mulle_char7_substring32( code, j, k);
            len     = mulle_char7_decode32( subcode, buf, sizeof( buf));
            printf( "%.*s( %d, %d) -> %.*s\n",  i,  text, j, k, (int) len, buf);
         }
   }

   for( i = 0; i <= mulle_char7_maxlength64; i++)
   {
      code64 = mulle_char7_encode64( text, i);
      for( j = 0; j < i; j++)
         for( k = 0; k <= i - j; k++)
         {
            subcode64 = mulle_char7_substring64( code64, j, k);
            len       = mulle_char7_decode64( subcode64, buf, sizeof( buf));
            printf( "%.*s( %d, %d) -> %.*s\n",  i,  text, j, k, (int) len, buf);
         }
   }

   return( 0);
}


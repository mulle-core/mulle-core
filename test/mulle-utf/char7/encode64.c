#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int  main()
{
   int         i;
   uint64_t    code;
   char        buf[ 7];
   size_t      size;

   /**/
   /**/
   /**/


   code = mulle_char7_encode64( ".u", 2);
   size = mulle_char7_decode64( code, buf, 2);
   buf[ size] = 0;

   printf( "%llx\n", (long long) code);

   for( i = 0; i < 2; i++)
      putchar( mulle_char7_get( code, i));
   printf( "\n");

   /**/
   /**/
   /**/

   memset( buf, 'X', sizeof( buf));

   code = mulle_char7_encode64( "a:b:c:", 6);
   size = mulle_char7_decode64( code, buf, 6);

   if( size != 6)
   {
      printf( "encode decode fail\n");
      abort();
   }

   printf( "strlen  = %zu\n", mulle_char7_strlen64( code));
   printf( "fstrlen = %zu\n", mulle_char7_fstrlen64( code));

   if( buf[ 4] != 'c' || buf[ 5] != ':')
   {
      printf( "encode decode overflow\n");
      abort();
   }
   buf[ size] = 0;

   if( strncmp( buf, "a:b:c:", 6))
   {
      printf( "encode decode failed\n");
      abort();
   }

   for( i = 0; i < 6; i++)
      putchar( mulle_char7_get64( code, i));

   printf( "\n");

   return( 0);
}


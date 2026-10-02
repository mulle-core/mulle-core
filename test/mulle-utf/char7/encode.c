#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int  main()
{
   int         i;
   uint32_t    code;
   uint32_t    code64;
   char        buf[ 7];
   size_t      size;

   /**/
   /**/
   /**/

   code = mulle_char7_encode32( ".u", 2);
   size = mulle_char7_decode32( code, buf, 2);
   buf[ size] = 0;

   printf( "%llx\n", (long long) code);

   for( i = 0; i < 2; i++)
      putchar( mulle_char7_get( code, i));
   printf( "\n");

   /**/
   /**/
   /**/
   mulle_utf16_t  text16[ 2] = { '.', 'u' };

   code = mulle_char7_encode32_utf16( text16, 2);
   mulle_char7_decode32( code, buf, 2);
   printf( "%llx\n", (long long) code);

   code64 = mulle_char7_encode64_utf16( text16, 2);
   mulle_char7_decode64( code64, buf, 2);
   printf( "%llx\n", (long long) code64);



   /**/
   /**/
   /**/
   mulle_utf32_t  text32[ 2] = { '.', 'u' };

   code = mulle_char7_encode32_utf32( text32, 2);
   mulle_char7_decode32( code, buf, 2);
   printf( "%llx\n", (long long) code);

   code64 = mulle_char7_encode64_utf32( text32, 2);
   mulle_char7_decode64( code64, buf, 2);
   printf( "%llx\n", (long long) code64);


   /**/
   /**/
   /**/

   memset( buf, 'X', sizeof( buf));

   code = mulle_char7_encode32( "LMPR", 4);
   size = mulle_char7_decode32( code, buf, 4);

   if( buf[ 5] != 'X')
   {
      printf( "encode decode overflow\n");
      abort();
   }
   buf[ size] = 0;

   if( strncmp( buf, "LMPR", 4))
   {
      printf( "encode decode failed\n");
      abort();
   }

   printf( "%llx\n", (long long) code);

   for( i = 0; i < 4; i++)
      putchar( mulle_char7_get32( code, i));
   printf( "\n");

   /**/
   /**/
   /**/

   memset( buf, 'X', sizeof( buf));

   code64 = mulle_char7_encode64( "LMPR", 4);
   size   = mulle_char7_decode64( code64, buf, 4);

   if( buf[ 5] != 'X')
   {
      printf( "encode decode overflow\n");
      abort();
   }
   buf[ size] = 0;

   if( strncmp( buf, "LMPR", 4))
   {
      printf( "encode decode failed\n");
      abort();
   }

   printf( "%llx\n", (long long) code64);

   for( i = 0; i < 4; i++)
      putchar( mulle_char7_get64( code64, i));
   printf( "\n");

   return( 0);
}


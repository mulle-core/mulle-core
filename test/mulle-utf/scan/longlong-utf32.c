#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


static void   print_longlong( long long value, int rval)
{
   if( value == LLONG_MIN)
      printf( "LLONG_MIN");
   else
      if( value == LLONG_MAX)
         printf( "LLONG_MAX");
      else
         if( rval == mulle_utf_is_too_large_for_signed)
         {
            if( (unsigned long long) value == ULLONG_MAX)
               printf( "ULLONG_MAX");
            else
               printf( "%llu", value);
         }
         else
            printf( "%lld", value);
}


static void  test( char *text)
{
   int             rval;
   long long       value;
   mulle_utf32_t   *s;
   mulle_utf32_t   buf[ 256];
   size_t          i;
   size_t          len;

   printf( "\"%s\"", text);

   len = strlen( text);
   for( i = 0; i < len; i++)
      buf[ i] = text[ i];

   s     = buf;
   value = 0x18481848;
   rval  = _mulle_utf32_scan_longlong_decimal( &s, len, &value);
   if( rval >= 0)
   {
      printf( " -> ");
      print_longlong( value, rval);
   }

   if( s != &buf[ len] && rval != mulle_utf_has_overflown)
      printf( " garbage: '%c'", (int) *s);

   if( rval != 0)
      printf( " (%d)", rval);

   printf( "\n");
}


int  main()
{
   char   buf[ 256];

   // happy path tests
   test( "1848");
   test( "+1848");
   test( "-1848");

   // garbage at end
   test( "-1848a");

   test( "+");
   test( "-");
   test( "a");
   test( ".");
   test( ",");
   test( "0");
   test( "1");
   test( "9");

   test( "++");
   test( "+-");
   test( "+a");
   test( "+.");
   test( "+,");
   test( "+0");
   test( "+1");
   test( "+9");

   test( "-+");
   test( "--");
   test( "-a");
   test( "-.");
   test( "-,");
   test( "-0");
   test( "-1");
   test( "-9");

   test( "0+");
   test( "0-");
   test( "0a");
   test( "0.");
   test( "0,");
   test( "00");
   test( "01");
   test( "09");

   test( "1+");
   test( "1-");
   test( "1a");
   test( "1.");
   test( "1,");
   test( "10");
   test( "11");
   test( "19");

   test( "");

   sprintf( buf, "%lld", LLONG_MIN);
   test( buf);

   sprintf( buf, "%lld", LLONG_MAX);
   test( buf);

   sprintf( buf, "%llu", ULLONG_MAX);
   test( buf);

   test( "-123456789012345678901234567890");
   test( "123456789012345678901234567890");
   test( "+123456789012345678901234567890");

   return( 0);
}


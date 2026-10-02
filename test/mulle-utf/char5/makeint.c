#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>


static void   bit32( char *s)
{
   uint32_t   encoded;
   char       decoded[ 6];
   size_t     len;
   size_t     decoded_len;

   len = strlen( s);
   if( ! mulle_char5_is_char5string32( s, len))
      return;

   encoded     = mulle_char5_encode32( s, len);
   decoded_len = mulle_char5_decode32( encoded, decoded, sizeof( decoded));

   if( decoded_len != len || strncmp( decoded, s, decoded_len))
   {
      printf( "%s failed\n", s);
      abort();
   }
   printf( "32bit: %.*s (%zu): 0x%lx\n", (int) decoded_len, decoded, decoded_len, (unsigned long) encoded);
}


static void   bit64( char *s)
{
   uint64_t   encoded;
   char       decoded[ 12];
   size_t     len;
   size_t     decoded_len;

   len = strlen( s);
   if( ! mulle_char5_is_char5string64( s, len))
      return;

   encoded     = mulle_char5_encode64( s, len);
   decoded_len = mulle_char5_decode64( encoded, decoded, sizeof( decoded));
   if( decoded_len != len || strncmp( decoded, s, decoded_len))
   {
      printf( "%s failed\n", s);
      abort();
   }
   printf( "64bit: %.*s (%zu): 0x%llx\n", (int) decoded_len, decoded, decoded_len, (unsigned long long) encoded);
}


static void  test( char *s)
{
   bit32( s);
   bit64( s);
}


int  main()
{
   test( "");
   test( "a");
   test( "ab");
   test( "abc");
   test( "abcd");
   test( "abcde");
   test( "abcdea");
   test( "abcdeab");
   test( "abcdeabc");
   test( "abcdeabcd");
   test( "abcdeabcde");
   test( "abcdeabcdea");
   test( "abcdeabcdeab");
   test( "abcdeabcdeabc");  // 13

   return( 0);
}


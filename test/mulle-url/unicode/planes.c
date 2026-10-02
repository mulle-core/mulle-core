#include <mulle-url/mulle-url.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void  test( char *name,
                   int (*member_f)( int32_t),
                   int (*plane_f)( unsigned int))
{
   int32_t         c;
   unsigned int    planes[ 0x11];
   unsigned int    i;
   unsigned int    expect;

   memset( planes, 0, sizeof( planes));

   for( c = 0; c < 0x110000; c++)
      if( (*member_f)( c))
      {
         planes[ c >> 16] = 1;
         c  = c & ~0xFFFF;
         c += 0xFFFF;
         continue;
      }


   for( i = 0; i < 0x11; i++)
   {
      printf( "%s plane #%d : ", name, i);
      expect = (*plane_f)( i);
      if( expect == planes[ i])
         printf( "pass\n");
      else
         printf( "failed (expect: %d, result: %d)\n", expect, planes[ i]);
   }
}


int  main()
{
   test( "validurlfragment", mulle_unicode_is_validurlfragment, mulle_unicode_is_validurlfragmentplane);
   test( "validurlhost", mulle_unicode_is_validurlhost, mulle_unicode_is_validurlhostplane);
   test( "validurlpassword", mulle_unicode_is_validurlpassword, mulle_unicode_is_validurlpasswordplane);
   test( "validurlpath", mulle_unicode_is_validurlpath, mulle_unicode_is_validurlpathplane);
   test( "validurlquery", mulle_unicode_is_validurlquery, mulle_unicode_is_validurlqueryplane);
   test( "validurlscheme", mulle_unicode_is_validurlscheme, mulle_unicode_is_validurlschemeplane);
   test( "validurluser", mulle_unicode_is_validurluser, mulle_unicode_is_validurluserplane);
   test( "validurlport", mulle_unicode_is_validurlport, mulle_unicode_is_validurlportplane);

   return( 0);
}


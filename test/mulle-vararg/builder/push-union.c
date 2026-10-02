//
// Test for bug #3.1: mulle_vararg_push_union uses 'p' instead of 'ap'
// This test should fail to compile (or produce wrong output) before the fix.
//
#include <mulle-vararg/mulle-vararg.h>

#include <stdio.h>
#include <string.h>


union test_union
{
   int   i;
   char  c[ 8];
};


int   main( void)
{
   mulle_vararg_builderbuffer_t   buf[ mulle_vararg_builderbuffer_n( sizeof( union test_union) * 2)];
   mulle_vararg_list              list;
   mulle_vararg_list              q;
   union test_union               a;
   union test_union               b;
   union test_union               result;

   memset( &a, 0, sizeof( a));
   memset( &b, 0, sizeof( b));
   a.i = 1848;
   b.i = 1849;

   list = mulle_vararg_list_make( buf);
   mulle_vararg_copy( q, list);

   mulle_vararg_push_union( q, a);
   mulle_vararg_push_union( q, b);

   result = mulle_vararg_next_union( list, union test_union);
   printf( "%d\n", result.i);

   result = mulle_vararg_next_union( list, union test_union);
   printf( "%d\n", result.i);

   return( 0);
}

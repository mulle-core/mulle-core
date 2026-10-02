//
// Test: mulle_vararg_push_pointer + mulle_vararg_next_pointer round-trip
//
#include <mulle-vararg/mulle-vararg.h>

#include <stdio.h>


int   main( void)
{
   mulle_vararg_builderbuffer_t   buf[ mulle_vararg_builderbuffer_n( mulle_vararg_sizeof_pointer( void *) * 2)];
   mulle_vararg_list              list;
   mulle_vararg_list              q;
   int                            a;
   int                            b;
   int                            *p1;
   int                            *p2;
   int                            *r1;
   int                            *r2;

   a  = 1848;
   b  = 1849;
   p1 = &a;
   p2 = &b;

   list = mulle_vararg_list_make( buf);
   mulle_vararg_copy( q, list);

   mulle_vararg_push_pointer( q, p1);
   mulle_vararg_push_pointer( q, p2);

   r1 = mulle_vararg_next_pointer( list, int *);
   r2 = mulle_vararg_next_pointer( list, int *);

   if( r1 != p1 || r2 != p2 || *r1 != 1848 || *r2 != 1849)
   {
      printf( "FAIL: pointer round-trip\n");
      return( 1);
   }

   printf( "passed\n");
   return( 0);
}

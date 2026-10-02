//
// Test: mulle_vararg_push_struct + mulle_vararg_next_struct (by value)
// and _mulle_vararg_next_struct (by address)
//
#include <mulle-vararg/mulle-vararg.h>

#include <stdio.h>


struct some_struct
{
   int      x;
   double   y;
};


int   main( void)
{
   mulle_vararg_builderbuffer_t   buf[ mulle_vararg_builderbuffer_n( sizeof( struct some_struct) * 2)];
   mulle_vararg_list              list;
   mulle_vararg_list              q;
   struct some_struct             a;
   struct some_struct             b;
   struct some_struct             r1;
   struct some_struct             *rp;

   a.x = 1848;
   a.y = 48.0;
   b.x = 1849;
   b.y = 49.0;

   list = mulle_vararg_list_make( buf);
   mulle_vararg_copy( q, list);

   mulle_vararg_push_struct( q, a);
   mulle_vararg_push_struct( q, b);

   r1 = mulle_vararg_next_struct( list, struct some_struct);
   rp = _mulle_vararg_next_struct( list, struct some_struct);

   if( r1.x != 1848 || r1.y != 48.0)
   {
      printf( "FAIL: struct round-trip (by value)\n");
      return( 1);
   }

   if( rp->x != 1849 || rp->y != 49.0)
   {
      printf( "FAIL: struct round-trip (by address)\n");
      return( 1);
   }

   printf( "passed\n");
   return( 0);
}

#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>


union some_union
{
   int   x;
   char  y[ 5];  // not so nice alignment
};


struct
{
   long long          a;
   union some_union   b;
   union some_union   c;
} value =
{
   -1848,
   {
      .y = { '1', '8', '4', '8', 0}
   },
   {
      .y = { '1', '8', '4', '9', 0}
   }
};


int   main( void)
{
   mulle_vararg_list   list;
   union some_union    v;

   _mulle_vararg_start( list, value.a);

   v = mulle_vararg_next_union( list, union some_union);
   printf( "%s\n", v.y);

   v = mulle_vararg_next_union( list, union some_union);
   printf( "%s\n", v.y);

   mulle_vararg_end( list);

   return( 0);
}

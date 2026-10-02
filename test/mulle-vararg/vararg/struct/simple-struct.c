#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>


struct some_struct
{
   int   x;
   char  y[ 5];  // not so nice alignment
};


struct
{
   long long            a;
   struct some_struct   b;
   struct some_struct   c;
} value =
{
   -1848,
   {
      1848,
      { '1', '8', '4', '8', 0}
   },
   {
      1849,
      { '1', '8', '4', '9', 0}
   }
};


int   main( void)
{
   mulle_vararg_list   list;
   struct some_struct  v;

   _mulle_vararg_start( list, value.a);

   v = mulle_vararg_next_struct( list, struct some_struct);
   printf( "%d, %s\n", v.x, v.y);

   v = mulle_vararg_next_struct( list, struct some_struct);
   printf( "%d, %s\n", v.x, v.y);

   mulle_vararg_end( list);

   return( 0);
}

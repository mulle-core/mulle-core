#include <mulle-data/mulle-data.h>

#include <stdio.h>
#include <string.h>


static int   int_compare( void *a, void *b)
{
   int   x;
   int   y;

   x = *(int *) a;
   y = *(int *) b;
   return( x < y ? -1 : (x > y ? 1 : 0));
}


static int   int_compare_r( void *a, void *b, void *thunk)
{
   int   direction;

   direction = *(int *) thunk;
   return( direction * int_compare( a, b));
}


static void   print_ints( char *title, int *buf, size_t n)
{
   size_t   i;

   printf( "%-16.16s:", title);
   for( i = 0; i < n; i++)
      printf( " %d", buf[ i]);
   printf( "\n");
}


static int   is_sorted( int *buf, size_t n)
{
   size_t   i;

   for( i = 1; i < n; i++)
      if( buf[ i - 1] > buf[ i])
         return( 0);
   return( 1);
}


int   main( int argc, char *argv[])
{
   int        big[ 200];
   int        direction;
   int        dupes[]    = { 3, 1, 3, 1, 3, 1, 3, 1 };
   int        empty[ 1]  = { 42 };
   int        one[]      = { 1 };
   int        reverse[]  = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
   int        seven[]    = { 4, 2, 6, 1, 5, 3, 7 };
   int        sorted[]   = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
   uint32_t   seed;
   unsigned   i;

   mulle_qsort( empty, 0, sizeof( int), int_compare);
   print_ints( "n == 0", empty, 1);

   mulle_qsort( one, 1, sizeof( int), int_compare);
   print_ints( "n == 1", one, 1);

   mulle_qsort( seven, 7, sizeof( int), int_compare);
   print_ints( "n == 7", seven, 7);

   mulle_qsort( sorted, 10, sizeof( int), int_compare);
   print_ints( "presorted", sorted, 10);

   mulle_qsort( reverse, 10, sizeof( int), int_compare);
   print_ints( "reversed", reverse, 10);

   mulle_qsort( dupes, 8, sizeof( int), int_compare);
   print_ints( "duplicates", dupes, 8);

   // NULL is tolerated by mulle_qsort_r
   mulle_qsort( NULL, 10, sizeof( int), int_compare);
   printf( "%-16.16s: survived\n", "NULL buffer");

   // deterministic pseudo random, exercises the median of nine path
   seed = 1848;
   for( i = 0; i < sizeof( big) / sizeof( big[ 0]); i++)
   {
      seed   = seed * 1103515245u + 12345u;
      big[ i] = (int) ((seed >> 16) % 1000);
   }
   mulle_qsort( big, sizeof( big) / sizeof( big[ 0]), sizeof( int), int_compare);
   printf( "%-16.16s: %d\n",
           "200 random",
           is_sorted( big, sizeof( big) / sizeof( big[ 0])));

   // descending via mulle_qsort_r thunk
   direction = -1;
   mulle_qsort_r( reverse, 10, sizeof( int), int_compare_r, &direction);
   print_ints( "descending", reverse, 10);

   return( 0);
}

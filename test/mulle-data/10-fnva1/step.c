#include <mulle-data/mulle-data.h>
#include <stdio.h>


int   main( void)
{
   uint32_t   hash;
   char       *s;

   hash = MULLE_FNV1A_32_INIT;
   s    = "VfL Bochum 1848";
   while( *s)
   {
      printf( "%08x + %c", hash, *s);
      hash = _mulle_fnv1a_step_32( hash, *s);
      printf( "-> %08x\n", hash);
      ++s;
   }
   return( 0);
}
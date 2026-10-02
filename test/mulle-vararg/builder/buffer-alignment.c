//
// Test for bug #3.5: Builder buffer alignment is insufficient for long double
// on platforms where alignof(long double) > alignof(double).
// The buffer element type must provide at least alignof(long double) alignment.
//
#include <mulle-vararg/mulle-vararg.h>

#include <stdio.h>
#include <stdalign.h>


int   main( void)
{
   int   buffer_align;
   int   longdouble_align;

   buffer_align    = alignof( mulle_vararg_builderbuffer_t);
   longdouble_align = alignof( long double);

   //
   // The buffer element type must have alignment >= alignof(long double)
   // so that mulle_vararg_push_longdouble doesn't produce misaligned writes.
   //
   if( buffer_align < longdouble_align)
   {
      printf( "FAIL: alignof(builderbuffer_t)=%d < alignof(long double)=%d\n",
              buffer_align, longdouble_align);
      return( 1);
   }

   //
   // Also verify a push/read round-trip for long double works correctly.
   //
   {
      mulle_vararg_builderbuffer_t   buf[ mulle_vararg_builderbuffer_n( sizeof( long double) * 2)];
      mulle_vararg_list              list;
      mulle_vararg_list              q;
      long double                    val1 = 18.48e10L;
      long double                    val2 = 48.18e10L;
      long double                    r1;
      long double                    r2;

      list = mulle_vararg_list_make( buf);
      mulle_vararg_copy( q, list);

      mulle_vararg_push_longdouble( q, val1);
      mulle_vararg_push_longdouble( q, val2);

      r1 = mulle_vararg_next_longdouble( list);
      r2 = mulle_vararg_next_longdouble( list);

      if( r1 != val1)
      {
         printf( "FAIL: long double round-trip 1: got %Lg, expected %Lg\n", r1, val1);
         return( 1);
      }
      if( r2 != val2)
      {
         printf( "FAIL: long double round-trip 2: got %Lg, expected %Lg\n", r2, val2);
         return( 1);
      }
   }

   printf( "passed\n");
   return( 0);
}

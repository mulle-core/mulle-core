#include <mulle-fprintf/mulle-fprintf.h>

#include <mulle-vararg/mulle-vararg.h>
#include <mulle-vararg/mulle-vararg-builder.h>


static void   test_mvfprintf( void)
{
   mulle_vararg_builderbuffer_t   buf[ mulle_vararg_builderbuffer_n(
                                          mulle_vararg_sizeof_integer( int)
                                        + mulle_vararg_sizeof_integer( long)
                                        + mulle_vararg_sizeof_double())];
   mulle_vararg_list             p;
   mulle_vararg_list             varargs;

   varargs = mulle_vararg_list_make( buf);
   p       = varargs;
   mulle_vararg_push_int( p, 1848);
   mulle_vararg_push_long( p, 48L);
   mulle_vararg_push_double( p, 3.5);

   mulle_mvfprintf( stdout, "%d %ld %g\n", varargs);
}


static void   test_mvprintf( void)
{
   mulle_vararg_builderbuffer_t   buf[ mulle_vararg_builderbuffer_n(
                                          mulle_vararg_sizeof_integer( int)
                                        + mulle_vararg_sizeof_integer( long))];
   mulle_vararg_list             p;
   mulle_vararg_list             varargs;

   varargs = mulle_vararg_list_make( buf);
   p       = varargs;
   mulle_vararg_push_int( p, 7);
   mulle_vararg_push_long( p, 11L);

   mulle_mvprintf( "%d-%ld\n", varargs);
}


int   main( void)
{
   test_mvfprintf();
   test_mvprintf();
   return( 0);
}

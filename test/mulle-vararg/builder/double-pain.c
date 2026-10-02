#include <mulle-vararg/mulle-vararg.h>
#include <mulle-buffer/mulle-buffer.h>

#include <stdio.h>
#include <string.h>

//
// varargs are always promoted to int/double if smaller !
// struct { struct { char c, char d } x}, not though... ?
//
struct reference
{
   int          c;
   double       d;
   int          e;
   long double  f;
};


void  reference_compare( struct reference *a, struct reference *b)
{
   if( a->c != b->c)
      printf( "a->c (%d), b->c (%d) differ\n", a->c, b->c);
   if( a->d != b->d)
      printf( "a->d (%g), b->d (%g) differ\n", a->d, b->d);
   if( a->e != b->e)
      printf( "a->e (%d), b->e (%d) differ\n", a->e, b->e);
   if( a->f != b->f)
      printf( "a->f (%Lg), b->f (%Lg) differ\n", a->f, b->f);
};


int   main( void)
{
   mulle_vararg_builderbuffer_t  buf[ mulle_vararg_builderbuffer_n( sizeof( struct reference))];
   mulle_vararg_list             list;
   mulle_vararg_list             q;
   struct reference              ref;

   list = mulle_vararg_list_make( buf);

   memset( buf, 0xA5, sizeof( buf));
   memset( &ref, 0xA5, sizeof( ref));

   ref.c = 18;
   ref.d = 48.0;
   ref.e = 1848;
   ref.f = 18.48e10L;

   mulle_vararg_copy( q, list);
   mulle_vararg_push_int( q, ref.c);
   mulle_vararg_push_double( q, ref.d);
   mulle_vararg_push_int( q, ref.e);
   mulle_vararg_push_longdouble( q, ref.f);

   reference_compare( &ref, (struct reference *) buf);

   //
   // memcmp doesn't really work, because a long double apparently doesn't use
   // all its bits
//   //
//   if( memcmp( &ref, list.p, sizeof( struct reference)))
//   {
//      fprintf( stderr, "reference layout (%ld bytes) and buffer (%ld bytes) differ\n",
//                     sizeof( struct reference),
//                     sizeof( buf));
//
//      mulle_flushablebuffer_init( &buffer,
//                                  storage,
//                                  sizeof( storage),
//                                  (mulle_flushablebuffer_flusher_t *) fwrite,
//                                  stderr);
//      {
//         output = mulle_flushablebuffer_as_buffer( &buffer);
//         mulle_buffer_hexdump( output, &ref, sizeof( ref), 0, 0);
//         mulle_buffer_add_string( output, "---\n");
//         mulle_buffer_hexdump( output, list.p, sizeof( ref), 0, 0);
//      }
//      mulle_flushablebuffer_done( &buffer);
//
//      return( 1);
//   }

   printf( "%d\n",   mulle_vararg_next_int( list));
   printf( "%.2f\n", mulle_vararg_next_double( list));
   printf( "%d\n",   mulle_vararg_next_int( list));
   printf( "%.2f\n", (double) mulle_vararg_next_longdouble( list));

   return( 0);
}

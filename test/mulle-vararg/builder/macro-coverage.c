//
//  macro-coverage.c
//
//  Coverage for the generated convenience macros in mulle-vararg that had no
//  call site anywhere in the project, and had therefore never been compiled:
//
//     mulle_vararg_sizeof_*   (char short int long longlong int32 int64
//                              unsignedchar unsignedshort unsignedint
//                              unsignedlong unsignedlonglong uint32 uint64
//                              struct)
//     mulle_vararg_alignof_*  (integer pointer functionpointer struct)
//     mulle_vararg_push_*     (the typed pushers)
//     mulle_vararg_next_*     (the typed readers)
//     mulle_vararg_builder_do
//     mulle_vararg_start_fp
//
//  The sizeof/alignof checks are route checks: the interesting failure is a
//  convenience macro that routes to the wrong base macro, which is exactly
//  what the earlier sizeof_float regression was.
//
#include <mulle-vararg/mulle-vararg.h>

#include <stdio.h>
#include <stdalign.h>


static int   failures;


struct   pair
{
   int      a;
   double   b;
};


//
// the metaABI convention: `mulle_vararg_start_fp` expects the first
// parameter to be reachable as `_param->ap`
//
struct   fpparam
{
   float    ap;
   double   b;
   double   c;
};


#define CHECK( expr)                                  \
   do                                                 \
   {                                                  \
      if( ! (expr))                                   \
      {                                               \
         printf( "FAIL:%d: %s\n", __LINE__, #expr);   \
         ++failures;                                  \
      }                                               \
   }                                                  \
   while( 0)


static void   test_sizeof( void)
{
   CHECK( mulle_vararg_sizeof_char() == mulle_vararg_sizeof_integer( char));
   CHECK( mulle_vararg_sizeof_short() == mulle_vararg_sizeof_integer( short));
   CHECK( mulle_vararg_sizeof_int() == mulle_vararg_sizeof_integer( int));
   CHECK( mulle_vararg_sizeof_long() == mulle_vararg_sizeof_integer( long));
   CHECK( mulle_vararg_sizeof_longlong() == mulle_vararg_sizeof_integer( long long));
   CHECK( mulle_vararg_sizeof_int32() == mulle_vararg_sizeof_integer( int32_t));
   CHECK( mulle_vararg_sizeof_int64() == mulle_vararg_sizeof_integer( int64_t));

   CHECK( mulle_vararg_sizeof_unsignedchar() == mulle_vararg_sizeof_integer( unsigned char));
   CHECK( mulle_vararg_sizeof_unsignedshort() == mulle_vararg_sizeof_integer( unsigned short));
   CHECK( mulle_vararg_sizeof_unsignedint() == mulle_vararg_sizeof_integer( unsigned int));
   CHECK( mulle_vararg_sizeof_unsignedlong() == mulle_vararg_sizeof_integer( unsigned long));
   CHECK( mulle_vararg_sizeof_unsignedlonglong() == mulle_vararg_sizeof_integer( unsigned long long));
   CHECK( mulle_vararg_sizeof_uint32() == mulle_vararg_sizeof_integer( uint32_t));
   CHECK( mulle_vararg_sizeof_uint64() == mulle_vararg_sizeof_integer( uint64_t));

   CHECK( mulle_vararg_sizeof_struct( struct pair) == sizeof( struct pair));

   // everything that promotes to int must occupy an int slot
   CHECK( mulle_vararg_sizeof_char() == sizeof( int));
   CHECK( mulle_vararg_sizeof_short() == sizeof( int));
   CHECK( mulle_vararg_sizeof_unsignedchar() == sizeof( int));
   CHECK( mulle_vararg_sizeof_unsignedshort() == sizeof( int));
}


static void   test_alignof( void)
{
   CHECK( mulle_vararg_alignof_integer( char) == alignof( int));
   CHECK( mulle_vararg_alignof_integer( long long) == alignof( long long));
   CHECK( mulle_vararg_alignof_pointer( void *) == alignof( void *));
   CHECK( mulle_vararg_alignof_functionpointer( void (*)( void)) == alignof( void (*)( void)));
   CHECK( mulle_vararg_alignof_struct( struct pair) == alignof( struct pair));
}


static void   test_push_next_roundtrips( void)
{
   mulle_vararg_builderbuffer_t   buf[ mulle_vararg_builderbuffer_n( 256)];
   mulle_vararg_list              list;
   mulle_vararg_list              q;

   list = mulle_vararg_list_make( buf);
   mulle_vararg_copy( q, list);

   mulle_vararg_push_char( q, 'V');
   mulle_vararg_push_short( q, (short) 1848);
   mulle_vararg_push_int32( q, (int32_t) -1848);
   mulle_vararg_push_int64( q, (int64_t) -18481848);
   mulle_vararg_push_longlong( q, (long long) -18481848);
   mulle_vararg_push_long( q, (long) 1848);
   mulle_vararg_push_uint32( q, (uint32_t) 18481848);
   mulle_vararg_push_uint64( q, (uint64_t) 184818481848);
   mulle_vararg_push_unsignedchar( q, (unsigned char) 184);
   mulle_vararg_push_unsignedshort( q, (unsigned short) 1848);
   mulle_vararg_push_unsignedint( q, (unsigned int) 18481848);
   mulle_vararg_push_unsignedlong( q, (unsigned long) 1848184818);
   mulle_vararg_push_unsignedlonglong( q, (unsigned long long) 184818481848);
   mulle_vararg_push_float( q, 18.48f);

   CHECK( mulle_vararg_next_char( list) == 'V');
   CHECK( mulle_vararg_next_short( list) == (short) 1848);
   CHECK( mulle_vararg_next_int32( list) == (int32_t) -1848);
   CHECK( mulle_vararg_next_int64( list) == (int64_t) -18481848);
   CHECK( mulle_vararg_next_longlong( list) == (long long) -18481848);
   CHECK( mulle_vararg_next_long( list) == (long) 1848);
   CHECK( mulle_vararg_next_uint32( list) == (uint32_t) 18481848);
   CHECK( mulle_vararg_next_uint64( list) == (uint64_t) 184818481848);
   CHECK( mulle_vararg_next_unsignedchar( list) == (unsigned char) 184);
   CHECK( mulle_vararg_next_unsignedshort( list) == (unsigned short) 1848);
   CHECK( mulle_vararg_next_unsignedint( list) == (unsigned int) 18481848);
   CHECK( mulle_vararg_next_unsignedlong( list) == (unsigned long) 1848184818);
   CHECK( mulle_vararg_next_unsignedlonglong( list) == (unsigned long long) 184818481848);
   CHECK( mulle_vararg_next_float( list) == 18.48f);
}


static void   test_builder_do( void)
{
   mulle_vararg_builder_do( buf, mulle_vararg_sizeof_int() + mulle_vararg_sizeof_long())
   {
      mulle_vararg_list   list;
      mulle_vararg_list   q;

      list = mulle_vararg_list_make( buf);
      mulle_vararg_copy( q, list);

      mulle_vararg_push_int( q, 18);
      mulle_vararg_push_long( q, 48L);

      CHECK( mulle_vararg_next_int( list) == 18);
      CHECK( mulle_vararg_next_long( list) == 48L);
   }
}


static void   test_start_fp( void)
{
   struct fpparam    *  _param;
   struct fpparam       value;
   mulle_vararg_list    list;

   value.ap = 1.0f;
   value.b  = 2.0;
   value.c  = 3.0;
   _param   = &value;

   mulle_vararg_start_fp( list, ap);

   CHECK( mulle_vararg_next_double( list) == 2.0);
   CHECK( mulle_vararg_next_double( list) == 3.0);
}


int   main( void)
{
   test_sizeof();
   test_alignof();
   test_push_next_roundtrips();
   test_builder_do();
   test_start_fp();

   if( failures)
   {
      printf( "%d check(s) failed\n", failures);
      return( 1);
   }

   printf( "passed\n");
   return( 0);
}

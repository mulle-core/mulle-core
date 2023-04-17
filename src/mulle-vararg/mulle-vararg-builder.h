
#ifndef mulle_vararg_builder_h__
#define mulle_vararg_builder_h__

#include "mulle-align.h"
#include "mulle-vararg.h"


// Sketched out: an idea for a vararg builder.
// The builder function returns the address to push the next argument unto.
// It's the callers responsiblity to:
// * remember the start of the vararg buffer
// * ensure that the buffer is large enough (tricky!)
// * use the returned value as the first argument next time
//
// Intended use:
//
// mulle_vararg_builderbuffer_t  buf[ mulle_vararg_builderbuffer_n( sizeof( int) + sizeof( long))];
// mulle_vararg_list             p = mulle_vararg_list_make( buf);
// mulle_vararg_list             q;
//
// q = mulle_vararg_push_integer( p, 18);
// q = mulle_vararg_push_integer( q, 48L);
// mulle_mvsprintf( buffer, "%d %ld", p);

// use double for alignment
typedef double   mulle_vararg_builderbuffer_t;


#define mulle_vararg_builderbuffer_n( n)  \
   (((n) + sizeof( mulle_vararg_builderbuffer_t) - 1) / sizeof( mulle_vararg_builderbuffer_t))


#define _mulle_vararg_push( ap, type, value)                           \
do                                                                     \
{                                                                      \
   ap.p             = mulle_pointer_align( ap.p, alignof( type));      \
   *((type *) ap.p) = value;                                           \
   ap.p             = &((char *) ap.p)[ sizeof( type)];                \
}                                                                      \
while( 0)                                                              \



#define mulle_vararg_push_integer( ap, type, value)                    \
do                                                                     \
{                                                                      \
   if( sizeof( type) <= sizeof( int))                                  \
      _mulle_vararg_push( ap, int, (value));                           \
   else                                                                \
      if( sizeof( type) <= sizeof( long))                              \
         _mulle_vararg_push( ap, long, (value));                       \
      else                                                             \
         _mulle_vararg_push( ap, long long, (value));                  \
}                                                                      \
while( 0)


#define mulle_vararg_push_fp( ap, type, value)                         \
do                                                                     \
{                                                                      \
   if( sizeof( type) <= sizeof( double))                               \
      _mulle_vararg_push( ap, double, (value));                        \
   else                                                                \
      _mulle_vararg_push( ap, long double, (value));                   \
}                                                                      \
while( 0)


#define mulle_vararg_push_pointer( ap, value) \
   _mulle_vararg_push( ap, void *, value)

#define mulle_vararg_push_functionpointer( ap, value) \
   _mulle_vararg_push( ap, void (*)( void), value)


#define mulle_vararg_push_struct( ap, value)                           \
do                                                                     \
{                                                                      \
   ap.p = mulle_pointer_align( ap.p, alignof( value));                 \
   memcpy( ap.p, &value, sizeof( value));                              \
   ap.p = &((char *) ap.p)[ sizeof( value)];                           \
}                                                                      \
while( 0)

#define mulle_vararg_push_union( ap, value)                            \
   mulle_vararg_push_struct( p, value)


#define mulle_vararg_push_char( ap, value)  \
   mulle_vararg_push_integer( ap, char, value)

#define mulle_vararg_push_short( ap, value)  \
   mulle_vararg_push_integer( ap, short, value)

#define mulle_vararg_push_int( ap, value)  \
   mulle_vararg_push_integer( ap, int, value)

#define mulle_vararg_push_int32( ap, value)  \
   mulle_vararg_push_integer( ap, int32_t, value)

#define mulle_vararg_push_int64( ap, value)  \
   mulle_vararg_push_integer( ap, int64_t, value)

#define mulle_vararg_push_long( ap, value)  \
   mulle_vararg_push_integer( ap, long, value)

#define mulle_vararg_push_longlong( ap, value)  \
   mulle_vararg_push_integer( ap, long long, value)


#define mulle_vararg_push_unsignedchar( ap, value) \
   mulle_vararg_push_integer( ap, unsigned char, value)

#define mulle_vararg_push_unsignedshort( ap, value) \
   mulle_vararg_push_integer( ap, unsigned short, value)


#define mulle_vararg_push_unsignedint( ap, value) \
   mulle_vararg_push_integer( ap, unsigned int, value)

#define mulle_vararg_push_uint32( ap, value)  \
   mulle_vararg_push_integer( ap, uint32_t, value)

#define mulle_vararg_push_uint64( ap, value)  \
   mulle_vararg_push_integer( ap, uint64_t, value)

#define mulle_vararg_push_unsignedlong( ap, value) \
   mulle_vararg_push_integer( ap, unsigned long, value)

#define mulle_vararg_push_unsignedlonglong( ap, value) \
   mulle_vararg_push_integer( ap, unsigned long long, value)


#define mulle_vararg_push_float( ap, value) \
   mulle_vararg_push_fp( ap, float, value)

#define mulle_vararg_push_double( ap, value) \
   mulle_vararg_push_fp( ap, double, value)

#define mulle_vararg_push_longdouble( ap, value) \
   mulle_vararg_push_fp( ap, long double, value)

#endif

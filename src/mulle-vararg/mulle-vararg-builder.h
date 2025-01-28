
#ifndef mulle_vararg_builder_h__
#define mulle_vararg_builder_h__

#include "mulle-align.h"
#include "mulle-vararg.h"


/* Sketched out: an idea for a vararg builder.
 * The builder function returns the address to push the next argument unto.
 * It's the callers responsibility to:
 * * remember the start of the vararg buffer
 * * ensure that the buffer is large enough (tricky!)
 * * use the returned value as the first argument next time
 *
 * Intended use:
 *
 * #define size                                                            \
 *     mulle_vararg_builderbuffer_n( mulle_vararg_sizeof_integer( int) +   \
 *                                   mulle_vararg_sizeof_integer( long))
 * mulle_vararg_builderbuffer_t  buf[ size];
 * mulle_vararg_list             varargs = mulle_vararg_list_make( buf);
 * mulle_vararg_list             p = varargs;
 *
 * mulle_vararg_push_integer( p, 18);
 * mulle_vararg_push_integer( p, 48L);
 * mulle_mvsprintf( buffer, "%d %ld", varargs);
 */

// use double for alignment
typedef double   mulle_vararg_builderbuffer_t;


#define mulle_vararg_builderbuffer_n( n)  \
   (((n) + sizeof( mulle_vararg_builderbuffer_t) - 1) / sizeof( mulle_vararg_builderbuffer_t))


#define _mulle_vararg_push( ap, type, value)                      \
do                                                                \
{                                                                 \
   ap.p             = mulle_pointer_align( ap.p, alignof( type)); \
   *((type *) ap.p) = value;                                      \
   ap.p             = &((char *) ap.p)[ sizeof( type)];           \
}                                                                 \
while( 0)                                                         \


#define mulle_vararg_sizeof_integer( type)          \
   ((sizeof( type) <= sizeof( int))                 \
   ? sizeof( int)                                   \
   : ((sizeof( type) <= sizeof( long))              \
      ? sizeof( long)                               \
      : sizeof( long long)))

#define mulle_vararg_alignof_integer( type)         \
   ((sizeof( type) <= sizeof( int))                 \
   ? alignof( int)                                  \
   : ((sizeof( type) <= sizeof( long))              \
      ? alignof( long)                              \
      : alignof( long long)))

#define mulle_vararg_sizeof_fp( type)               \
   ((sizeof( type) <= sizeof( double))              \
   ? sizeof( double)                                \
   : sizeof( long double))

#define mulle_vararg_alignof_fp( type)              \
   ((sizeof( type) <= sizeof( int))                 \
   ? alignof( double)                               \
   : alignof( long double))

#define mulle_vararg_sizeof_pointer( type)          \
   sizeof( void *)                                  \

#define mulle_vararg_alignof_pointer( type)         \
   alignof( void *)                                 \

#define mulle_vararg_sizeof_functionpointer( type)  \
   sizeof( void (*)( void))                         \

#define mulle_vararg_alignof_functionpointer( type) \
   alignof( void (*)( void))                        \

#define mulle_vararg_sizeof_struct( type)           \
   sizeof( type)                                    \

#define mulle_vararg_alignof_struct( type)          \
   alignof( type)                                   \


#define mulle_vararg_sizeof_char()               mulle_vararg_sizeof_integer( char)
#define mulle_vararg_sizeof_short()              mulle_vararg_sizeof_integer( short)
#define mulle_vararg_sizeof_int()                mulle_vararg_sizeof_integer( int)
#define mulle_vararg_sizeof_long()               mulle_vararg_sizeof_integer( long)
#define mulle_vararg_sizeof_longlong()           mulle_vararg_sizeof_integer( long long)
#define mulle_vararg_sizeof_int32()              mulle_vararg_sizeof_integer( int32_t)
#define mulle_vararg_sizeof_int64()              mulle_vararg_sizeof_integer( int64_t)

#define mulle_vararg_sizeof_unsignedchar()       mulle_vararg_sizeof_integer( unsigned char)
#define mulle_vararg_sizeof_unsignedshort()      mulle_vararg_sizeof_integer( unsigned short)
#define mulle_vararg_sizeof_unsignedint()        mulle_vararg_sizeof_integer( unsigned int)
#define mulle_vararg_sizeof_unsignedlong()       mulle_vararg_sizeof_integer( unsigned long)
#define mulle_vararg_sizeof_unsignedlonglong()   mulle_vararg_sizeof_integer( unsigned long long)
#define mulle_vararg_sizeof_uint32()             mulle_vararg_sizeof_integer( uint32_t)
#define mulle_vararg_sizeof_uint64()             mulle_vararg_sizeof_integer( uint64_t)

#define mulle_vararg_sizeof_float()              mulle_vararg_sizeof_integer( float)
#define mulle_vararg_sizeof_double()             mulle_vararg_sizeof_integer( double)


#define mulle_vararg_push_integer( ap, type, value)    \
do                                                     \
{                                                      \
   if( sizeof( type) <= sizeof( int))                  \
      _mulle_vararg_push( ap, int, (value));           \
   else                                                \
      if( sizeof( type) <= sizeof( long))              \
         _mulle_vararg_push( ap, long, (value));       \
      else                                             \
         _mulle_vararg_push( ap, long long, (value));  \
}                                                      \
while( 0)


#define mulle_vararg_push_fp( ap, type, value)         \
do                                                     \
{                                                      \
   if( sizeof( type) <= sizeof( double))               \
      _mulle_vararg_push( ap, double, (value));        \
   else                                                \
      _mulle_vararg_push( ap, long double, (value));   \
}                                                      \
while( 0)


#define mulle_vararg_push_struct( ap, value)           \
do                                                     \
{                                                      \
   ap.p = mulle_pointer_align( ap.p, alignof( value)); \
   memcpy( ap.p, &value, sizeof( value));              \
   ap.p = &((char *) ap.p)[ sizeof( value)];           \
}                                                      \
while( 0)

#define mulle_vararg_push_union( ap, value)            \
   mulle_vararg_push_struct( p, value)


#define mulle_vararg_push_pointer( ap, value) \
   _mulle_vararg_push( ap, void *, value)

#define mulle_vararg_push_functionpointer( ap, value) \
   _mulle_vararg_push( ap, void (*)( void), value)


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


#define mulle_vararg_builder_do( name, size) \
   mulle_alloca_do( name, mulle_vararg_builderbuffer_t, mulle_vararg_builderbuffer_n( size))


#endif

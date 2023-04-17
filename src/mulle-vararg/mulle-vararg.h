//
//  mulle-vararg.h
//  mulle-objc-runtime
//
//  Created by Nat! on 29.10.15.
//  Copyright © 2015 Mulle kybernetiK. All rights reserved.
//

#ifndef mulle_vararg_h__
#define mulle_vararg_h__

#include "mulle-align.h"

//
// community version is always even
//
#define MULLE_VARARG_VERSION  ((1 << 20) | (1 << 8) | 5)


/*
 * -print:(NSString *) format, ...
 * [print:@"%d",  0.1]
 *
 * 32 bit:
 * struct vararg
 * {
 *    NSString *X;    // assume .align 2
 *    double    Y;    // assume .align 8
 * }
 *
 * 00: XXXXXXXX
 * 04: 00000000
 * 08: YYYYYYYY
 * 0A: YYYYYYYY
 *
 * -addIfTrue:(BOOL) flag a:(float) a, ...
 * [addIfTrue:YES a:0.2, 0.3]
 *
 * 32 bit:
 * struct vararg
 * {
 *    BOOL      X;
 *    float     Y;    // assume .align 4
 *    double    Z;    // assume .align 8
 * }
 *
 * 00: XX000000
 * 04: YYYYYYYY
 * 08: ZZZZZZZZ
 * 0A: ZZZZZZZZ
 */
typedef struct
{
   void   *p;
}  mulle_vararg_list;


// this is rarely used!!
static inline mulle_vararg_list   mulle_vararg_list_make( void *buf)
{
   mulle_vararg_list  list;

   list.p = buf;
   return( list);
}


#define mulle_vararg_start( args, ap)                                                  \
do                                                                                     \
{                                                                                      \
   args.p = &((char *) &ap)[ sizeof( ap) < sizeof( int) ? sizeof( int) : sizeof( ap)]; \
}                                                                                      \
while( 0)


// use this for integer types
static inline char  *_mulle_vararg_int_aligned_pointer( mulle_vararg_list *args,
                                                        size_t size,
                                                        unsigned int align)
{
   char   *q;

   if( size < sizeof( int))
   {
      size  = sizeof( int);
      align = alignof( struct{ int x; });  // weirdness for i386 gives warning C4116, just suppress
   }

   q       = mulle_pointer_align( args->p, align);
   args->p = &q[ size];
   return( q);
}


#define mulle_vararg_next_integer( args, type)                                                                \
   (sizeof( type) < sizeof( int)                                                                              \
      ? (type) *(int *) _mulle_vararg_int_aligned_pointer( &args, sizeof( type), alignof( struct{ type x; })) \
      : *(type *) _mulle_vararg_int_aligned_pointer( &args, sizeof( type), alignof( struct{ type x; })))


static inline char  *_mulle_vararg_aligned_pointer( mulle_vararg_list *args, unsigned int align)
{
   char   *q;

   q       = mulle_pointer_align( args->p, align);
   args->p = &q[ sizeof( void *)];
   return( q);
}


// use this for all pointer types
#define mulle_vararg_next_pointer( args, type)  \
   (*(type *) _mulle_vararg_aligned_pointer( &args, alignof( struct{ type x; })))


static inline void  *_mulle_vararg_aligned_struct( mulle_vararg_list *args,
                                                   size_t size,
                                                   unsigned int align)
{
   char   *q;

   q       = mulle_pointer_align( args->p, align);
   args->p = &q[ size];
   return( q);
}


// use this for all struct types
#define _mulle_vararg_next_struct( args, type)                              \
   ((type *) _mulle_vararg_aligned_struct( &args,                           \
                                           sizeof( struct{ type x; }),      \
                                           alignof( struct{ type x; })))

#define mulle_vararg_next_struct( args, type)    \
   (*_mulle_vararg_next_struct( args, type))


// use this for all union types
#define _mulle_vararg_next_union( args, type)                            \
   ((type *) _mulle_vararg_aligned_struct( &args,                        \
                                           sizeof( struct{ type x; }),   \
                                           alignof( struct{ type x; })))

#define mulle_vararg_next_union( args, type)    \
   (*_mulle_vararg_next_union( args, type))


#define _mulle_fp_correct_type_size( size)         \
   (size < sizeof( double)                         \
         ? sizeof( double)                         \
         : size)

// weirdness for i386
#define _mulle_fp_correct_type_align( size, align)   \
   (size < sizeof( double)                           \
         ? alignof( struct{ double x; })             \
         : align)

static inline char  *_mulle_vararg_double_aligned_pointer( mulle_vararg_list *args,
                                                           size_t size,
                                                           unsigned int align)
{
   char   *q;

   align   = _mulle_fp_correct_type_align( size, align);
   size    = _mulle_fp_correct_type_size( size);
   q       = mulle_pointer_align( args->p, align);
   args->p = &q[ size];
   return( q);
}


// need separate routine for FP arguments, as float promotes to double
#define mulle_vararg_next_fp( args, type)                                                                           \
   (sizeof( type) < sizeof( double)                                                                                 \
      ? (type) *(double *) _mulle_vararg_double_aligned_pointer( &args, sizeof( type), alignof( struct{ type x; })) \
      : *(type *) _mulle_vararg_double_aligned_pointer( &args, sizeof( type), alignof( struct{ type x; })))


// untested code!
static inline char  *_mulle_vararg_long_double_aligned_pointer( mulle_vararg_list *args,
                                                                size_t size,
                                                                unsigned int align)
{
   char   *q;

   if( size < sizeof( long double))
   {
      size  = sizeof( long double);
      align = alignof( struct{ long double x; });  // weirdness for i386
   }

   q       = mulle_pointer_align( args->p, align);
   args->p = &q[ size];
   return( q);
}


#define mulle_vararg_next_long_double( args, type)                                                                           \
   (sizeof( type) < sizeof( long double)                                                                                 \
      ? (type) *(long double *) _mulle_vararg_long_double_aligned_pointer( &args, sizeof( type), alignof( struct{ type x; })) \
      : *(type *) _mulle_vararg_long_double_aligned_pointer( &args, sizeof( type), alignof( struct{ type x; })))


#define mulle_vararg_copy( dst, src)  \
do                                    \
{                                     \
   dst = src;                         \
}                                     \
while( 0)

#define mulle_vararg_end( args)


// only works with pointers

static inline size_t   mulle_vararg_count_pointers( mulle_vararg_list args,
                                                    void *first)
{
   size_t   count;
   void     *p;

   count = 0;
   p     = first;
   while( p)
   {
      ++count;
      p = mulle_vararg_next_pointer( args, void *);
   }

   return( count);
}

/*
 * conveniences
 */
#define mulle_vararg_next_char( ap)  \
   mulle_vararg_next_integer( ap, char)

#define mulle_vararg_next_short( ap)  \
   mulle_vararg_next_integer( ap, short)

#define mulle_vararg_next_int( ap)  \
   mulle_vararg_next_integer( ap, int)

#define mulle_vararg_next_int32( ap)  \
   mulle_vararg_next_integer( ap, int32_t)

#define mulle_vararg_next_int64( ap)  \
   mulle_vararg_next_integer( ap, int64_t)

#define mulle_vararg_next_long( ap)  \
   mulle_vararg_next_integer( ap, long)

#define mulle_vararg_next_longlong( ap)  \
   mulle_vararg_next_integer( ap, long long)


#define mulle_vararg_next_unsignedchar( ap) \
   mulle_vararg_next_integer( ap, unsigned char)

#define mulle_vararg_next_unsignedshort( ap) \
   mulle_vararg_next_integer( ap, unsigned short)

#define mulle_vararg_next_unsignedint( ap) \
   mulle_vararg_next_integer( ap, unsigned int)

#define mulle_vararg_next_uint32( ap)  \
   mulle_vararg_next_integer( ap, uint32_t)

#define mulle_vararg_next_uint64( ap)  \
   mulle_vararg_next_integer( ap, uint64_t)

#define mulle_vararg_next_unsignedlong( ap) \
   mulle_vararg_next_integer( ap, unsigned long)

#define mulle_vararg_next_unsignedlonglong( ap) \
   mulle_vararg_next_integer( ap, unsigned long long)


#define mulle_vararg_next_float( ap) \
   mulle_vararg_next_fp( ap, float)

#define mulle_vararg_next_double( ap) \
   mulle_vararg_next_fp( ap, double)

#define mulle_vararg_next_longdouble( ap) \
   mulle_vararg_next_fp( ap, long double)


#include "mulle-vararg-builder.h"


#ifdef __has_include
# if __has_include( "_mulle-vararg-versioncheck.h")
#  include "_mulle-vararg-versioncheck.h"
# endif
#endif

#endif /* mulle_vararg_h */

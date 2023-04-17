//
//  mulle--pointerfifo.h
//  mulle-fifo
//
//  Created by Nat! on 10.02.2021
//  Copyright © 2021 Nat! for Mulle kybernetiK.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#ifndef mulle_pointerfifo_h_
#define mulle_pointerfifo_h_

#include "include.h"


/*
 * FIFO: single consumer thread / single producer thread only
 *       Non growing but size can be configured at init time.
 */
struct mulle_pointerfifo
{
   mulle_atomic_pointer_t   n;        
   unsigned int             write;    // only accessed by producer
   unsigned int             read;     // only accessed by consumer
   unsigned int             size;    // read only after init
   struct mulle_allocator   *allocator;
   mulle_atomic_pointer_t   *storage;
};


static inline void   _mulle_pointerfifo_init( struct mulle_pointerfifo *p, 
                                              unsigned int size,
                                              struct mulle_allocator *allocator)
{
   assert( size >= 2);

   p->write     = 0;
   p->read      = 0;
   p->size      = size;
   _mulle_atomic_pointer_nonatomic_write( &p->n, 0);
   p->allocator = allocator;
   p->storage   = mulle_allocator_malloc( allocator, size * sizeof( mulle_atomic_pointer_t));
}

static inline void   mulle_pointerfifo_init( struct mulle_pointerfifo *p,
                                             unsigned int size,
                                             struct mulle_allocator *allocator)
{
   if( p)
      _mulle_pointerfifo_init( p, size, allocator);
}


static inline void   _mulle_pointerfifo_done( struct mulle_pointerfifo *p)
{
   mulle_allocator_free( p->allocator, p->storage);
}


static inline void   mulle_pointerfifo_done( struct mulle_pointerfifo *p)
{
   if( p)
      _mulle_pointerfifo_done( p);
}



static inline unsigned int   _mulle_pointerfifo_get_count( struct mulle_pointerfifo *p)
{
   return( (unsigned int) (uintptr_t) _mulle_atomic_pointer_read( &p->n));
}


static inline unsigned int   mulle_pointerfifo_get_count( struct mulle_pointerfifo *p)
{
   if( ! p)
      return( 0);
   return( _mulle_pointerfifo_get_count( p));
}




static inline void   *_mulle_pointerfifo_read( struct mulle_pointerfifo *p)
{
   void   *pointer;

   if( ! _mulle_pointerfifo_get_count( p))
      return( NULL);

   pointer  = _mulle_atomic_pointer_read( &p->storage[ p->read]);
   p->read  = (p->read + 1) % p->size;
   _mulle_atomic_pointer_decrement( &p->n);

   return( pointer);
}


static inline void   *mulle_pointerfifo_read( struct mulle_pointerfifo *p)
{
   if( ! p)
      return( NULL);

   return( _mulle_pointerfifo_read( p));
}



static inline int   _mulle_pointerfifo_write( struct mulle_pointerfifo *p,
                                                 void *pointer)
{
   if( _mulle_pointerfifo_get_count( p) == p->size)
      return( -1);

   _mulle_atomic_pointer_write( &p->storage[ p->write], pointer);
   p->write = (p->write + 1) % p->size;
   _mulle_atomic_pointer_increment( &p->n);

   return( 0);
}


static inline int   mulle_pointerfifo_write( struct mulle_pointerfifo *p,
                                             void *pointer)
{
   if( ! p)
      return( -1);

   return( _mulle_pointerfifo_write( p, pointer));
}

#endif



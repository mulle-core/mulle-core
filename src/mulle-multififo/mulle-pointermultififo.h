//
//  mulle--pointerfifo.h
//  mulle-multififo
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
#ifndef mulle_pointermultififo_h_
#define mulle_pointermultififo_h_

#include "include.h"


struct mulle_pointermultififo
{
   mulle_atomic_pointer_t   n;        
   mulle_atomic_pointer_t   write;    // only accessed by producers
   mulle_atomic_pointer_t   read;     // only accessed by consumers
   unsigned int             size;     // read only after init
   struct mulle_allocator   *allocator;
   mulle_atomic_pointer_t   *storage;
};


void   _mulle_pointermultififo_init( struct mulle_pointermultififo *p,
                                     unsigned int size,
                                     struct mulle_allocator *allocator);


static inline void   _mulle_pointermultififo_done( struct mulle_pointermultififo *p)
{
   mulle_allocator_free( p->allocator, p->storage);
}


static inline unsigned int   _mulle_pointermultififo_get_count( struct mulle_pointermultififo *p)
{
   return( (unsigned int) (uintptr_t) _mulle_atomic_pointer_read( &p->n));
}


MULLE_C_NONNULL_FIRST
void   *_mulle_pointermultififo_read_barrier( struct mulle_pointermultififo *p);

//
// will return -1 on failure, which is usually EAGAIN (errno), which means the
// fifo is full. This can be remedied by making the fifo larger, but ususally
// indicates, that consumers are taking too long...
//
MULLE_C_NONNULL_FIRST
int   _mulle_pointermultififo_write( struct mulle_pointermultififo *p, void *pointer);



#endif



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
#ifndef mulle_lockingpointermultififo_h_
#define mulle_lockingpointermultififo_h_

#include "include.h"


struct mulle_lockingpointermultififo
{
   mulle_thread_mutex_t     lock;
   unsigned int             n;
   unsigned int             write;    // only accessed by producers
   unsigned int             read;     // only accessed by consumers
   unsigned int             size;     // read only after init
   struct mulle_allocator   *allocator;
   void                     **storage;
};


void   _mulle_lockingpointermultififo_init( struct mulle_lockingpointermultififo *p,
                                            unsigned int size,
                                            struct mulle_allocator *allocator);


void   _mulle_lockingpointermultififo_done( struct mulle_lockingpointermultififo *p);


MULLE_C_NONNULL_FIRST
unsigned int   _mulle_lockingpointermultififo_get_count( struct mulle_lockingpointermultififo *p);


MULLE_C_NONNULL_FIRST
void   *_mulle_lockingpointermultififo_read_barrier( struct mulle_lockingpointermultififo *p);

//
// will return -1 on failure, which is usually EAGAIN (errno), which means the
// fifo is full. This can be remedied by making the fifo larger, but ususally
// indicates, that consumers are taking too long...
//
MULLE_C_NONNULL_FIRST_SECOND
int   _mulle_lockingpointermultififo_write( struct mulle_lockingpointermultififo *p, void *pointer);



#endif



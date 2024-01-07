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
#include "mulle-lockingpointermultififo.h"

#include <string.h>
#include <errno.h>


#define NOTYET    ((void *) ~0)

void   _mulle_lockingpointermultififo_init( struct mulle_lockingpointermultififo *p,
                                            unsigned int size,
                                            struct mulle_allocator *allocator)
{
   assert( size >= 2);

   memset( p, 0, sizeof( *p));

   mulle_thread_mutex_init( &p->lock);
   p->size      = size;
   p->allocator = allocator;
   p->storage   = mulle_allocator_calloc( allocator, size, sizeof( void *));
}


void   _mulle_lockingpointermultififo_done( struct mulle_lockingpointermultififo *p)
{
   mulle_thread_mutex_done( &p->lock);
   mulle_allocator_free( p->allocator, p->storage);
}


unsigned int   _mulle_lockingpointermultififo_get_count( struct mulle_lockingpointermultififo *p)
{
   unsigned int   n;

   mulle_thread_mutex_do( p->lock)
   {
      n = p->n;
   }
   return( n);
}


void   *_mulle_lockingpointermultififo_read_barrier( struct mulle_lockingpointermultififo *p)
{
   void           *pointer;
   unsigned int   i;

   pointer = NULL;
   mulle_thread_mutex_do( p->lock)
   {
      if( ! p->n)
         break;

      i       = p->read % p->size;
      pointer = p->storage[ i];
      assert( pointer);
      --p->n;
      ++p->read;

      // mulle_atomic_memory_barrier();   implicit in lock
   }
   return( pointer);
}


int   _mulle_lockingpointermultififo_write( struct mulle_lockingpointermultififo *p,
                                            void *pointer)
{
   unsigned int   i;
   int            rval;

   assert( pointer != NULL);

   if( pointer == (void *) ~0)  // future
   {
      errno = EINVAL;
      return( -1);
   }

   rval = 0;
   mulle_thread_mutex_do( p->lock)
   {
      if( p->n == p->size)
      {
         errno = EBUSY;
         rval  = -1;
         break;
      }

      i              = p->write % p->size;
      p->storage[ i] = pointer;

      ++p->n;
      ++p->write;
   }
   return( rval);
}


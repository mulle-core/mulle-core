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
#include "mulle-pointermultififo.h"

#include <string.h>
#include <errno.h>


#define NOTYET    ((void *) ~0)

void   _mulle_pointermultififo_init( struct mulle_pointermultififo *p,
                                     unsigned int size,
                                     struct mulle_allocator *allocator)
{
   assert( size >= 2);

   memset( p, 0, sizeof( *p));

   p->size      = size;
   p->allocator = allocator;
   p->storage   = mulle_allocator_calloc( allocator, 1, size * sizeof( mulle_atomic_pointer_t));
}


void   *_mulle_pointermultififo_read_barrier( struct mulle_pointermultififo *p)
{
   void           *pointer;
   void           *actual;
   void           *a_i;
   void           *a_n;
   unsigned int   i;
   unsigned int   loops;

   for(;;)
   {
      for( loops = 32;--loops;)
      {
         a_n = _mulle_atomic_pointer_read( &p->n);
         if( a_n == NULL)
            return( NULL);

         a_i = _mulle_atomic_pointer_read( &p->read);
         i   = ((unsigned int) (uintptr_t) a_i) % p->size;

         // we read the pointer to be able to do a CAS later, if it's NULL
         // then another thread already did the CAS, so it's taken.
         // Just reloop.
         pointer = _mulle_atomic_pointer_read( &p->storage[ i]);
         if( pointer == NULL || pointer == NOTYET)  // contention with other thread, who's just below
            continue;

         // If actual is not pointer, it must be NULL (otherwise the
         // algorithm is broken and a writer overwrote stugg). If it is NULL,
         // then another thread was faster. Just reloop.
         actual = __mulle_atomic_pointer_cas( &p->storage[ i], NOTYET, pointer);
         if( actual == NULL || actual == NOTYET)
            continue;

         assert( actual == pointer);

         //
         // As soon as we increment p->read it's possible for another thread
         // to read another entry. Before that, we know its indexing NULL
         // and that's a no-go. So decrement p->n first. As soon as we
         // decrement p->n another writer may now appear and overwrite our
         // old NULL value though, that's not good either. The solution to
         // this is, that we write NOTYET into the storage and only change
         // it to NULL once we are done with decrement increment
         //
         _mulle_atomic_pointer_decrement( &p->n);
         _mulle_atomic_pointer_increment( &p->read);

         // this can't fail... brave words...
         for(;;)
         {
            actual = __mulle_atomic_pointer_cas( &p->storage[ i], NULL, NOTYET);
            if( actual == NOTYET)
               break;
         }

         // For API user, ensure that the contents of memory pointed to by
         // pointer is consistent
         mulle_atomic_memory_barrier();
         return( pointer);
      }

      // if we loop successlessly for some time, just yield and then try anew
      mulle_thread_yield();   // wait for other thread to finish up
   }
}


int   _mulle_pointermultififo_write( struct mulle_pointermultififo *p,
                                     void *pointer)
{
   void           *actual;
   void           *a_i;
   void           *a_n;
   unsigned int   i;
   unsigned int   loops;

   if( pointer == NULL)
      return( 0);
   if( pointer == NOTYET)
   {
      errno = EINVAL;
      return( -1);
   }

   for(;;)
   {
      for( loops = 32;--loops;)
      {
         a_n = _mulle_atomic_pointer_read( &p->n);
         if( a_n == (void *) (uintptr_t) p->size)
         {
            errno = EAGAIN; // EWOULDBLOCK
            return( -1);
         }

         a_i = _mulle_atomic_pointer_read( &p->write);
         i   = ((unsigned int) (uintptr_t) a_i) % p->size;

         // If actual was not NULL, then just reloop.
         actual = __mulle_atomic_pointer_cas( &p->storage[ i], NOTYET, NULL);
         if( actual != NULL)
            continue;

         _mulle_atomic_pointer_increment( &p->n);
         _mulle_atomic_pointer_increment( &p->write);

         // this can't fail... brave words...
         for(;;)
         {
            actual = __mulle_atomic_pointer_cas( &p->storage[ i], pointer, NOTYET);
            if( actual == NOTYET)
               break;
         }

         return( 0);
      }

      // if we loop successlessly for some time, just yield and then try anew
      mulle_thread_yield();   // wait for other thread to finish up
   }
}


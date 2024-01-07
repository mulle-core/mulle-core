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
   void           *previous;
   uintptr_t      a_i;
   unsigned int   i;
   unsigned int   loops;

   for(;;)
   {
      for( loops = 32;--loops;)
      {
         MULLE_THREAD_UNPLEASANT_RACE_YIELD();

         a_i = (uintptr_t) _mulle_atomic_pointer_read( &p->read);
         i   = (unsigned int) a_i % p->size;

         MULLE_THREAD_UNPLEASANT_RACE_YIELD();

         // we read the pointer to be able to do a CAS later, if it's NULL
         // then another thread already did the CAS, so it's taken.
         // Just reloop.
         pointer = _mulle_atomic_pointer_read( &p->storage[ i]);
         if( pointer == NULL)
            return( NULL);
         if( pointer == NOTYET)  // contention with other thread, who's just below
            continue;

         MULLE_THREAD_UNPLEASANT_RACE_YIELD();

         // If actual is not pointer, just reloop
         actual = __mulle_atomic_pointer_cas( &p->storage[ i], NOTYET, pointer);
         if( actual != pointer)
            continue;

         previous = actual;
         MULLE_THREAD_UNPLEASANT_RACE_YIELD();

         // if the read pointer changed unexpectedly then we undo what we
         // did and try to use a changed stable pointer
         actual = __mulle_atomic_pointer_cas( &p->read, (void *) (a_i + 1), (void *) a_i);
         if( actual != (void *) a_i)
         {
            MULLE_THREAD_UNPLEASANT_RACE_YIELD();
            // rewrite NULL into storage and retry
            for(;;)
            {
               actual = __mulle_atomic_pointer_cas( &p->storage[ i], pointer, NOTYET);
               assert( actual == NOTYET);
               if( actual == NOTYET)
                  break;
            }
            continue;
         }

         // only check this here
         assert( previous == pointer);
         MULLE_C_UNUSED( previous);

         MULLE_THREAD_UNPLEASANT_RACE_YIELD();

         // this can't fail... brave words...
         for(;;)
         {
            actual = __mulle_atomic_pointer_cas( &p->storage[ i], NULL, NOTYET);
            assert( actual == NOTYET);
            if( actual == NOTYET)
               break;
         }

         MULLE_THREAD_UNPLEASANT_RACE_YIELD();

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
   uintptr_t      a_i;
   unsigned int   i;
   unsigned int   loops;

   if( pointer == NULL || pointer == NOTYET)
   {
      errno = EINVAL;
      return( -1);
   }

   for(;;)
   {
      for( loops = 32;--loops;)
      {
         MULLE_THREAD_UNPLEASANT_RACE_YIELD();

         a_i = (uintptr_t) _mulle_atomic_pointer_read( &p->write);
         i   = (unsigned int) a_i % p->size;

         MULLE_THREAD_UNPLEASANT_RACE_YIELD();

         // If actual was not NULL, then just reloop.
         actual = __mulle_atomic_pointer_cas( &p->storage[ i], NOTYET, NULL);
         if( actual != NULL)
         {
            if( actual == NOTYET)
               continue;

            errno = ENOSPC;
            return( -1);
         }

         MULLE_THREAD_UNPLEASANT_RACE_YIELD();

         // we increment this now, so the next value can be written, ours
         // is protected from read/write by NOTYET, so if this has changed
         // though, we retry (gotta get rid of our NOTYET though)
         actual = __mulle_atomic_pointer_cas( &p->write, (void *) (a_i + 1), (void *) a_i);
         if( actual != (void *) a_i)
         {
            MULLE_THREAD_UNPLEASANT_RACE_YIELD();
            // rewrite NULL into storage and retry
            for(;;)
            {
               actual = __mulle_atomic_pointer_cas( &p->storage[ i], NULL, NOTYET);
               assert( actual == NOTYET);
               if( actual == NOTYET)
                  break;
            }
            continue;
         }

         MULLE_THREAD_UNPLEASANT_RACE_YIELD();

         // this can't fail... brave words...
         for(;;)
         {
            actual = __mulle_atomic_pointer_cas( &p->storage[ i], pointer, NOTYET);
            assert( actual == NOTYET);
            if( actual == NOTYET)
               break;
         }

         return( 0);
      }

      // if we loop successlessly for some time, just yield and then try anew
      mulle_thread_yield();   // wait for other thread to finish up
   }
}

